/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

template<typename T, typename RedOp, typename Fan, int Direct,
         int SlicePerChunk, int StepPerSlice, int Unroll, int P2p>
class Primitives<
    T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
  > {
  static constexpr int MaxRecv = Fan::MaxRecv, MaxSend = Fan::MaxSend;
  static constexpr int Input=0, Output=1;
  static constexpr int RoleInput = 0x01,
                       RoleOutput = 0x02,
                       RoleWaitRecv = 0x04,
                       RoleWaitSend = 0x08,
                       RolePostSend = 0x10,
                       RolePostRecv = 0x20,
                       Aborted = 0x40,
                       OffsFifoEnabled = 0x80,
                       SizesFifoEnabled = 0x100,
                       DirectWrite = 0x200,
                       DirectRead = 0x400,
                       ThreadsSynced = 0x800;
  const int tid;
  int nthreads;
  int nworkers;
  const int stepSize;
  Fan fan;
  int index; // Peer index I'm responsible for
  int flags;
  int group;
  uint64_t step;
  int *connOffsFifoPtr;   // (flags & OffsFifoEnabled)
  union {
    T *userBuff;            // (flags & (RoleInput|RoleOutput))
    T *connEltsFifo;        // !(flags & (RoleInput|RoleOutput))
  };
  union {
    int volatile *connSizesFifoPtr; //  (flags & SizesFifoEnabled)
    T *directBuff;                  // !(flags & SizesFifoEnabled)
  };
  uint64_t volatile *connStepPtr;
  uint64_t connStepCache; // Cache last seen value of (*connStepPtr)

  // Don't use barrier 0 as it's used by the final sync
  inline __device__ void barrier() {
    if (nthreads == WARP_SIZE)
      __syncwarp();
    else
      asm volatile("bar.sync %0, %1;" :: "r"(15-group), "r"(nthreads));
    flags |= ThreadsSynced;
  }
  inline __device__ void subBarrier() {
    if (nworkers == nthreads)
      barrier();
    else
      asm volatile("bar.sync %0, %1;" :: "r"(8-group), "r"(nworkers));
  }

  inline __device__ bool checkAbort(int &spins) {
    spins++;
    if (!(flags & Aborted) && spins == NCCL_SPINS_BEFORE_CHECK_ABORT) {
      flags |= *ncclShmem.comm.abortFlag ? Aborted : 0;
      spins = 0;
    }
    return flags & Aborted;
  }

  template <int DirectRecv, int DirectSend, int Recv, int Send, int Src, int Dst>
/* 中文导读：
 * 在真正读写数据前等待输入就绪或输出槽位有空闲，并根据 direct/shared 能力计算本次地址。
 * 等待依据连接进度与 slot，而非每次都做一次全 communicator 同步；中止标志用于打破异常等待。
 */
  __device__ __forceinline__ void waitPeer(intptr_t dstIx, intptr_t remoteIx, int offset, int nelts) {
    const bool isSendNotRecv = (Send && Recv) ? (flags & RoleWaitSend) : Send;
    const bool noRecvWait = DirectRecv && Src && (flags & DirectRead);        // no wait when directly reading from remote input
    const bool noSendWait = DirectSend && (flags & (DirectRead|DirectWrite)); // no wait in empty send (e.g. directScatter) or direct remote write
    if (((flags & (Recv*RoleWaitRecv)) && !noRecvWait) ||
        ((flags & (Send*RoleWaitSend)) && !noSendWait)) {
      int spins = 0;
/* 中文精读：把 waitPeer 的不等式读成人话
 * 发送等待：消费者 head + 8 >= 本次结束 step，表示环形数据槽尚未被覆盖，可以写。
 * 接收等待：生产者 tail >= 本次结束 step，表示所需数据已经发布，可以读。
 * connStepCache 先用寄存器缓存，只有不足时才反复读取共享进度，减少热路径访存。
 * DirectRead/Write 在满足协议契约时能跳过部分等待，但仍要遵守指针交换与生命周期约束。
 */
      while (connStepCache + (isSendNotRecv ? NCCL_STEPS : 0) < step + StepPerSlice) {
        connStepCache = *connStepPtr;
        if (checkAbort(spins)) break;
        //if (spins == 0) printf("r=%d b=%d t=%d SPUN OUT got=%d want=%d\n", ncclShmem.comm.rank, blockIdx.x, threadIdx.x, int(connStepCache + (isSendNotRecv ? NCCL_STEPS : 0)), int(step+StepPerSlice));
      }
    }

    if (flags & (Recv*RoleWaitRecv | Send*RoleWaitSend)) {
      if (isSendNotRecv && (flags & SizesFifoEnabled))
        connSizesFifoPtr[step%NCCL_STEPS] = nelts*sizeof(T);

      void **ptrs = isSendNotRecv ? (ncclShmem.groups[group].dsts + Dst)
                                  : (ncclShmem.groups[group].srcs + Src);
      if (flags & OffsFifoEnabled)
        ptrs[index] = connEltsFifo + loadInt(connOffsFifoPtr + (step%NCCL_STEPS))/sizeof(T);
      else if (isSendNotRecv && DirectSend) {
        if (flags & DirectWrite) {
          ptrs[index] = directBuff + remoteIx + offset;
        } else if (flags & DirectRead) {  // empty send
          ptrs[index] = nullptr;
        } else {
          ptrs[index] = connEltsFifo + (step%NCCL_STEPS)*stepSize;
        }
      } else if (!isSendNotRecv && DirectRecv) {
        if (flags & DirectRead) {
          ptrs[index] = directBuff + remoteIx + offset;
        } else if (flags & DirectWrite) {
          ptrs[index] = directBuff + dstIx + offset;  // send to next from my output buffer
        } else {
          ptrs[index] = connEltsFifo + (step%NCCL_STEPS)*stepSize;
        }
      }
      else {
        ptrs[index] = connEltsFifo + (step%NCCL_STEPS)*stepSize;
      }
      step += StepPerSlice;
    }
  }

  template<int Recv, int Send>
/* 中文导读：
 * 完成本 slice 后发布对应连接的进度，使远端/GPU/Proxy 能消费数据或回收槽位。
 * 必须结合 genericOp 中的数据写入、线程同步和 fence 阅读，只有 volatile 不能单独保证所有可见性。
 */
  inline __device__ void postPeer() {
    if (flags & (Recv*RolePostRecv | Send*RolePostSend)) {
      step += StepPerSlice;
      *connStepPtr = step;
    }
  }

  template <int DirectRecv1, int DirectSend1, int Recv, int Send, int SrcBuf, int DstBuf>
/* 中文导读：
 * Simple 协议的组合原语核心，模板参数选择收/发、本地输入/输出及 direct 访问。
 * 按 slice 等待、装配源/目标指针、执行融合复制/归约、同步并发布进度，避免为每个步骤另启 kernel。
 */
  __device__ __forceinline__ void genericOp(
      intptr_t srcIx, intptr_t dstIx, intptr_t remoteIx, int nelem, bool postOp
    ) {
    constexpr int DirectRecv = 1 && Direct && DirectRecv1;
    constexpr int DirectSend = 1 && Direct && DirectSend1;
    constexpr int Src = SrcBuf != -1;
    constexpr int Dst = DstBuf != -1;

/* 中文精读：尾块、空块与通信进度
 * 某 channel 的尾部 offset 可能已经超出 size，算法会传入非正 nelem；这里归一到 0。
 * 空数据仍须按固定 SlicePerChunk 推进同步和 step，不能简单 return 导致另一端永远等进度。
 * sliceSize 根据 nelem、片数与 step 容量调小，并保留最小搬运粒度，改善小尾块的延迟。
 * 它是有效元素数，乘 sizeof(T) 后才是 sizesFifo 中交给 Proxy 的字节数。
 */
    nelem = nelem < 0 ? 0 : nelem;
    int sliceSize = stepSize*StepPerSlice;
    sliceSize = max(divUp(nelem, 16*SlicePerChunk)*16, sliceSize/32);
    int slice = 0;
    int offset = 0;

    if (tid < nworkers && offset < nelem) {
      // Worker-only loop for non-empty slices. Non-workers and empty slices are
      // processed in the loop following this if block. The benefit of splitting
      // the loop like this is we pull two branches out of the critical path.
      // Using "number of branch insns (taken or not) encountered dynamically"
      // as the performance metric, then:
      //   perf_orig = 2*numslices
      //   perf_new = 2+numslices
      // So the new code and old code behave the same for numslices=2, and for
      // numslices>2 the new code is superior. And note that in the case
      // numslices=1, the loop is trivially unrollable (single iteration) so we
      // don't incur that that tail branch and we still have perf_new=2.
      //
      // ORIGINAL CODE:
      //   unrolled for(slices) {
      //     if(worker) { // This branch removed
      //       wait();
      //       subBarrier();
      //       if(slice not empty) // This branch removed
      //         ReduceCopyMulti();
      //     }
      //     barrier();
      //     post();
      //   } // Since we no longer unroll, new branch added here
      #if __CUDA_ARCH__ < 700
        // Yeah, so all that above don't matter a lick on older hardware.
        #pragma unroll SlicePerChunk
      #else
        #pragma unroll 1
      #endif
      do {
        sliceSize = sliceSize < nelem-offset ? sliceSize : nelem-offset;
        if (Src && (flags & (SrcBuf==Input ? RoleInput : RoleOutput)))
          ncclShmem.groups[group].srcs[0] = userBuff + srcIx + offset;
        if (Dst && (flags & (DstBuf==Input ? RoleInput : RoleOutput)))
          ncclShmem.groups[group].dsts[0] = userBuff + dstIx + offset;
        waitPeer<DirectRecv, DirectSend, Recv, Send, Src, Dst>(dstIx, remoteIx, offset, sliceSize);
        subBarrier();
        if (DirectRecv && ncclShmem.groups[group].srcs[0] == ncclShmem.groups[group].dsts[0]) {
          // We can only have one direct receive. Since srcs[0] == dstPtr+offset, skip one copy
          if (Send) {
            // (1-Send) is only there to avoid compilation errors in case MaxSend=0 (and Send=0).
            ReduceOrCopyMulti<Unroll, RedOp, T, 1, 1, 1, (1-Send)+MaxSend, 0>
              (tid, nworkers, nullptr, false,
               1, (T const**)ncclShmem.groups[group].srcs,
               fan.nsend(), (T**)ncclShmem.groups[group].dsts+1,
               sliceSize);
          }
        } else if (DirectSend && !DirectRecv && SrcBuf != Input && ncclShmem.groups[group].dsts[Dst] == nullptr) {
          // For broadcast in CollNet to do empty send
          ReduceOrCopyMulti<Unroll, RedOp, T, 1, 1, 1, 1, 0>
            (tid, nworkers, ncclShmem.redOpArgs, postOp,
             Recv, (T const**)ncclShmem.groups[group].srcs,
             Dst, (T**)ncclShmem.groups[group].dsts,
             sliceSize);
        } else {
/* 中文精读：多个原语怎样共享一个融合搬运内核
 * srcs 合并“本地输入（如果有）+ 接收邻居”，dsts 合并“本地输出（如果有）+ 发送邻居”。
 * ReduceOrCopyMulti 对同一批加载值执行归约并写入多个目标，避免先写临时结果再启动另一个 kernel。
 * PreOpN 限定哪些源需要输入预处理；postOp 只在全局结果完成处执行，例如整数平均最后除法。
 * 中间转发如果重复应用最终 postOp，会改变数学结果，因此算法层明确传 true/false。
 */
          constexpr int PreOpN = SrcBuf != Input ? 0 :
                                 DirectRecv*MaxRecv == NCCL_MAX_DIRECT_ARITY ? (1+NCCL_MAX_DIRECT_ARITY) : 1;
          ReduceOrCopyMulti<Unroll, RedOp, T, Recv+Src, Recv*MaxRecv+Src, Send+Dst, Send*MaxSend+Dst, PreOpN>
            (tid, nworkers, ncclShmem.redOpArgs, postOp,
             Recv*fan.nrecv()+Src, (T const**)ncclShmem.groups[group].srcs,
             Send*fan.nsend()+Dst, (T**)ncclShmem.groups[group].dsts,
             sliceSize);
        }
        barrier(); // This barrier has a counterpart in following loop
        if (Send && (flags & RolePostSend) && index == 0) __threadfence_system();
        __syncwarp();
        postPeer<Recv, Send>();
        offset += sliceSize;
        slice += 1;
      } while (slice < SlicePerChunk && offset < nelem);
    }

/* 中文精读：同步线程与复制线程必须走匹配的屏障
 * worker 在上面的循环复制有效 slice，非 worker 直接进入这里参与同一组 barrier/postPeer。
 * worker 遇到剩余空 slice 后也进入这里，确保所有参与线程的屏障次数一致。
 * Send 发布 tail 之前先做系统级 fence，使数据写入对 GPU/CPU/NIC 协作路径可见。
 * volatile 让进度被反复读取，barrier 协调本组线程，fence 约束可见性；三者职责不同。
 */
    // Non-workers come straight here. Workers too but only once the remaining
    // slices are all empty. Since empty slices are the uncommon case, and
    // worker perf is the limiter, perf-wise this loop is effectively unentered,
    // hence just a single branch insn.
    #pragma unroll 1
    while (slice < SlicePerChunk) {
      sliceSize = sliceSize < nelem-offset ? sliceSize : nelem-offset;
      { // Only workers could have Wait roles so we know the slice must be empty
        // since we've exited the loop above.
        waitPeer<DirectRecv, DirectSend, Recv, Send, Src, Dst>(0, 0, 0, 0);
      }
      barrier(); // Has couterpart in preceding worker-only loop.
      if (Send && (flags & RolePostSend) && sliceSize > 0 && index == 0) __threadfence_system();
      __syncwarp();
      postPeer<Recv, Send>();
      offset += sliceSize;
      slice += 1;
    }
  }

  // Scatter/Gather generic op
  // skip: my own rank order in the buffer chunks
  // shift: peer offset to avoid all ranks sending to or receiving from same peer
  template <int DirectRecv1, int DirectSend1, int Recv, int Send>
  __device__ __forceinline__ void
  ScatterGatherOp(intptr_t inpIx, intptr_t outIx, int totalElem, int peerElem, int skip, int shift, bool postOp) {
    constexpr int DirectRecv = 1 && Direct && DirectRecv1;
    constexpr int DirectSend = 1 && Direct && DirectSend1;
    int offset = 0; // slice offset
    int sliceSize = stepSize*StepPerSlice;
    int dataSize = max(DIVUP(peerElem, 16*SlicePerChunk)*16, sliceSize/32);  // per-peer slice size

    #pragma unroll
    for (int slice=0; slice<SlicePerChunk; ++slice) {
      int realSize = max(0, min(dataSize, peerElem-offset));
      if (tid < nworkers) {
        if (Send) {
          // Scatter pre-scales data of input buffer only in non-Direct case
          constexpr int PreOpN = DirectSend ? 0 : 1;
          if (flags & RoleInput) ncclShmem.groups[group].srcs[0] = userBuff + inpIx + offset;
          if (tid == 0) ncclShmem.groups[group].totalSendSize[slice] = 0; // Skip the threadfence
          // realSize is not accurate here; but intra-node does not rely on sizes FIFO
          waitPeer<0, DirectSend, 0, 1, 1, 0>(0, inpIx, offset, realSize);
          subBarrier();
          #pragma unroll
          // Loop over peers
          for (int j=0; j<fan.nsend(); j++) {
            int i = (j+shift)%fan.nsend();
            int peerOffset = i*peerElem;
            // Skip the data I am responsible of reducing myself
            if (skip >= 0 && i >= skip) peerOffset += peerElem;
            const T* src0 = (T*)ncclShmem.groups[group].srcs[0] + peerOffset;
            int realPeerSize = min(realSize, totalElem-peerOffset);
            if (realPeerSize > 0 && ncclShmem.groups[group].dsts[i] != nullptr) {
              ReduceOrCopyMulti<Unroll, RedOp, T, 1, 1, 1, 1, PreOpN>(tid, nworkers, ncclShmem.redOpArgs, false, 1, &src0, 1, (T**)ncclShmem.groups[group].dsts+i, realPeerSize);
              // Mark for threadfence at the end
              if (tid == 0) ncclShmem.groups[group].totalSendSize[slice] += realPeerSize;
            }
          }
        } else if (Recv) {
          if (flags & RoleOutput) ncclShmem.groups[group].dsts[0] = userBuff + outIx + offset;
          int peerOffset = index*peerElem;
          if (skip >= 0 && index >= skip) peerOffset += peerElem;
          // Adjust remote index with peer offset in case we are directly pulling from peer's output buffer
          waitPeer<DirectRecv, 0, 1, 0, 0, 1>(outIx, outIx+peerOffset, offset, realSize);
          subBarrier();
          if (DirectRecv && ncclShmem.groups[group].srcs[0] == ncclShmem.groups[group].dsts[0]) {
            // Since waitPeer sets srcs[0] to output buffer + offset, we are doing a direct-write based recv
            // Do nothing
          } else {
            #pragma unroll
            for (int j=0; j<fan.nrecv(); j++) {
              int i = (j+shift)%fan.nrecv();
              peerOffset = i*peerElem;
              if (skip >= 0 && i >= skip) peerOffset += peerElem;
              T* dst0 = (T*)ncclShmem.groups[group].dsts[0] + peerOffset;
              int realPeerSize = min(realSize, totalElem-peerOffset);
              if (realPeerSize > 0) ReduceOrCopyMulti<Unroll, RedOp, T, 1, 1, 1, 1, 0>(tid, nworkers, ncclShmem.redOpArgs, postOp, 1, (const T**)ncclShmem.groups[group].srcs+i, 1, &dst0, realPeerSize);
            }
          }
        }
      }
      barrier();
      // If we indeed send something, threadfence
      if (Send && (flags & RolePostSend) && ncclShmem.groups[group].totalSendSize[slice] > 0 && index == 0)
        __threadfence_system();
      __syncwarp();
      postPeer<Recv, Send>();
      offset += realSize;
    }
  }

/* 中文精读：把 connector 状态装入原语的线程角色
 * 只有承担 WaitRecv/PostRecv 的特定线程加载对应连接和进度指针，其他线程负责搬运计算。
 * 根据 direct/shared 能力选择指针交换、固定 FIFO 或 offsFifo 动态偏移。
 * 接收一侧读 tail 等生产，写 head 释放消费；发送一侧读 head 等空间，写 tail 发布生产。
 * 这里的 head/tail 是连接视图，具体位于哪一侧 CPU/GPU 内存由 transport connect 填好。
 */
  __device__ __forceinline__ void loadRecvConn(ncclPeer *peer, int connIndex, struct ncclWorkElem* e) {
    if (flags & (RoleWaitRecv|RolePostRecv)) {
      auto *conn = &peer->recv[connIndex].conn;
      step = conn->step;
      step = roundUp(step, SlicePerChunk*StepPerSlice);
      if (flags & RolePostRecv) {
        connStepPtr = conn->head;
        *connStepPtr = step; // Return credits in case we rounded up.
      }
      if (flags & RoleWaitRecv) {
        ncclShmem.groups[group].recvConns[index] = conn; // WaitRecv role saves since that's who needs it in setDataPtrs()
        connStepPtr = conn->tail;
        connStepCache = *connStepPtr;
        flags |= (conn->offsFifo != nullptr) ? OffsFifoEnabled : 0;
        if (Direct) {
          // User buffers have been registered
          if ((conn->direct & (NCCL_IPC_READ|NCCL_IPC_WRITE)) && e != nullptr && e->regUsed) {
            if (connIndex == 1 && P2p == 0) {
              flags |= DirectRead;  // scatter-reduce use direct pull
            } else {
              flags |= (e->direct & NCCL_DIRECT_WRITE) ? DirectWrite :
                       (e->direct & NCCL_DIRECT_READ)  ? DirectRead  : 0;
            }
          } else if (conn->direct & (NCCL_DIRECT_WRITE|NCCL_DIRECT_READ)) {
            if (connIndex == 1 && P2p == 0) {
              flags |= DirectRead;  // scatter-reduce use direct pull
            } else {
              // direct read not allowed in non-register case
              // otherwise, in one-to-multi send, we could mix empty send and intermediate send
              flags |= (conn->direct & NCCL_DIRECT_WRITE) ? DirectWrite : 0;
            }
          }
        }
        if (flags & OffsFifoEnabled)
          connOffsFifoPtr = conn->offsFifo;
        connEltsFifo = (T*)conn->buffs[NCCL_PROTO_SIMPLE];
      }
    }
  }

  __device__ __forceinline__ void loadSendConn(ncclPeer *peer, int connIndex, struct ncclWorkElem* e) {
    if (flags & (RoleWaitSend|RolePostSend)) {
      auto *conn = &peer->send[connIndex].conn;
      step = conn->step;
      step = roundUp(step, SlicePerChunk*StepPerSlice);
      if (flags & RolePostSend) {
        connStepPtr = conn->tail;
      }
      if (flags & RoleWaitSend) {
        ncclShmem.groups[group].sendConns[index] = conn; // WaitSend role saves since that's who needs it in setDataPtrs()
        connStepPtr = conn->head;
        connStepCache = *connStepPtr;
        flags |= (conn->offsFifo != nullptr) ? OffsFifoEnabled : 0;
        if (flags & OffsFifoEnabled)
          connOffsFifoPtr = conn->offsFifo;
        connEltsFifo = (T*)conn->buffs[NCCL_PROTO_SIMPLE];

        if (conn->sizesFifo != nullptr) {
          flags |= SizesFifoEnabled;
          connSizesFifoPtr = conn->sizesFifo;
        } else if (Direct) {
          // User buffers have been registered
          if ((conn->direct & (NCCL_IPC_READ|NCCL_IPC_WRITE)) && e != nullptr && e->regUsed) {
            if (connIndex == 1 && P2p == 0) {
              flags |= DirectRead;  // scatter-reduce use direct pull
            } else {
              flags |= (e->direct & NCCL_DIRECT_WRITE) ? DirectWrite :
                       (e->direct & NCCL_DIRECT_READ)  ? DirectRead  : 0;
            }
          } else if (conn->direct & (NCCL_DIRECT_WRITE|NCCL_DIRECT_READ)) {
            if (connIndex == 1 && P2p == 0) {
              flags |= DirectRead;  // scatter-reduce use direct pull
            } else {
              // direct read not allowed in non-register case
              // otherwise, in one-to-multi send, we could mix empty send and intermediate send
              flags |= (conn->direct & NCCL_DIRECT_WRITE) ? DirectWrite : 0;
            }
          }
        }
      }
    }
  }

 public:
/* 中文精读：构造原语就是建立本次线程组的执行上下文
 * tid/nthreads 是本原语线程组内编号，不一定等于整个 block 的线程数。
 * 扫描有效 recvPeers/sendPeers 决定 fan；把部分线程指定为 Wait/Post/Input/Output 角色。
 * group 的低位区分共享内存/命名 barrier 组，高位携带 connIndex，P2P 使用索引 1。
 * 最后装载连接并交换 direct 指针；匹配双方必须执行相容构造路径，否则可能在交换阶段等待。
 */
  __device__ Primitives(
      int tid, int nthreads, int const *recvPeers, int const *sendPeers,
      void const *inputBuf, void *outputBuf, uint64_t redOpArg, uint32_t group=0, struct ncclWorkElem* e = nullptr
    ):
    tid(tid),
    stepSize(ncclShmem.comm.buffSizes[NCCL_PROTO_SIMPLE]/NCCL_STEPS/sizeof(T)) {

    // For send operations, we need an extra warp to overlap the threadfence and the copy
    this->nthreads = nthreads;
    this->nworkers = nthreads - (MaxSend > 0 && nthreads-WARP_SIZE >= 64 ? WARP_SIZE : 0);
    this->group = group & (uint16_t)0xFFFF;
    int connIndex = group >> 16;

    int nrecv=0, nsend=0;
    while (nrecv < MaxRecv && recvPeers[nrecv] != -1) nrecv++;
    while (nsend < MaxSend && sendPeers[nsend] != -1) nsend++;
    this->fan = Fan(nrecv, nsend);

    constexpr int ThreadPerSync = 8;
    static_assert(MaxSend < ThreadPerSync && MaxRecv < ThreadPerSync, "Not enough threads to cover all peers");

    int g = tid / ThreadPerSync;
    int ng = nthreads / ThreadPerSync;
    index = tid % ThreadPerSync;
    flags = 0;
    if (g == 0) {
      if (index < nrecv) flags |= RoleWaitRecv;
      if (index == nrecv) flags |= RoleInput;
    } else if (g == 1) {
      if (index < nsend) flags |= RoleWaitSend;
      if (index == nsend) flags |= RoleOutput;
    } else if (g == ng - 2) {
      if (index < nrecv) flags |= RolePostRecv;
    } else if (g == ng - 1) {
      if (index < nsend) flags |= RolePostSend;
    }

    int peer = 0;
    if (flags & (RoleWaitRecv|RolePostRecv)) peer = recvPeers[index];
    if (flags & (RoleWaitSend|RolePostSend)) peer = sendPeers[index];

    loadRecvConn(&ncclShmem.channel.devPeers[peer], connIndex, e);
    loadSendConn(&ncclShmem.channel.devPeers[peer], connIndex, e);

    setDataPtrs(inputBuf, outputBuf, redOpArg, (struct ncclWorkElemReg*)e);
  }

/* 中文精读：step 必须跨原语对象和操作保留
 * 原语内部推进的 step 先保存在各个角色线程的局部状态，析构时写回连接 conn->step。
 * 下一个操作继续同一进度，避免旧 flag/旧槽位内容被误认为本次新数据。
 * 析构还要保证线程同步满足协议要求，不仅仅是 C++ 对象释放内存。
 * 这也是不能把一次原语调用解释为“重新创建一条连接”的原因。
 */
  __device__ ~Primitives() {
    // Ensure ncclShmem.groups[].send/recvConns are available
    if (!(flags & ThreadsSynced))
      barrier();
    // Save steps for the next operation
    if (flags & (RolePostSend|RolePostRecv)) {
      auto *conns = (flags & RolePostSend) ? ncclShmem.groups[group].sendConns : ncclShmem.groups[group].recvConns;
      conns[index]->step = step;
    }
    // Make sure all threads are done writing back conn->step and done using
    // ncclShmem.groups[group]
    barrier();
  }

  __device__ void setDataPtrs(void const *inputBuf, void *outputBuf, uint64_t redOpArg, struct ncclWorkElemReg* e) {
    if (flags & RoleInput) {
      userBuff = (T*)inputBuf;
      ncclShmem.redOpArgs[0] = redOpArg;  // scaler for local input
    }
    if (flags & RoleOutput) userBuff = (T*)outputBuf;
    bool recvProvider = flags == (flags|RoleWaitRecv|DirectWrite);
    bool sendAcceptor = flags == (flags|RoleWaitSend|DirectWrite);
    bool sendProvider = flags == (flags|RoleWaitSend|DirectRead); // sender provides direct buffer (to be fetched)
    bool recvAcceptor = flags == (flags|RoleWaitRecv|DirectRead); // receiver accepts direct buffer
    int regUsed = e != nullptr ? e->elem.regUsed : 0;

    if (Direct && recvProvider) {
      int spins = 0;
      void *volatile *slot = ncclShmem.groups[group].recvConns[index]->ptrExchange;
      // Wait for consumer to consume previous value before trampling it.
      while (*slot != nullptr && !checkAbort(spins));
      directBuff = (T*)outputBuf;
      // Encode pointer by XOR'ing against some address they definitely wouldn't send
      // since we want to allow them sending us nullptr while not colliding with
      // the empty slot value.
      *slot = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(directBuff) ^ reinterpret_cast<uintptr_t>(slot));
    }
    if (Direct && sendAcceptor) {
      int spins = 0;
      void *volatile *slot = ncclShmem.groups[group].sendConns[index]->ptrExchange;
      void *ptr;
      while (true) {
        ptr = *slot;
        if (ptr != nullptr || checkAbort(spins)) break;
      }
      directBuff = regUsed ? (T*)(e->dnOutputs[index]) :
                   reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) ^ reinterpret_cast<uintptr_t>(slot));
      *slot = nullptr;
    }
    if (Direct && sendProvider) {
      int spins = 0;
      void *volatile *slot = ncclShmem.groups[group].sendConns[index]->ptrExchange;
      volatile uint64_t* argSlot0 = ncclShmem.groups[group].sendConns[index]->redOpArgExchange;
      volatile uint64_t* argSlot1 = ncclShmem.groups[group].sendConns[index]->redOpArgExchange+1;
      // Wait for consumer to consume previous value before trampling it.
      while ((*slot != nullptr || *argSlot0 != 0 || *argSlot1 !=0) && !checkAbort(spins));
      // If there is no recv, then we are directly pulling from input buffer (e.g. directScatter)
      // Otherwise, we are pulling from output buffer (e.g. recvCopyDirectSend)
      directBuff = MaxRecv == 0 ? (T*)inputBuf : (T*)outputBuf;
      // Exchange pre-scalers for use in direct pull
      *argSlot0 = (uint64_t(1)<<32) | (uint32_t)redOpArg;
      *argSlot1 = (uint64_t(1)<<32) | (uint32_t)(redOpArg>>32);
      // Encode pointer by XOR'ing against some address they definitely wouldn't send
      // since we want to allow them sending us nullptr while not colliding with
      // the empty slot value.
      *slot = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(directBuff) ^ reinterpret_cast<uintptr_t>(slot));
    }
    if (Direct && recvAcceptor) {
      int spins = 0;
      void *volatile *slot = ncclShmem.groups[group].recvConns[index]->ptrExchange;
      volatile uint64_t* argSlot0 = ncclShmem.groups[group].recvConns[index]->redOpArgExchange;
      volatile uint64_t* argSlot1 = ncclShmem.groups[group].recvConns[index]->redOpArgExchange+1;
      void *ptr;
      while (true) {
        ptr = *slot;
        if (ptr != nullptr || checkAbort(spins)) break;
      }
      directBuff = regUsed ? (T*)(MaxSend == 0 ? e->upOutputs[index] : e->dnInputs[index]) :
                   reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) ^ reinterpret_cast<uintptr_t>(slot));
      if (MaxSend != 0) { // reduce group rather than gather group
        // Store scalers for remote inputs
        uint64_t arg0, arg1;
        while (true) {
          arg0 = *argSlot0;
          arg1 = *argSlot1;
          if ((arg0 != 0 && arg1 != 0) || checkAbort(spins)) break;
        }
        ncclShmem.redOpArgs[1+index] = ((arg1 & 0xffffffff)<<32) | (arg0 & 0xffffffff);
      }
      *argSlot0 = 0; *argSlot1 = 0;
      *slot = nullptr;
    }
  }

  __device__ void moveDataPtrs(intptr_t delta) {
    if (flags & (RoleInput|RoleOutput))
      userBuff += delta;
  }

