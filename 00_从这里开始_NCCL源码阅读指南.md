# 从这里开始：NCCL 2.12.12 中文源码与图解

版本：V3.3.2（2026-11-01），标记 `NCCL-BEGINNER-ANNOTATED-V3`。分析分支：`codex/nccl-2.12.12-full-source-walkthrough`；源码基线：`04832308d0885226169b9376c1ba4f17b0cb6484`。

## 下载后先打开哪一个

| 文件 | 用途 | 下载方式 |
| --- | --- | --- |
| [12_使用方法与编号说明_离线版.html](docs/zh_CN/12_使用方法与编号说明_离线版.html) | **第一次使用：打开方法、操作步骤、F/T/V/B等定位号和C0/X0图示记号速查** | 单独下载也可离线阅读；11页内另有“使用方法/编号说明”按钮 |
| [13_SourceInsight与VSCode_源码图示跳转_离线版.html](docs/zh_CN/13_SourceInsight与VSCode_源码图示跳转_离线版.html) | **在Source Insight或VS Code中，按当前文件/光标行跳到对应图与分析** | 说明可单独阅读；实际跳转需要完整ZIP与Python，见13的配置步骤 |
| [11_源码注释与图示对照_离线版.html](docs/zh_CN/11_源码注释与图示对照_离线版.html) | **左侧完整源码及中文注释，右侧当前代码对应图与分析；支持双向跳转** | 日常主入口；这一个HTML可单独下载、断网阅读 |
| [01_通信原语与算法图解_离线版.html](docs/zh_CN/01_通信原语与算法图解_离线版.html) | 45张SVG、三种Ring逐轮切换、七类原语和修正说明 | **只下载这一个HTML也能看全部图片和操作交互** |
| [05_全部函数内部步骤_离线搜索.html](docs/zh_CN/05_全部函数内部步骤_离线搜索.html) | 搜索1178个函数，展开内部步骤和完整函数体 | 这个HTML也可单独下载、断网使用 |
| `src/`、`ext-net/` | 直接在代码旁阅读函数、类型、变量和关键路径注释 | 下载整个分析分支ZIP并完整解压 |

GitHub不会直接运行仓库中的HTML：打开文件页面，使用 Download raw file 下载，再用Edge/Chrome/Firefox打开。HTML正文和图片不需要网络；标有“在线”的源码及其他章节链接需要网络。完整ZIP中的Markdown与源码可用本地编辑器阅读。

[源码与图示绑定使用说明](docs/zh_CN/10_源码与图示绑定_使用说明.md)：125处绑定覆盖100个关键函数/结构体，45张图全部可反查；可按B编号或函数/变量编号定位。

旧名 `visual-guide.html` 和 `function-logic.html` 仍提供相同的单文件版本，兼容之前的下载入口。旧Markdown文件保留到新名称的指引。

## 文档名称与内容

| 顺序 | 文件 | 内容 |
| --- | --- | --- |
| 01 | [通信原语与算法图解](docs/zh_CN/01_通信原语与算法图解.md) | 分片、slice/step、七类原语、Simple/LL/LL128、Ring/Tree/CollNet/P2P |
| 02 | [Ring五类集合通信源码精读](docs/zh_CN/02_Ring五类集合通信_源码精读.md) | AllReduce、AllGather、ReduceScatter、Broadcast、Reduce逐段推导 |
| 03 | [初始化、Channel生成与Kernel执行](docs/zh_CN/03_初始化_Channel生成与Kernel执行.md) | 拓扑搜索、图拼接、调优、入队、Group、CUDA Graph与kernel提交 |
| 04 | [Proxy、RDMA与QP/MR资源核算](docs/zh_CN/04_Proxy_RDMA与QP_MR资源核算.md) | 网络状态机、IB连接、注册缓存、flush、资源作用域和计数 |
| 05 | [全部函数内部步骤文件索引](docs/zh_CN/05_全部函数内部步骤_文件索引.md) | 按文件查找函数与分支、循环和完整函数体 |
| 06 | [按问题查找源码](docs/zh_CN/06_按问题查找源码.md) | 需求/问题到源码入口的映射，以及逐文件覆盖表 |
| 07 | [函数与类型定义索引](docs/zh_CN/07_函数与类型定义索引.md) | F/T编号定位；变量说明在源码V编号旁 |
| 08 | [核验方法与已知边界](docs/zh_CN/08_核验方法与已知边界.md) | 可重跑检查、测试范围与尚未验证内容 |
| 09 | [本轮修正记录与依据](docs/zh_CN/09_本轮修正记录与依据.md) | 修正前的问题、准确含义及源码证据 |
| 10 | [源码与图示绑定](docs/zh_CN/10_源码与图示绑定_使用说明.md) | 图示位置与源码的对应关系及维护方法 |
| 11 | [源码注释与图示对照](docs/zh_CN/11_源码注释与图示对照_离线版.html) | 在同一页阅读源码、注释、图示和分析 |
| 12 | [使用方法与编号说明](docs/zh_CN/12_使用方法与编号说明.md) | 第一次怎么打开、具体操作、注释编号与图示符号解释、常见问题 |
| 13 | [Source Insight与VS Code源码图示跳转](docs/zh_CN/13_SourceInsight与VSCode_源码图示跳转.md) | 项目任务、自定义命令、可选快捷键、当前文件/行号定位与排错 |

`docs/zh_CN/logic/`保存按源文件展开的步骤；`visuals/`保存可单独编辑的SVG与逐轮数据模型。`annotation-coverage.json`是注释符号清单，`source-checksums.json`用于验证原始源码未变，`function-logic.json`是函数搜索页的生成输入。

## 源码中的注释标记

| 标记 | 阅读内容 |
| --- | --- |
| `中文逐项-函数 F编号` | 函数用途、参数、调用候选和条件 |
| `中文逐项-类型 T编号` | 结构体、类、联合体、枚举及成员 |
| `中文逐项-变量 V编号` | 字段/局部变量的类型、作用域与来源 |
| `中文内部逻辑 F编号` | 静态语句与控制结构展开 |
| `中文精读`、`中文导读` | 关键算法、协议、地址和因果关系的人工讲解 |

覆盖116个C/C++/CUDA文件、1178个函数定义、460个类型定义、4930个变量项与7881个注释ID。数量包含附带SDK、ABI和条件分支；**数量覆盖不等于每个函数均已手工精读、语义全部被证明正确。** V3.2对分片/原语/编号/Proxy/资源关键路径复核，见09；V3.3增加源码与图示绑定，见10和11。

## 推荐阅读顺序

先看01的输入输出、分片和原语，再看02的跨rank流程。随后从 `src/collectives/*.cc` 跟到 `misc/argcheck.cc → enqueue.cc → group.cc → collectives/device/`。回看 `init.cc → graph/` 理解channel的来源，最后阅读 `proxy.cc → transport/net.cc → net_ib.cc`。

## 下载后自检

```bash
python3 tools/verify_annotations.py
python3 tools/verify_visual_models.py
python3 tools/verify_offline_docs.py
python3 tools/verify_source_diagram_bindings.py
python3 tools/check_editor_diagram_jump.py
```

Windows可将python3改为python。浏览器回归检查另见08；无须为了阅读HTML安装Python或Node。

本轮仍未执行CUDA编译、GPU正确性或多机性能测试。现有静态步骤不是编译器CFG、模板实例化或并发内存模型证明。
