/*************************************************************************
 * Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

/* 中文精读：内置 Proxy profiling 的观察范围
 * 本模块记录 Proxy 子任务在 GPU 等待、网络等待、flush 等阶段的时间，用于定位进度瓶颈。
 * 编译条件关闭时函数退化为空实现，不应把没有记录理解成没有网络活动。
 * 这些时间属于主机 Proxy 观测点，不能直接当作 GPU 每个归约指令的执行时间。
 * 实际对比性能还需统一张量大小、rank/channel 配置与同步测量边界。
 */
#include "profiler.h"

//#define PROFILE_PROXY 1
#ifdef PROFILE_PROXY
#include "timer.h"
#include "alloc.h"

static const char* profilingStateSendStr[] = { "BufferWait", "GPUWait", "SendWait", "", "End" };
static const char* profilingStateRecvStr[] = { "BufferWait", "RecvWait", "FlushWait", "GPUWait", "End" };
static const char* profilingEventStr[] = { "SendRecv", "Sleep", "Idle", "Append" };
struct ncclProxyProfileEvent {
  double timestamp[6];
  uint64_t opCount;
  int peer;
  int step;
  uint16_t channel;
  uint8_t type; // send / recv
  uint8_t opIndex;
};

struct ncclProxyProfileEvent* profilingEvents = NULL;
int profilingIndex = 0;
double profilingStart = 0;
#define MAX_EVENTS 200000

ncclResult_t ncclProfilingRecord(struct ncclProxyArgs* args, int sub, int step, int state) {
  if (profilingEvents == NULL) {
    NCCLCHECK(ncclCalloc(&profilingEvents, MAX_EVENTS));
    profilingStart = gettime();
  }
  struct ncclProxyProfileEvent* event = NULL;
  if (state%8 == 0) {
    if (profilingIndex == MAX_EVENTS) return ncclSuccess;
    args->subs[sub].profilingEvents[step%NCCL_STEPS] = event = profilingEvents+profilingIndex++;
    if (state == ncclProxyProfileBegin) {
      // Proxy operation information
      event->opCount = args->opCount;
      event->channel = args->subs[sub].channelId;
      event->peer = args->subs[sub].peer;
      event->type = args->pattern;
      event->step = step;
      event->opIndex = (((uint64_t)args)/sizeof(struct ncclProxyArgs))%256;
    } else event->peer = -state;
  } else {
    event = (struct ncclProxyProfileEvent*)args->subs[sub].profilingEvents[step%NCCL_STEPS];
    if (state == ncclProxyProfileEnd) args->subs[sub].profilingEvents[step%NCCL_STEPS] = NULL;
    if (state == ncclProxyProfileAppendEnd) event->opCount = args->opCount;
  }
  // Timestamp
  event->timestamp[state%8] = gettime()-profilingStart;
  return ncclSuccess;
}

void ncclProfilingDump() {
  static int dumpDone = 0;
  if (dumpDone) return;
  dumpDone = 1;
  const char* str = getenv("NCCL_PROXY_PROFILE");
  if (!str) { free(profilingEvents); return; }
  FILE* f = fopen(str, "w");
  fprintf(f, "[\n");

  for (int i=0; i<profilingIndex; i++) {
    struct ncclProxyProfileEvent* e = profilingEvents+i;
    const int sendrecv = e->peer >= 0;
    const char* typeStr = sendrecv ? (e->type == ncclPatternSend ? "Send" : "Recv") :
      profilingEventStr[-(e->peer/8)];


    if (sendrecv) {
      int state = ncclProxyProfileBegin;
      const char** stateStr = e->type == ncclPatternSend ? profilingStateSendStr : profilingStateRecvStr;
      fprintf(f, "{\"name\": \"%s-%d-%d\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f, \"args\": { \"opCount\": %ld, \"proxyOpIndex\":%d } },\n",
          typeStr, e->peer, e->step, i, e->channel, e->timestamp[state], e->opCount, e->opIndex);

      while (state<ncclProxyProfileEnd) {
        if (e->timestamp[state]) {
          const char* name = stateStr[state];
          fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f },\n",
              name, i, e->channel, e->timestamp[state]);
          state++;
          while (e->timestamp[state] == 0) state++;
          fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"e\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f },\n",
              name, i, e->channel, e->timestamp[state]);
        }
      }

      fprintf(f, "{\"name\": \"%s-%d-%d\", \"cat\": \"NET\", \"ph\": \"e\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f },\n",
          typeStr, e->peer, e->step, i, e->channel, e->timestamp[state]);
    } else {
      if (e->peer == -ncclProxyProfileAppend) {
      fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": -1, \"tid\": 1, \"ts\": %f, \"args\": { \"added\": %ld } },\n",
          typeStr, i, e->timestamp[0], e->opCount);
      } else {
        fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": -1, \"tid\": 1, \"ts\": %f },\n",
          typeStr, i, e->timestamp[0]);
      }
      fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"e\", \"id\": %d, \"pid\": -1, \"tid\": 1, \"ts\": %f },\n",
          typeStr, i, e->timestamp[1]);
    }
  }
  fprintf(f, "{} ]\n");
  fclose(f);
  free(profilingEvents);
}
#else
ncclResult_t ncclProfilingRecord(struct ncclProxyArgs* args, int sub, int step, int state) { return ncclSuccess; }
void ncclProfilingDump() {}
#endif
