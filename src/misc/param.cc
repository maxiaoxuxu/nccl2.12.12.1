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

/* 中文逐项-函数 F0957：userHomeDir
 * 用途与位置：按当前用户 id 查询用户目录，给用户级 NCCL 配置文件构造路径。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/param.cc::initEnv。
 * 本体直接出现的调用/包装：getpwuid；getuid。
 * 源码返回表达式：return pwUser == NULL ? NULL : pwUser->pw_dir;。CHECK类宏还可能提前返回/跳转。
 */
const char* userHomeDir() {
/* 中文逐项-变量 V3417：pwUser
 * pwUser（struct passwd；函数 userHomeDir 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 pwUser = getpwuid(getuid())；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  struct passwd *pwUser = getpwuid(getuid());
  return pwUser == NULL ? NULL : pwUser->pw_dir;
}

/* 中文逐项-函数 F0958：setEnvFile
 * 用途与位置：读取 KEY=VALUE 配置行并只填入尚未存在的环境键，保留启动环境的优先级。
 * 参数 fileName（const char* fileName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/param.cc::initEnv。
 * 本体直接出现的调用/包装：fclose；fopen；free；getline；setenv；std::min；strncpy。
 * 需要对照的边界/推进条件：file == NULL；read = getline(&line, &n, file)) != -1；line[read-1] == '\n'；line[s] != '\0' && line[s] !=
 * '='。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return;。CHECK类宏还可能提前返回/跳转。
 */
void setEnvFile(const char* fileName) {
/* 中文逐项-变量 V3418：file
 * file（FILE；函数 setEnvFile 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 初值/来源 file = fopen(fileName, "r")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  FILE * file = fopen(fileName, "r");
  if (file == NULL) return;

/* 中文逐项-变量 V3419：line
 * line（char；函数 setEnvFile 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 初值/来源 line = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  char *line = NULL;
/* 中文逐项-变量 V3420：envVar
 * envVar（char；函数 setEnvFile 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：strncpy(envVar, line, std::min(1023,s));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char envVar[1024];
/* 中文逐项-变量 V3421：envValue
 * envValue（char；函数 setEnvFile 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：strncpy(envValue, line+s, 1023);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char envValue[1024];
/* 中文逐项-变量 V3422：n
 * n（size_t；函数 setEnvFile 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  size_t n = 0;
/* 中文逐项-变量 V3423：read
 * read（ssize_t；函数 setEnvFile 内）：P2P/GDR方向是否采用读/拉取策略，发送读显存和接收写显存可能有不同能力条件。
 * 此处未显式给初值；后续首个文本使用点：while ((read = getline(&line, &n, file)) != -1) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ssize_t read;
  while ((read = getline(&line, &n, file)) != -1) {
    if (line[read-1] == '\n') line[read-1] = '\0';
/* 中文逐项-变量 V3424：s
 * s（int；函数 setEnvFile 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
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
/* 中文逐项-函数 F0959：initEnv
 * 用途与位置：按用户配置、系统配置顺序加载配置文件，已有键不会被后读文件覆盖。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclInit。
 * 本体直接出现的调用/包装：setEnvFile；sprintf；userHomeDir。
 * 需要对照的边界/推进条件：userDir。各条件所属分支见紧邻函数体。
 */
void initEnv() {
/* 中文逐项-变量 V3425：confFilePath
 * confFilePath（char；函数 initEnv 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：sprintf(confFilePath, "%s/.nccl.conf", userDir);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char confFilePath[1024];
/* 中文逐项-变量 V3426：userDir
 * userDir（char；函数 initEnv 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 userDir = userHomeDir()；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  const char * userDir = userHomeDir();
  if (userDir) {
    sprintf(confFilePath, "%s/.nccl.conf", userDir);
    setEnvFile(confFilePath);
  }
  sprintf(confFilePath, "/etc/nccl.conf");
  setEnvFile(confFilePath);
}

/* 中文逐项-函数 F0960：ncclLoadParam
 * 用途与位置：在互斥锁内首次读取/解析数值参数并写原子缓存，后续 getter 直接复用缓存值。
 * 参数 env（char const* env）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 deftVal（int64_t deftVal）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 uninitialized（int64_t uninitialized）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 cache（int64_t* cache）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__atomic_load_n；__atomic_store_n；getenv；pthread_mutex_lock；pthread_mutex_unlock；strlen；strtoll。
 * 需要对照的边界/推进条件：__atomic_load_n(cache, __ATOMIC_RELAXED) == uninitialized；str && strlen(str) > 0；errno。各条件所属分支见紧邻函数体。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
void ncclLoadParam(char const* env, int64_t deftVal, int64_t uninitialized, int64_t* cache) {
/* 中文逐项-变量 V3427：mutex
 * mutex（pthread_mutex_t；函数 ncclLoadParam 内）：CPU 线程/进程间同步原语；锁保护元数据，条件变量唤醒等待，与 GPU head/tail 协议不同。
 * 初值/来源 mutex = PTHREAD_MUTEX_INITIALIZER；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(&mutex);
  if (__atomic_load_n(cache, __ATOMIC_RELAXED) == uninitialized) {
/* 中文逐项-变量 V3428：str
 * str（char；函数 ncclLoadParam 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 初值/来源 str = getenv(env)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    char* str = getenv(env);
/* 中文逐项-变量 V3429：value
 * value（int64_t；函数 ncclLoadParam 内）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 初值/来源 value = deftVal；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
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
