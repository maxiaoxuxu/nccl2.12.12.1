# 从这里读：NCCL 2.12.12 中文逐项注释版

你解压得到的是包含分析文字的源码。无需另开ChatGPT，也无需先编译：打开 `.cc`、`.h`、`.cu`、`.hpp` 就能看到中文注释。

**本包识别标记：`NCCL-BEGINNER-ANNOTATED-V3`。** 如在代码中搜索不到 `中文逐项-函数`，说明打开了旧版文件或master原版。

## V3新增：图解与函数内部步骤

双击打开 [visual-guide.html](docs/zh_CN/visual-guide.html)，即可离线阅读42张图，并逐轮查看Ring AllReduce、AllGather、ReduceScatter的块与贡献来源。GitHub网页不直接运行HTML，请下载解压后打开；在线可读 [Markdown图解](docs/zh_CN/visual-guide.md)。

图解涵盖channel分片与尾块、七种基础原语、Simple线程分工/等待/屏障/发布、LL与LL128标记、direct指针交换、Tree/CollNet/P2P与Proxy。

[离线函数搜索](docs/zh_CN/function-logic.html) 与 [逐文件函数内部步骤](docs/zh_CN/function-logic-index.md) 覆盖1178个函数、15912条语句/控制步骤。**全部步骤也写入函数前的 `中文内部逻辑 F编号` 注释，直接打开源码即可阅读。** 关键函数有手工精读；静态步骤保留真实表达式和嵌套，但不是编译器CFG或并发执行证明。

## 先确认下载与查看方式

- GitHub请选择分支 `codex/nccl-2.12.12-full-source-walkthrough`。仓库默认的master尚未合并这些注释，默认页面/默认Download ZIP可能仍是原版。
- 使用交付消息提供的固定提交ZIP链接，或本地注释包；不要根据仓库名字猜分支。
- 解压后在本目录打开 `src/collectives/all_reduce.cc`，搜索 `中文逐项-函数`，应看到ncclAllReduce用途、每个参数、调用入口以及内部info说明。
- 再打开 `src/include/devcomm.h`，每个结构体/联合体及字段旁有 `中文逐项-类型` / `中文逐项-变量`。文件编码为UTF-8。

## 代码旁有什么

| 标记 | 包含内容 |
| --- | --- |
| 中文内部逻辑 F编号 | 内部语句、条件、循环、跳转、写入与返回；缩进显示控制嵌套，编号是阅读顺序 |
| 中文逐项-函数 F编号 | 该定义的用途、形参、静态调用候选、调用接口、关键条件及返回路径 |
| 中文逐项-类型 T编号 | 结构/联合/枚举职责、成员列表及存储/类型规则 |
| 中文逐项-变量 V编号 | 字段、局部及文件变量的作用、声明类型、作用域、初值来源或后续使用点；for变量还列条件 |
| 中文逐项-接口/宏/参数getter/常量/枚举项 | 声明与定义的区别、编译期生成、环境值/默认值及协议取值 |
| 中文精读 / 中文导读 | 串联调用流程、设计原因、分片公式和完整数值例子 |

本次符号清单包含 **1178个函数定义、460个结构体/类/联合体/枚举定义、4930个变量项**，覆盖116个C/C++/CUDA文件。函数数含重载、模板特化与不同条件编译分支；类型数含匿名嵌套类型。字段/局部变量同名时按各自声明分别记录。模板宏会生成更多实例，不把它们重复算成文本定义。

[全部函数与类型索引](docs/zh_CN/symbol-index.md) · [按问题阅读源码](docs/zh_CN/source-map.md) · [机器可读清单](docs/zh_CN/annotation-coverage.json)

## 初学者顺序

1. 从公开API的输入输出例子开始：`src/collectives/*.cc`。
2. 看检查、选型、分片与发射：`misc/argcheck.cc` → `enqueue.cc` → `group.cc`。
3. 看设备如何解释任务：`include/devcomm.h` → `collectives/device/common.h`。
4. 看每种算法怎样定位块并调用原语：`collectives/device/`各API同名头文件。
5. 看原语怎样等、读、归约、写、发布：`prims_simple.h` → `common_kernel.h`，再比较LL/LL128。
6. 回看初始化与channel来源：`init.cc` → `graph/topo.cc` → `paths.cc` → `search.cc` → `connect.cc`。
7. 看跨机CPU/GPU如何配合：`proxy.cc` → `transport/net.cc` → `net_ib.cc`或`net_socket.cc`。

需要画出全链时配合：[总体架构](docs/zh_CN/README.md)、[初始化与执行](docs/zh_CN/execution.md)、[Proxy与RDMA](docs/zh_CN/rdma.md)。附带NVTX是性能标记代码，其注释明确说明不负责通信与同步。

## 核验与解释边界

运行 `python3 tools/verify_annotations.py`（Windows可用 `python tools/verify_annotations.py`）。工具检查每个清单ID存在、发布源码校验和及移除新增注释后的原始源码校验和。

符号清点用于防止遗漏，不能代替正确性论证；同名静态调用检索没有完整解析C++重载/间接调用，已在注释中标明“候选”。注释给出代码当前的尺寸、条件和使用点，不把变量名猜测当作运行事实。基线为 `04832308d0885226169b9376c1ba4f17b0cb6484`，版本2.12.12、打包修订1。当前环境未进行nvcc编译和GPU/多机运行验证。