/* 中文精读：从原语名字反推源与目的
 * send: 本地 input -> 下游；recv: 上游 -> 本地 output；copySend: input -> output 和下游。
 * recvReduceSend: 上游 + 本地 input -> 下游；recvReduceCopy: 上游 + input -> output。
 * recvCopySend: 上游 -> output 和下游；recvReduceCopySend: 上游 + input -> output 和下游。
 * 下面每个包装仅选择 genericOp 的模板位，不会每次单独发射 CUDA kernel。
 * Input/Output 下标都是 T 元素偏移；remoteOutIx 只在 direct 访问远端用户缓冲时需要。
 */
  __device__ __forceinline__ void send(intptr_t inpIx, int eltN) {
    genericOp<0, 0, 0, 1, Input, -1>(inpIx, -1, -1, eltN, false);
  }
  __device__ __forceinline__ void sendFromOutput(intptr_t outIx, int eltN) {
    genericOp<0, 0, 0, 1, Output, -1>(outIx, -1, -1, eltN, false);
  }
  __device__ __forceinline__ void directSend(intptr_t inpIx, intptr_t remoteOutIx, int eltN) {
    genericOp<0, 1, 0, 1, Input, -1>(inpIx, -1, remoteOutIx, eltN, false);
  }
  __device__ __forceinline__ void directSendFromOutput(intptr_t outIx, intptr_t remoteOutIx, int eltN) {
    genericOp<0, 1, 0, 1, Output, -1>(outIx, -1, remoteOutIx, eltN, false);
  }

  __device__ __forceinline__ void recv(intptr_t outIx, int eltN, bool postOp=false) {
    genericOp<0, 0, 1, 0, -1, Output>(-1, outIx, -1, eltN, postOp);
  }
  __device__ __forceinline__ void directRecv(intptr_t outIx, int eltN) {
    genericOp<1, 0, 1, 0, -1, Output>(-1, outIx, -1, eltN, /*postOp=*/false);
  }

  __device__ __forceinline__ void copySend(intptr_t inpIx, intptr_t outIx, int eltN, bool postOp=false) {
    genericOp<0, 0, 0, 1, Input, Output>(inpIx, outIx, -1, eltN, postOp);
  }
  __device__ __forceinline__ void directCopySend(intptr_t inpIx, intptr_t outIx, intptr_t remoteOutIx, int eltN, bool postOp=false) {
    genericOp<0, 1, 0, 1, Input, Output>(inpIx, outIx, remoteOutIx, eltN, postOp);
  }

  __device__ __forceinline__ void recvCopySend(intptr_t outIx, int eltN, bool postOp=false) {
    genericOp<0, 0, 1, 1, -1, Output>(-1, outIx, -1, eltN, postOp);
  }
  __device__ __forceinline__ void directRecvCopySend(intptr_t outIx, intptr_t remoteOutIx, int eltN) {
    genericOp<1, 1, 1, 1, -1, Output>(-1, outIx, remoteOutIx, eltN, false);
  }
  __device__ __forceinline__ void recvCopyDirectSend(intptr_t outIx, intptr_t remoteOutIx, int eltN, bool postOp=false) {
    genericOp<0, 1, 1, 1, -1, Output>(-1, outIx, remoteOutIx, eltN, postOp);
  }

  __device__ __forceinline__ void recvReduceCopy(intptr_t inpIx, intptr_t outIx, int eltN, bool postOp=false) {
    genericOp<0, 0, 1, 0, Input, Output>(inpIx, outIx, -1, eltN, postOp);
  }

  __device__ __forceinline__ void recvReduceSend(intptr_t inpIx, int eltN, bool postOp=false) {
    genericOp<0, 0, 1, 1, Input, -1>(inpIx, -1, -1, eltN, postOp);
  }
  __device__ __forceinline__ void directRecvReduceSend(intptr_t inpIx, intptr_t remoteInpIx, int eltN, bool postOp=false) {
    genericOp<1, 0, 1, 1, Input, -1>(inpIx, -1, remoteInpIx, eltN, postOp);
  }

  __device__ __forceinline__ void recvReduceCopySend(intptr_t inpIx, intptr_t outIx, int eltN, bool postOp=false) {
    genericOp<0, 0, 1, 1, Input, Output>(inpIx, outIx, -1, eltN, postOp);
  }
  __device__ __forceinline__ void directRecvReduceCopySend(intptr_t inpIx, intptr_t outIx, intptr_t remoteOutIx, int eltN, bool postOp=false) {
    // Direct is only for the send part
    genericOp<0, 1, 1, 1, Input, Output>(inpIx, outIx, remoteOutIx, eltN, postOp);
  }

  __device__ __forceinline__ void
  scatter(intptr_t inpIx, int totalElem, int peerElem, int skip, int shift) {
    ScatterGatherOp<0, 0, 0, 1>(inpIx, -1, totalElem, peerElem, skip, shift, /*postOp=*/false);
  }
  __device__ __forceinline__ void
  directScatter(intptr_t inpIx, int totalElem, int peerElem, int skip, int shift) {
    ScatterGatherOp<0, 1, 0, 1>(inpIx, -1, totalElem, peerElem, skip, shift, /*postOp=*/false);
  }

  __device__ __forceinline__ void
  gather(intptr_t outIx, int totalElem, int peerElem, int skip, int shift, bool postOp=false) {
    ScatterGatherOp<0, 0, 1, 0>(-1, outIx, totalElem, peerElem, skip, shift, postOp);
  }
  __device__ __forceinline__ void
  directGather(intptr_t outIx, int totalElem, int peerElem, int skip, int shift) {
    ScatterGatherOp<1, 0, 1, 0>(-1, outIx, totalElem, peerElem, skip, shift, /*postOp=*/false);
  }
};
