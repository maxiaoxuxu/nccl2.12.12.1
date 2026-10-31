/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_UTILS_H_
/* 中文逐项-常量 X1141：NCCL_UTILS_H_
 * 编译期定义：NCCL_UTILS_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_UTILS_H_

#include "nccl.h"
#include "checks.h"
#include <stdint.h>

/* 中文逐项-接口 X1142：ncclCudaCompCap
 * 接口职责：查询当前 CUDA 设备主次计算能力并编码成整数，供协议和架构策略判断。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：int ncclCudaCompCap();
 */
int ncclCudaCompCap();

// PCI Bus ID <-> int64 conversion functions
/* 中文逐项-接口 X1143：int64ToBusId
 * 接口职责：在 PCI BDF 字符串和整数标识之间转换，统一关联 CUDA/NVML/NET 的同一硬件设备。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t int64ToBusId(int64_t id, char* busId);
 */
ncclResult_t int64ToBusId(int64_t id, char* busId);
/* 中文逐项-接口 X1144：busIdToInt64
 * 接口职责：在 PCI BDF 字符串和整数标识之间转换，统一关联 CUDA/NVML/NET 的同一硬件设备。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t busIdToInt64(const char* busId, int64_t* id);
 */
ncclResult_t busIdToInt64(const char* busId, int64_t* id);

/* 中文逐项-接口 X1145：getBusId
 * 接口职责：查询本进程 CUDA 设备的 PCI 地址并转为整数标识，不能把设备 ordinal 当作 busId。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t getBusId(int cudaDev, int64_t *busId);
 */
ncclResult_t getBusId(int cudaDev, int64_t *busId);

/* 中文逐项-接口 X1146：getHostName
 * 接口职责：取得并按分隔符截短主机名，主要用于标识/日志。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t getHostName(char* hostname, int maxlen, const char delim);
 */
ncclResult_t getHostName(char* hostname, int maxlen, const char delim);
/* 中文逐项-接口 X1147：getHash
 * 接口职责：对给定字符串字节计算哈希，供主机/进程身份摘要使用，不承担安全认证。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：uint64_t getHash(const char* string, int n);
 */
uint64_t getHash(const char* string, int n);
/* 中文逐项-接口 X1148：getHostHash
 * 接口职责：由主机/启动环境相关身份生成一致摘要，帮助识别同一主机上的参与者。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：uint64_t getHostHash();
 */
uint64_t getHostHash();
/* 中文逐项-接口 X1149：getPidHash
 * 接口职责：结合进程命名空间/进程标识形成摘要，用于判断是否同进程共享本地指针与发射状态。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：uint64_t getPidHash();
 */
uint64_t getPidHash();

/* 中文逐项-类型 T0385：netIf
 * 类型职责：接口过滤配置项，保存名字前缀和可选端口条件。
 * 声明内可见成员（含嵌套成员）：prefix、port。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct netIf {
/* 中文逐项-变量 V3200：prefix
 * prefix（char；成员 netIf）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 */
  char prefix[64];
/* 中文逐项-变量 V3201：port
 * port（int；成员 netIf）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 */
  int port;
};

/* 中文逐项-接口 X1150：parseStringList
 * 接口职责：把逗号分隔的接口名前缀及可选端口解析成过滤表。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：int parseStringList(const char* string, struct netIf* ifList, int maxList);
 */
int parseStringList(const char* string, struct netIf* ifList, int maxList);
/* 中文逐项-接口 X1151：matchIfList
 * 接口职责：按完整名字/前缀及可选端口规则判断接口是否命中配置列表。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：bool matchIfList(const char* string, int port, struct netIf* ifList, int listSize,
 * bool matchExact);
 */
bool matchIfList(const char* string, int port, struct netIf* ifList, int listSize, bool matchExact);

/* 中文逐项-函数 F0872：log2i
 * 用途与位置：用移位计算整数以2为底的向下取整对数，供树深、性能分桶等离散决策使用。
 * 参数 n（long n）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::computeColl；src/graph/connect.cc::connectTrees；src/graph/tuning.cc::ncclTopoTuneModel；src/graph/tuning.cc::ncclTopoGetAlgoTime。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：n>>=1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return l;。CHECK类宏还可能提前返回/跳转。
 */
static long log2i(long n) {
/* 中文逐项-变量 V3202：l
 * l（long；函数 log2i 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
 long l = 0;
 while (n>>=1) l++;
 return l;
}

// Recyclable list that avoids frequent malloc/free
/* 中文逐项-类型 T0386：ncclListElem
 * 类型职责：可复用节点链及使用边界/游标管理容器，用于高频API排队减少malloc/free。
 * 声明内可见成员（含嵌套成员）：data、next。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<typename T>
struct ncclListElem {
/* 中文逐项-变量 V3203：data
 * data（T；成员 ncclListElem）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 */
  T data;
/* 中文逐项-变量 V3204：next
 * next（struct ncclListElem；成员 ncclListElem）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 */
  struct ncclListElem* next;
};

