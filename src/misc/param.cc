/*************************************************************************
 * Copyright (c) 2019-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "param.h"
#include "debug.h"

#include <algorithm>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <pwd.h>

const char* userHomeDir() {
  struct passwd *pwUser = getpwuid(getuid());
  return pwUser == NULL ? NULL : pwUser->pw_dir;
}

void setEnvFile(const char* fileName) {
  FILE * file = fopen(fileName, "r");
  if (file == NULL) return;

  char *line = NULL;
  char envVar[1024];
  char envValue[1024];
  size_t n = 0;
  ssize_t read;
  while ((read = getline(&line, &n, file)) != -1) {
    if (line[read-1] == '\n') line[read-1] = '\0';
    int s=0; // Env Var Size
    while (line[s] != '\0' && line[s] != '=') s++;
    if (line[s] == '\0') continue;
    strncpy(envVar, line, std::min(1023,s));
    envVar[s] = '\0';
    s++;
    strncpy(envValue, line+s, 1023);
    envValue[1023]='\0';
    setenv(envVar, envValue, 0);
    //printf("%s : %s->%s\n", fileName, envVar, envValue);
  }
  if (line) free(line);
  fclose(file);
}

/* 中文精读：配置文件与环境变量的优先级
 * 先读用户目录 .nccl.conf，再读 /etc/nccl.conf；setEnvFile 用 setenv(...,0) 不覆盖已有值。
 * 所以已存在的进程环境优先，用户配置先填入的键也不会被后读的系统配置覆盖。
 * 数值参数由 ncclLoadParam 在锁内首次解析并保存缓存；不同线程不会同时重复写缓存。
 * 配置加载属于进程初始化，不能按每个 communicator 重新读取全部配置来理解。
 */
void initEnv() {
  char confFilePath[1024];
  const char * userDir = userHomeDir();
  if (userDir) {
    sprintf(confFilePath, "%s/.nccl.conf", userDir);
    setEnvFile(confFilePath);
  }
  sprintf(confFilePath, "/etc/nccl.conf");
  setEnvFile(confFilePath);
}

void ncclLoadParam(char const* env, int64_t deftVal, int64_t uninitialized, int64_t* cache) {
  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(&mutex);
  if (__atomic_load_n(cache, __ATOMIC_RELAXED) == uninitialized) {
    char* str = getenv(env);
    int64_t value = deftVal;
    if (str && strlen(str) > 0) {
      errno = 0;
      value = strtoll(str, nullptr, 0);
      if (errno) {
        value = deftVal;
        INFO(NCCL_ALL,"Invalid value %s for %s, using default %lld.", str, env, (long long)deftVal);
      } else {
        INFO(NCCL_ALL,"%s set by environment to %lld.", env, (long long)value);
      }
    }
    __atomic_store_n(cache, value, __ATOMIC_RELAXED);
  }
  pthread_mutex_unlock(&mutex);
}
