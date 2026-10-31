/*
* Copyright 2009-2020  NVIDIA Corporation.  All rights reserved.
*
* Licensed under the Apache License v2.0 with LLVM Exceptions.
* See https://llvm.org/LICENSE.txt for license information.
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

#ifndef __NVTX_LINKONCE_H__
/* 中文逐项-常量 X0905：__NVTX_LINKONCE_H__
 * 编译期定义：__NVTX_LINKONCE_H__ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define __NVTX_LINKONCE_H__

/* This header defines macros to permit making definitions of global variables
 * and functions in C/C++ header files which may be included multiple times in
 * a translation unit or linkage unit.  It allows authoring header-only libraries
 * which can be used by multiple other header-only libraries (either as the same
 * copy or multiple copies), and does not require any build changes, such as
 * adding another .c file, linking a static library, or deploying a dynamic
 * library.  Globals defined with these macros have the property that they have
 * the same address, pointing to a single instance, for the entire linkage unit.
 * It is expected but not guaranteed that each linkage unit will have a separate
 * instance.
 *
 * In some situations it is desirable to declare a variable without initializing
 * it, refer to it in code or other variables' initializers, and then initialize
 * it later.  Similarly, functions can be prototyped, have their address taken,
 * and then have their body defined later.  In such cases, use the FWDDECL macros 
 * when forward-declaring LINKONCE global variables without initializers and
 * function prototypes, and then use the DEFINE macros when later defining them.
 * Although in many cases the FWDDECL macro is equivalent to the DEFINE macro,
 * following this pattern makes code maximally portable.
 */

#if defined(__MINGW32__) /* MinGW */
/* 中文逐项-常量 X0906：NVTX_LINKONCE_WEAK
 * 编译期定义：NVTX_LINKONCE_WEAK = __attribute__((section(".gnu.linkonce.0.")))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define NVTX_LINKONCE_WEAK __attribute__((section(".gnu.linkonce.0.")))
    #if defined(__cplusplus)
/* 中文逐项-常量 X0907：NVTX_LINKONCE_DEFINE_GLOBAL
 * 编译期定义：NVTX_LINKONCE_DEFINE_GLOBAL = __declspec(selectany)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_GLOBAL   __declspec(selectany)
/* 中文逐项-常量 X0908：extern
 * 编译期定义：extern = "C" inline NVTX_LINKONCE_WEAK。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_FUNCTION extern "C" inline NVTX_LINKONCE_WEAK
    #else
/* 中文逐项-常量 X0909：NVTX_LINKONCE_DEFINE_GLOBAL
 * 编译期定义：NVTX_LINKONCE_DEFINE_GLOBAL = __declspec(selectany)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_GLOBAL   __declspec(selectany)
/* 中文逐项-常量 X0910：NVTX_LINKONCE_WEAK
 * 编译期定义：NVTX_LINKONCE_WEAK = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_FUNCTION NVTX_LINKONCE_WEAK
    #endif
#elif defined(_MSC_VER) /* MSVC */
    #if defined(__cplusplus)
