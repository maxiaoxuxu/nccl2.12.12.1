/*************************************************************************
 * Copyright (c) 2019-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_ALLOC_H_
#define NCCL_ALLOC_H_

#include "nccl.h"
#include "checks.h"
#include "align.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

template <typename T>
/* 中文精读：三类常见内存的区别
 * ncclCalloc 是普通 CPU 堆内存；ncclCudaHostCalloc 用 cudaHostAllocMapped 分配 GPU 可访问主机内存；
 * ncclCudaCalloc 用 cudaMalloc 分配显存，并在专用 stream 上清零后等待完成。
 * 模板参数 nelem 是元素数，内部乘 sizeof(T)，看调用时不要把元素数和字节数重复相乘。
 * GPU 能访问主机内存不代表这块内存位于 SM 共享内存，更不代表访问成本等同于显存。
 */
static ncclResult_t ncclCudaHostCallocDebug(T** ptr, size_t nelem, const char *filefunc, int line) {
  CUDACHECK(cudaHostAlloc(ptr, nelem*sizeof(T), cudaHostAllocMapped));
  memset(*ptr, 0, nelem*sizeof(T));
  INFO(NCCL_ALLOC, "%s:%d Cuda Host Alloc Size %ld pointer %p", filefunc, line, nelem*sizeof(T), *ptr);
  return ncclSuccess;
}
#define ncclCudaHostCalloc(...) ncclCudaHostCallocDebug(__VA_ARGS__, __FILE__, __LINE__)

static inline ncclResult_t ncclCudaHostFree(void* ptr) {
  CUDACHECK(cudaFreeHost(ptr));
  return ncclSuccess;
}

template <typename T>
static ncclResult_t ncclCallocDebug(T** ptr, size_t nelem, const char *filefunc, int line) {
  void* p = malloc(nelem*sizeof(T));
  if (p == NULL) {
    WARN("Failed to malloc %ld bytes", nelem*sizeof(T));
    return ncclSystemError;
  }
  //INFO(NCCL_ALLOC, "%s:%d malloc Size %ld pointer %p", filefunc, line, nelem*sizeof(T), p);
  memset(p, 0, nelem*sizeof(T));
  *ptr = (T*)p;
  return ncclSuccess;
}
#define ncclCalloc(...) ncclCallocDebug(__VA_ARGS__, __FILE__, __LINE__)

template <typename T>
static ncclResult_t ncclRealloc(T** ptr, size_t oldNelem, size_t nelem) {
  if (nelem < oldNelem) return ncclInternalError;
  if (nelem == oldNelem) return ncclSuccess;

  T* oldp = *ptr;
  T* p = (T*)malloc(nelem*sizeof(T));
  if (p == NULL) {
    WARN("Failed to malloc %ld bytes", nelem*sizeof(T));
    return ncclSystemError;
  }
  memcpy(p, oldp, oldNelem*sizeof(T));
  free(oldp);
  memset(p+oldNelem, 0, (nelem-oldNelem)*sizeof(T));
  *ptr = (T*)p;
  INFO(NCCL_ALLOC, "Mem Realloc old size %ld, new size %ld pointer %p", oldNelem*sizeof(T), nelem*sizeof(T), *ptr);
  return ncclSuccess;
}

template <typename T>
static ncclResult_t ncclCudaCallocDebug(T** ptr, size_t nelem, const char *filefunc, int line) {
  // Need async stream for P2P pre-connect + CUDA Graph
  cudaStream_t stream;
  CUDACHECK(cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking));
  CUDACHECK(cudaMalloc(ptr, nelem*sizeof(T)));
  CUDACHECK(cudaMemsetAsync(*ptr, 0, nelem*sizeof(T), stream));
  CUDACHECK(cudaStreamSynchronize(stream));
  CUDACHECK(cudaStreamDestroy(stream));
  INFO(NCCL_ALLOC, "%s:%d Cuda Alloc Size %ld pointer %p", filefunc, line, nelem*sizeof(T), *ptr);
  return ncclSuccess;
}
#define ncclCudaCalloc(...) ncclCudaCallocDebug(__VA_ARGS__, __FILE__, __LINE__)

template <typename T>
static ncclResult_t ncclCudaMemcpy(T* dst, T* src, size_t nelem) {
  CUDACHECK(cudaMemcpy(dst, src, nelem*sizeof(T), cudaMemcpyDefault));
  return ncclSuccess;
}

// Allocate memory to be potentially ibv_reg_mr'd. This needs to be
// allocated on separate pages as those pages will be marked DONTFORK
// and if they are shared, that could cause a crash in a child process
/* 中文精读：为何为可能注册的缓冲单独按页分配
 * verbs 的 fork 支持会对注册页施加 DONTFORK 等处理；若与普通小对象共用页，可能影响子进程。
 * 这里按系统页尺寸对齐并独占所需页，随后具体后端才调用 reg_mr。
 * 分配内存与创建 MR 是两个步骤，拥有一块缓冲不自动消耗一个 MR 注册项。
 * 页对齐也与后端 MR 缓存键相关，但不意味着不同 PD/NIC 上的注册可任意共享。
 */
static ncclResult_t ncclIbMallocDebug(void** ptr, size_t size, const char *filefunc, int line) {
  size_t page_size = sysconf(_SC_PAGESIZE);
  void* p;
  int size_aligned = ROUNDUP(size, page_size);
  int ret = posix_memalign(&p, page_size, size_aligned);
  if (ret != 0) return ncclSystemError;
  memset(p, 0, size);
  *ptr = p;
  INFO(NCCL_ALLOC, "%s:%d Ib Alloc Size %ld pointer %p", filefunc, line, size, *ptr);
  return ncclSuccess;
}
#define ncclIbMalloc(...) ncclIbMallocDebug(__VA_ARGS__, __FILE__, __LINE__)

#endif
