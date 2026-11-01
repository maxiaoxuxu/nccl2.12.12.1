<!-- NCCL-BEGINNER-ANNOTATED-V3 -->
# 中文逐项注释版：先看这里

**源码内已包含中文分析。请先打开 [START_HERE_ZH.md](START_HERE_ZH.md)。**

当前注释分支：`codex/nccl-2.12.12-full-source-walkthrough`。默认 `master` 尚未合并这些注释；直接从默认分支下载会得到原版。

[详细图解](docs/zh_CN/01_通信原语与算法图解.md) · [全部函数内部步骤](docs/zh_CN/05_全部函数内部步骤_文件索引.md) · [全部函数/类型索引](docs/zh_CN/07_函数与类型定义索引.md) · [按问题阅读](docs/zh_CN/06_按问题查找源码.md) · [本地完整性检查](tools/verify_annotations.py)

---

# NCCL

Optimized primitives for inter-GPU communication.

## Introduction

NCCL (pronounced "Nickel") is a stand-alone library of standard communication routines for GPUs, implementing all-reduce, all-gather, reduce, broadcast, reduce-scatter, as well as any send/receive based communication pattern. It has been optimized to achieve high bandwidth on platforms using PCIe, NVLink, NVswitch, as well as networking using InfiniBand Verbs or TCP/IP sockets. NCCL supports an arbitrary number of GPUs installed in a single node or across multiple nodes, and can be used in either single- or multi-process (e.g., MPI) applications.

For more information on NCCL usage, please refer to the [NCCL documentation](https://docs.nvidia.com/deeplearning/sdk/nccl-developer-guide/index.html).

## Build

Note: the official and tested builds of NCCL can be downloaded from: https://developer.nvidia.com/nccl. You can skip the following build steps if you choose to use the official builds.

To build the library :

```shell
$ cd nccl
$ make -j src.build
```

If CUDA is not installed in the default /usr/local/cuda path, you can define the CUDA path with :

```shell
$ make src.build CUDA_HOME=<path to cuda install>
```

NCCL will be compiled and installed in `build/` unless `BUILDDIR` is set.

By default, NCCL is compiled for all supported architectures. To accelerate the compilation and reduce the binary size, consider redefining `NVCC_GENCODE` (defined in `makefiles/common.mk`) to only include the architecture of the target platform :
```shell
$ make -j src.build NVCC_GENCODE="-gencode=arch=compute_70,code=sm_70"
```

## Install

To install NCCL on the system, create a package then install it as root.

Debian/Ubuntu :
```shell
$ # Install tools to create debian packages
$ sudo apt install build-essential devscripts debhelper fakeroot
$ # Build NCCL deb package
$ make pkg.debian.build
$ ls build/pkg/deb/
```

RedHat/CentOS :
```shell
$ # Install tools to create rpm packages
$ sudo yum install rpm-build rpmdevtools
$ # Build NCCL rpm package
$ make pkg.redhat.build
$ ls build/pkg/rpm/
```

OS-agnostic tarball :
```shell
$ make pkg.txz.build
$ ls build/pkg/txz/
```

## Tests

Tests for NCCL are maintained separately at https://github.com/nvidia/nccl-tests.

```shell
$ git clone https://github.com/NVIDIA/nccl-tests.git
$ cd nccl-tests
$ make
$ ./build/all_reduce_perf -b 8 -e 256M -f 2 -g <ngpus>
```

## Copyright

All source code and accompanying documentation is copyright (c) 2015-2020, NVIDIA CORPORATION. All rights reserved.

## 中文源码导读

本仓库的中文注释、调用流程与关键数据结构说明见 [NCCL 2.12.12 中文源码导读](docs/zh_CN/README.md)。其中包含 [初始化与 GPU 执行](docs/zh_CN/03_初始化_Channel生成与Kernel执行.md) 及 [Proxy、IB 和 QP/MR 核算](docs/zh_CN/04_Proxy_RDMA与QP_MR资源核算.md)。


中文阅读入口：[00_从这里开始_NCCL源码阅读指南.md](00_从这里开始_NCCL源码阅读指南.md)。


V3.3：[源码注释与图示对照（离线单文件）](docs/zh_CN/11_源码注释与图示对照_离线版.html) · [绑定使用说明](docs/zh_CN/10_源码与图示绑定_使用说明.md)。125处人工绑定覆盖100个关键符号，45张图全部可反查源码。


V3.3.1使用引导：[使用方法与编号速查](docs/zh_CN/12_使用方法与编号说明_离线版.html)。源码对照页顶部新增“使用方法”和“编号说明”，主要选项按函数名和中文主题显示；原定位号及链接继续有效。