/* 中文逐项-类型 T0387：ncclRecyclableList
 * 类型职责：可复用节点链及使用边界/游标管理容器，用于高频API排队减少malloc/free。
 * 声明内可见成员（含嵌套成员）：head、tail、cursor、n。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<typename T>
/* 中文精读：排队链表为何 recycle 而非每轮 free
 * head 保存已分配节点链，tail 是当前已用边界，cursor 是遍历进度；recycle 把使用状态归零。
 * getNewElem 优先重用现有节点并清零 data，只在容量不够时扩展，减少高频通信调用的 malloc/free。
 * 队列中保存的是任务/指针等元数据，复用节点不代表可以覆盖仍在 GPU/NIC 使用的用户数据。
 * P2P 列表和 QueueElem 列表都复用此模板，注意 getNext 会移动 cursor。
 */
class ncclRecyclableList {
 private:
/* 中文逐项-变量 V3205：head
 * head（struct ncclListElem<T>；成员 ncclRecyclableList）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 */
  struct ncclListElem<T>* head;
/* 中文逐项-变量 V3206：tail
 * tail（struct ncclListElem<T>；成员 ncclRecyclableList）：生产者已发布的数据进度，消费者据此判断相应片段是否可读。
 */
  struct ncclListElem<T>* tail;
/* 中文逐项-变量 V3207：cursor
 * cursor（struct ncclListElem<T>；成员 ncclRecyclableList）：可复用链表本轮遍历游标，getNext推进，peakNext只查看。
 */
  struct ncclListElem<T>* cursor;
/* 中文逐项-变量 V3208：n
 * n（int；成员 ncclRecyclableList）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 */
  int n;

 public:
/* 中文逐项-函数 F0873：ncclRecyclableList
 * 用途与位置：建立空的可复用链表，分离已有容量、已用边界和遍历游标。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  ncclRecyclableList() {
    tail = cursor = head = NULL;
    n = 0;
  }

/* 中文逐项-函数 F0874：count
 * 用途与位置：返回当前这轮列表已使用的元素数，而不是历史分配容量。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return n;。CHECK类宏还可能提前返回/跳转。
 */
  int count() const { return n; }

  // Get a new element from the list and return pointer
/* 中文逐项-函数 F0875：getNewElem
 * 用途与位置：取可复用节点并清零 data，容量不足时扩展节点链，减少高频排队的动态分配。
 * 参数 dataOut（T** dataOut）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；memset；ncclCalloc。
 * 需要对照的边界/推进条件：tail != NULL；tail->next == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
  ncclResult_t getNewElem(T** dataOut) {
    if (tail != NULL) {
      *dataOut = &tail->data;
      memset(*dataOut, 0, sizeof(T));
    } else {
      NCCLCHECK(ncclCalloc(&tail, 1));
      *dataOut = &tail->data;
      cursor = head = tail;
    }
    if (tail->next == NULL) {
      NCCLCHECK(ncclCalloc(&tail->next, 1));
    }
    tail = tail->next;
    n += 1;
    return ncclSuccess;
  }

/* 中文逐项-函数 F0876：begin
 * 用途与位置：定位本轮首个已用节点并初始化后续遍历游标，空列表返回 NULL。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：head == NULL || head == tail。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return NULL;；return &head->data;。CHECK类宏还可能提前返回/跳转。
 */
  T* begin() {
    if (head == NULL || head == tail) return NULL;
    cursor = head->next;
    return &head->data;
  }

  // Get next element from the list during an iteration
/* 中文逐项-函数 F0877：getNext
 * 用途与位置：返回游标处元素并向后推进，越过本轮已用边界时返回 NULL。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：cursor == NULL || cursor == tail。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return NULL;；return rv;。CHECK类宏还可能提前返回/跳转。
 */
  T* getNext() {
    // tail always points to the next element to be enqueued
    // hence does not contain valid data
    if (cursor == NULL || cursor == tail) return NULL;
/* 中文逐项-变量 V3209：rv
 * rv（T；函数 getNext 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 rv = &cursor->data；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    T* rv = &cursor->data;
    cursor = cursor->next;
    return rv;
  }

/* 中文逐项-函数 F0878：peakNext
 * 用途与位置：只查看下一个元素是否存在而不推进游标，供队列耗尽后回收判断。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：cursor == NULL || cursor == tail。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return NULL;；return &cursor->data;。CHECK类宏还可能提前返回/跳转。
 */
  T* peakNext() {
    if (cursor == NULL || cursor == tail) return NULL;
    return &cursor->data;
  }

  // Recycle the list without freeing the space
/* 中文逐项-函数 F0879：recycle
 * 用途与位置：把本轮已用边界/游标复位到已分配链头，下轮复用现有节点而不释放容量。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  void recycle() {
    tail = cursor = head;
    n = 0;
  }

/* 中文逐项-函数 F0880：~ncclRecyclableList
 * 用途与位置：遍历并释放整条已分配链，和仅重置使用状态的 recycle 区分。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：free。
 * 需要对照的边界/推进条件：head != NULL。各条件所属分支见紧邻函数体。
 */
  ~ncclRecyclableList() {
    while (head != NULL) {
/* 中文逐项-变量 V3210：temp
 * temp（struct ncclListElem<T>；函数 ~ncclRecyclableList 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 temp = head；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      struct ncclListElem<T>* temp = head;
      head = head->next;
      free(temp);
    }
  }
};

#endif