/* 中文逐项-常量 X0911：NVTX_LINKONCE_DEFINE_GLOBAL
 * 编译期定义：NVTX_LINKONCE_DEFINE_GLOBAL = extern "C" __declspec(selectany)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_GLOBAL   extern "C" __declspec(selectany)
/* 中文逐项-常量 X0912：extern
 * 编译期定义：extern = "C" inline。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_FUNCTION extern "C" inline
    #else
/* 中文逐项-常量 X0913：NVTX_LINKONCE_DEFINE_GLOBAL
 * 编译期定义：NVTX_LINKONCE_DEFINE_GLOBAL = __declspec(selectany)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_GLOBAL   __declspec(selectany)
/* 中文逐项-常量 X0914：__inline
 * 编译期定义：__inline = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_FUNCTION __inline
    #endif
#elif defined(__CYGWIN__) && defined(__clang__) /* Clang on Cygwin */
/* 中文逐项-常量 X0915：NVTX_LINKONCE_WEAK
 * 编译期定义：NVTX_LINKONCE_WEAK = __attribute__((section(".gnu.linkonce.0.")))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define NVTX_LINKONCE_WEAK __attribute__((section(".gnu.linkonce.0.")))
    #if defined(__cplusplus)
/* 中文逐项-常量 X0916：NVTX_LINKONCE_DEFINE_GLOBAL
 * 编译期定义：NVTX_LINKONCE_DEFINE_GLOBAL = NVTX_LINKONCE_WEAK。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_GLOBAL   NVTX_LINKONCE_WEAK
/* 中文逐项-常量 X0917：extern
 * 编译期定义：extern = "C" NVTX_LINKONCE_WEAK。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_FUNCTION extern "C" NVTX_LINKONCE_WEAK
    #else
/* 中文逐项-常量 X0918：NVTX_LINKONCE_DEFINE_GLOBAL
 * 编译期定义：NVTX_LINKONCE_DEFINE_GLOBAL = NVTX_LINKONCE_WEAK。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_GLOBAL   NVTX_LINKONCE_WEAK
/* 中文逐项-常量 X0919：NVTX_LINKONCE_WEAK
 * 编译期定义：NVTX_LINKONCE_WEAK = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_FUNCTION NVTX_LINKONCE_WEAK
    #endif
#elif defined(__CYGWIN__) /* Assume GCC or compatible */
/* 中文逐项-常量 X0920：NVTX_LINKONCE_WEAK
 * 编译期定义：NVTX_LINKONCE_WEAK = __attribute__((weak))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define NVTX_LINKONCE_WEAK __attribute__((weak))
    #if defined(__cplusplus)
/* 中文逐项-常量 X0921：NVTX_LINKONCE_DEFINE_GLOBAL
 * 编译期定义：NVTX_LINKONCE_DEFINE_GLOBAL = __declspec(selectany)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_GLOBAL   __declspec(selectany)
/* 中文逐项-常量 X0922：extern
 * 编译期定义：extern = "C" inline。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_FUNCTION extern "C" inline
    #else
/* 中文逐项-常量 X0923：NVTX_LINKONCE_DEFINE_GLOBAL
 * 编译期定义：NVTX_LINKONCE_DEFINE_GLOBAL = NVTX_LINKONCE_WEAK。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_GLOBAL   NVTX_LINKONCE_WEAK
/* 中文逐项-常量 X0924：NVTX_LINKONCE_WEAK
 * 编译期定义：NVTX_LINKONCE_WEAK = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_FUNCTION NVTX_LINKONCE_WEAK
    #endif
#else /* All others: Assume GCC, clang, or compatible */
/* 中文逐项-常量 X0925：NVTX_LINKONCE_WEAK
 * 编译期定义：NVTX_LINKONCE_WEAK = __attribute__((weak))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define NVTX_LINKONCE_WEAK   __attribute__((weak))
/* 中文逐项-常量 X0926：NVTX_LINKONCE_HIDDEN
 * 编译期定义：NVTX_LINKONCE_HIDDEN = __attribute__((visibility("hidden")))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define NVTX_LINKONCE_HIDDEN __attribute__((visibility("hidden")))
    #if defined(__cplusplus)
/* 中文逐项-常量 X0927：NVTX_LINKONCE_DEFINE_GLOBAL
 * 编译期定义：NVTX_LINKONCE_DEFINE_GLOBAL = NVTX_LINKONCE_HIDDEN NVTX_LINKONCE_WEAK。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_GLOBAL   NVTX_LINKONCE_HIDDEN NVTX_LINKONCE_WEAK
/* 中文逐项-常量 X0928：extern
 * 编译期定义：extern = "C" NVTX_LINKONCE_HIDDEN inline。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_FUNCTION extern "C" NVTX_LINKONCE_HIDDEN inline
    #else
/* 中文逐项-常量 X0929：NVTX_LINKONCE_DEFINE_GLOBAL
 * 编译期定义：NVTX_LINKONCE_DEFINE_GLOBAL = NVTX_LINKONCE_HIDDEN NVTX_LINKONCE_WEAK。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_GLOBAL   NVTX_LINKONCE_HIDDEN NVTX_LINKONCE_WEAK
/* 中文逐项-常量 X0930：NVTX_LINKONCE_HIDDEN
 * 编译期定义：NVTX_LINKONCE_HIDDEN = NVTX_LINKONCE_WEAK。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX_LINKONCE_DEFINE_FUNCTION NVTX_LINKONCE_HIDDEN NVTX_LINKONCE_WEAK
    #endif
#endif

/* 中文逐项-常量 X0931：NVTX_LINKONCE_FWDDECL_GLOBAL
 * 编译期定义：NVTX_LINKONCE_FWDDECL_GLOBAL = NVTX_LINKONCE_DEFINE_GLOBAL extern。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_LINKONCE_FWDDECL_GLOBAL   NVTX_LINKONCE_DEFINE_GLOBAL   extern
#define NVTX_LINKONCE_FWDDECL_FUNCTION NVTX_LINKONCE_DEFINE_FUNCTION

#endif /* __NVTX_LINKONCE_H__ */
