#!/bin/bash
#
# Copyright (c) 2018-2021, NVIDIA CORPORATION. All rights reserved.
#
# See LICENSE.txt for license information
#

# 中文构建：dir来自第1个参数，指定生成目标文件目录；脚本只输出Make规则，不在这里调用nvcc。
dir=$1

# 中文构建：datatypes按公开ncclDataType_t枚举顺序列出类型缩写；CUDA>=11才追加bf16。
datatypes="i8 u8 i32 u32 i64 u64 f16 f32 f64"
if [ "$CUDA_MAJOR" -ge 11 ]
then
    datatypes+=" bf16"
fi

# 中文构建：targets累积GENOBJS对象文件清单，转义的换行/反斜线是输出Makefile语法，不能随意删改。
targets="GENOBJS := \\\\\n"

# 中文构建：base遍历6类设备实现文件；同一.cu用不同NCCL_OP/NCCL_TYPE多次编译产生模板实例。
for base in sendrecv all_reduce all_gather broadcast reduce reduce_scatter; do
# 中文构建：opn对应构建时归约类型编号；op是目标文件名缩写。顺序由common.h的NCCL_OP分支解释。
  opn=0
  for op in sum prod min max premulsum sumpostdiv; do
# 中文构建：dtn是本归约下从0开始的数据类型编号，dt是名字；每次切换op重新从0遍历类型。
    dtn=0
    # Order must match that of the ncclDataType_t enum
    for dt in ${datatypes}; do
      echo "${dir}/${base}_${op}_${dt}.o : ${base}.cu ${dir}/${base}.dep"
      echo "	@printf \"Compiling  %-35s > %s\\\\n\" ${base}.cu ${dir}/${base}_${op}_${dt}.o"
      echo "	mkdir -p ${dir}"
      echo "	\${NVCC} -DNCCL_OP=${opn} -DNCCL_TYPE=${dtn} \${NVCUFLAGS} -dc ${base}.cu -o ${dir}/${base}_${op}_${dt}.o"
      echo ""
      targets="$targets\t${dir}/${base}_${op}_${dt}.o \\\\\n"
      dtn=$(($dtn + 1))
    done
    opn=$(($opn + 1))
  done
done
# 中文构建：最终输出完整对象列表；标准输出会被设备Makefile重定向保存成Makefile.rules。
echo -e "$targets"
