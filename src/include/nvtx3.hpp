/*
 *  Copyright (c) 2020, NVIDIA CORPORATION.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/* Temporary helper #defines, #undef'ed at end of header */
/* 中文逐项-常量 X1029：NVTX3_CPP_VERSION_MAJOR
 * 编译期定义：NVTX3_CPP_VERSION_MAJOR = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX3_CPP_VERSION_MAJOR 1
/* 中文逐项-常量 X1030：NVTX3_CPP_VERSION_MINOR
 * 编译期定义：NVTX3_CPP_VERSION_MINOR = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX3_CPP_VERSION_MINOR 0

/* This section handles the decision of whether to provide unversioned symbols.
 * If NVTX3_CPP_REQUIRE_EXPLICIT_VERSION is #defined, unversioned symbols are
 * not provided, and explicit-version symbols such as nvtx3::v1::thread_range
 * and NVTX3_V1_FUNC_RANGE must be used.  By default, the first #include of this
 * header will define the unversioned symbols such as nvtx3::thread_range and
 * NVTX3_FUNC_RANGE.  Subsequently including a different major version of this
 * header without #defining NVTX3_CPP_REQUIRE_EXPLICIT_VERSION triggers an error
 * since the symbols would conflict.  Subsequently including of a different
 * minor version within the same major version is allowed. Functionality of
 * minor versions is cumulative, regardless of include order.
 * 
 * Since NVTX3_CPP_REQUIRE_EXPLICIT_VERSION allows all combinations of versions
 * to coexist without problems within a translation unit, the recommended best
 * practice for instrumenting header-based libraries with NVTX C++ Wrappers is
 * is to #define NVTX3_CPP_REQUIRE_EXPLICIT_VERSION before including nvtx3.hpp,
 * #undef it afterward, and only use explicit-version symbols.  This is not
 * necessary in common cases, such as instrumenting a standalone application, or
 * static/shared libraries in .cpp files or headers private to those projects.
 */
/* clang-format off */
#if !defined(NVTX3_CPP_REQUIRE_EXPLICIT_VERSION)
    /* Define macro used by all definitions in this header to indicate the
     * unversioned symbols should be defined in addition to the versioned ones.
     */
/* 中文逐项-常量 X1031：NVTX3_INLINE_THIS_VERSION
 * 编译期定义：NVTX3_INLINE_THIS_VERSION = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define NVTX3_INLINE_THIS_VERSION

    #if !defined(NVTX3_CPP_INLINED_VERSION_MAJOR)
        /* First occurrence of this header in the translation unit.  Define macros
         * indicating which version shall be used for unversioned symbols.
         */

        /**
         * @brief Semantic major version number for NVTX C++ wrappers of unversioned symbols
         *
         * Breaking changes may occur between major versions, and different major versions
         * cannot provide unversioned symbols in the same translation unit (.cpp file).
         * 
         * Note: If NVTX3_CPP_REQUIRE_EXPLICIT_VERSION is defined, this macro is not defined.
         *
         * Not to be confused with the version number of the NVTX core library.
         */
/* 中文逐项-常量 X1032：NVTX3_CPP_INLINED_VERSION_MAJOR
 * 编译期定义：NVTX3_CPP_INLINED_VERSION_MAJOR = 1 // NVTX3_CPP_VERSION_MAJOR。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX3_CPP_INLINED_VERSION_MAJOR 1  // NVTX3_CPP_VERSION_MAJOR

        /**
         * @brief Semantic minor version number for NVTX C++ wrappers of unversioned symbols
         *
         * No breaking changes occur between minor versions -- minor version changes within
         * a major version are purely additive.
         *
         * Note: If NVTX3_CPP_REQUIRE_EXPLICIT_VERSION is defined, this macro is not defined.
         *
         * Not to be confused with the version number of the NVTX core library.
         */
/* 中文逐项-常量 X1033：NVTX3_CPP_INLINED_VERSION_MINOR
 * 编译期定义：NVTX3_CPP_INLINED_VERSION_MINOR = 0 // NVTX3_CPP_VERSION_MINOR。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX3_CPP_INLINED_VERSION_MINOR 0  // NVTX3_CPP_VERSION_MINOR
    #elif NVTX3_CPP_INLINED_VERSION_MAJOR != NVTX3_CPP_VERSION_MAJOR
        /* Unsupported case -- cannot define unversioned symbols for different major versions
         * in the same translation unit.
         */
        #error \
          "Two different major versions of the NVTX C++ Wrappers are being included in a single .cpp file, with unversioned symbols enabled in both.  Only one major version can enable unversioned symbols in a .cpp file.  To disable unversioned symbols, #define NVTX3_CPP_REQUIRE_EXPLICIT_VERSION before #including nvtx3.hpp, and use the explicit-version symbols instead -- this is the preferred way to use nvtx3.hpp from a header file."
    #elif (NVTX3_CPP_INLINED_VERSION_MAJOR == NVTX3_CPP_VERSION_MAJOR) && \
      (NVTX3_CPP_INLINED_VERSION_MINOR < NVTX3_CPP_VERSION_MINOR)
        /* An older minor version of the same major version already defined unversioned
         * symbols.  The new features provided in this header will be inlined 
         * redefine the minor version macro to this header's version.
         */
        #undef NVTX3_CPP_INLINED_VERSION_MINOR
/* 中文逐项-常量 X1034：NVTX3_CPP_INLINED_VERSION_MINOR
 * 编译期定义：NVTX3_CPP_INLINED_VERSION_MINOR = 0 // NVTX3_CPP_VERSION_MINOR。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define NVTX3_CPP_INLINED_VERSION_MINOR 0  // NVTX3_CPP_VERSION_MINOR
        // else, already have this version or newer, nothing to do
    #endif
#endif
/* clang-format on */

#include <nvtx3/nvToolsExt.h>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

/**
 * @file nvtx3.hpp
 *
 * @brief Provides C++ constructs making the NVTX library safer and easier to
 * use with zero overhead.
 */

/**
 * \mainpage
 * \tableofcontents
 *
 * \section QUICK_START Quick Start
 *
 * To add NVTX ranges to your code, use the `nvtx3::thread_range` RAII object. A
 * range begins when the object is created, and ends when the object is
 * destroyed.
 *
 * \code{.cpp}
 * #include "nvtx3.hpp"
 * void some_function(){
 *    // Begins a NVTX range with the messsage "some_function"
 *    // The range ends when some_function() returns and `r` is destroyed
 *    nvtx3::thread_range r{"some_function"};
 *
 *    for(int i = 0; i < 6; ++i){
 *       nvtx3::thread_range loop{"loop range"};
 *       std::this_thread::sleep_for(std::chrono::seconds{1});
 *    }
 * } // Range ends when `r` is destroyed
 * \endcode
 *
 * The example code above generates the following timeline view in Nsight
 * Systems:
 *
 * \image html
 * https://raw.githubusercontent.com/jrhemstad/nvtx_wrappers/master/docs/example_range.png
 *
 * Alternatively, use the \ref MACROS like `NVTX3_FUNC_RANGE()` to add
 * ranges to your code that automatically use the name of the enclosing function
 * as the range's message.
 *
 * \code{.cpp}
 * #include "nvtx3.hpp"
 * void some_function(){
 *    // Creates a range with a message "some_function" that ends when the
 * enclosing
 *    // function returns
 *    NVTX3_FUNC_RANGE();
 *    ...
 * }
 * \endcode
 *
 *
 * \section Overview
 *
 * The NVTX library provides a set of functions for users to annotate their code
 * to aid in performance profiling and optimization. These annotations provide
 * information to tools like Nsight Systems to improve visualization of
 * application timelines.
 *
 * \ref RANGES are one of the most commonly used NVTX constructs for annotating
 * a span of time. For example, imagine a user wanted to see every time a
 * function, `my_function`, is called and how long it takes to execute. This can
 * be accomplished with an NVTX range created on the entry to the function and
 * terminated on return from `my_function` using the push/pop C APIs:
 *
 * ```
 * void my_function(...){
 *    nvtxRangePushA("my_function"); // Begins NVTX range
 *    // do work
 *    nvtxRangePop(); // Ends NVTX range
 * }
 * ```
 *
 * One of the challenges with using the NVTX C API is that it requires manually
 * terminating the end of the range with `nvtxRangePop`. This can be challenging
 * if `my_function()` has multiple returns or can throw exceptions as it
 * requires calling `nvtxRangePop()` before all possible return points.
 *
 * NVTX++ solves this inconvenience through the "RAII" technique by providing a
 * `nvtx3::thread_range` class that begins a range at construction and ends the
 * range on destruction. The above example then becomes:
 *
 * ```
 * void my_function(...){
 *    nvtx3::thread_range r{"my_function"}; // Begins NVTX range
 *    // do work
 * } // Range ends on exit from `my_function` when `r` is destroyed
 * ```
 *
 * The range object `r` is deterministically destroyed whenever `my_function`
 * returns---ending the NVTX range without manual intervention. For more
 * information, see \ref RANGES and `nvtx3::domain_thread_range`.
 *
 * Another inconvenience of the NVTX C APIs are the several constructs where the
 * user is expected to initialize an object at the beginning of an application
 * and reuse that object throughout the lifetime of the application. For example
 * Domains, Categories, and Registered messages.
 *
 * Example:
 * ```
 * nvtxDomainHandle_t D = nvtxDomainCreateA("my domain");
 * // Reuse `D` throughout the rest of the application
 * ```
 *
 * This can be problematic if the user application or library does not have an
 * explicit initialization function called before all other functions to
 * ensure that these long-lived objects are initialized before being used.
 *
 * NVTX++ makes use of the "construct on first use" technique to alleviate this
 * inconvenience. In short, a function local static object is constructed upon
 * the first invocation of a function and returns a reference to that object on
 * all future invocations. See the documentation for
 * `nvtx3::registered_string`, `nvtx3::domain`, `nvtx3::named_category`,  and
 * https://isocpp.org/wiki/faq/ctors#static-init-order-on-first-use for more
 * information.
 *
 * Using construct on first use, the above example becomes:
 * ```
 * struct my_domain{ static constexpr char const* name{"my domain"}; };
 *
 * // The first invocation of `domain::get` for the type `my_domain` will
 * // construct a `nvtx3::domain` object and return a reference to it. Future
 * // invocations simply return a reference.
 * nvtx3::domain const& D = nvtx3::domain::get<my_domain>();
 * ```
 * For more information about NVTX and how it can be used, see
 * https://docs.nvidia.com/cuda/profiler-users-guide/index.html#nvtx and
 * https://devblogs.nvidia.com/cuda-pro-tip-generate-custom-application-profile-timelines-nvtx/
 * for more information.
 *
 * \section RANGES Ranges
 *
 * Ranges are used to describe a span of time during the execution of an
 * application. Common examples are using ranges to annotate the time it takes
 * to execute a function or an iteration of a loop.
 *
 * NVTX++ uses RAII to automate the generation of ranges that are tied to the
 * lifetime of objects. Similar to `std::lock_guard` in the C++ Standard
 * Template Library.
 *
 * \subsection THREAD_RANGE Thread Range
 *
 * `nvtx3::domain_thread_range` is a class that begins a range upon construction
 * and ends the range at destruction. This is one of the most commonly used
 * constructs in NVTX++ and is useful for annotating spans of time on a
 * particular thread. These ranges can be nested to arbitrary depths.
 *
 * `nvtx3::thread_range` is an alias for a `nvtx3::domain_thread_range` in the
 * global NVTX domain. For more information about Domains, see \ref DOMAINS.
 *
 * Various attributes of a range can be configured constructing a
 * `nvtx3::domain_thread_range` with a `nvtx3::event_attributes` object. For
 * more information, see \ref ATTRIBUTES.
 *
 * Example:
 *
 * \code{.cpp}
 * void some_function(){
 *    // Creates a range for the duration of `some_function`
 *    nvtx3::thread_range r{};
 *
 *    while(true){
 *       // Creates a range for every loop iteration
 *       // `loop_range` is nested inside `r`
 *       nvtx3::thread_range loop_range{};
 *    }
 * }
 * \endcode
 *
 * \subsection PROCESS_RANGE Process Range
 *
 * `nvtx3::domain_process_range` is identical to `nvtx3::domain_thread_range`
 * with the exception that a `domain_process_range` can be created and destroyed
 * on different threads. This is useful to annotate spans of time that can
 * bridge multiple threads.
 *
 * `nvtx3::domain_thread_range`s should be preferred unless one needs the
 * ability to begin and end a range on different threads.
 *
 * \section MARKS Marks
 *
 * `nvtx3::mark` allows annotating an instantaneous event in an application's
 * timeline. For example, indicating when a mutex is locked or unlocked.
 *
 * \code{.cpp}
 * std::mutex global_lock;
 * void lock_mutex(){
 *    global_lock.lock();
 *    // Marks an event immediately after the mutex is locked
 *    nvtx3::mark<my_domain>("lock_mutex");
 * }
 * \endcode
 *
 * \section DOMAINS Domains
 *
 * Similar to C++ namespaces, Domains allow for scoping NVTX events. By default,
 * all NVTX events belong to the "global" domain. Libraries and applications
 * should scope their events to use a custom domain to differentiate where the
 * events originate from.
 *
 * It is common for a library or application to have only a single domain and
 * for the name of that domain to be known at compile time. Therefore, Domains
 * in NVTX++ are represented by _tag types_.
 *
 * For example, to define a custom  domain, simply define a new concrete type
 * (a `class` or `struct`) with a `static` member called `name` that contains
 * the desired name of the domain.
 *
 * ```
 * struct my_domain{ static constexpr char const* name{"my domain"}; };
 * ```
 *
 * For any NVTX++ construct that can be scoped to a domain, the type `my_domain`
 * can be passed as an explicit template argument to scope it to the custom
 * domain.
 *
 * The tag type `nvtx3::domain::global` represents the global NVTX domain.
 *
 * \code{.cpp}
 * // By default, `domain_thread_range` belongs to the global domain
 * nvtx3::domain_thread_range<> r0{};
 *
 * // Alias for a `domain_thread_range` in the global domain
 * nvtx3::thread_range r1{};
 *
 * // `r` belongs to the custom domain
 * nvtx3::domain_thread_range<my_domain> r{};
 * \endcode
 *
 * When using a custom domain, it is reccomended to define type aliases for NVTX
 * constructs in the custom domain.
 * ```
 * using my_thread_range = nvtx3::domain_thread_range<my_domain>;
 * using my_registered_string = nvtx3::registered_string<my_domain>;
 * using my_named_category = nvtx3::named_category<my_domain>;
 * ```
 *
 * See `nvtx3::domain` for more information.
 *
 * \section ATTRIBUTES Event Attributes
 *
 * NVTX events can be customized with various attributes to provide additional
 * information (such as a custom message) or to control visualization of the
 * event (such as the color used). These attributes can be specified per-event
 * via arguments to a `nvtx3::event_attributes` object.
 *
 * NVTX events can be customized via four "attributes":
 * - \ref COLOR : color used to visualize the event in tools.
 * - \ref MESSAGES :  Custom message string.
 * - \ref PAYLOAD :  User-defined numerical value.
 * - \ref CATEGORY : Intra-domain grouping.
 *
 * It is possible to construct a `nvtx3::event_attributes` from any number of
 * attribute objects (nvtx3::color, nvtx3::message, nvtx3::payload,
 * nvtx3::category) in any order. If an attribute is not specified, a tool
 * specific default value is used. See `nvtx3::event_attributes` for more
 * information.
 *
 * \code{.cpp}
 * // Custom color, message
 * event_attributes attr{nvtx3::rgb{127, 255, 0},
 *                      "message"};
 *
 * // Custom color, message, payload, category
 * event_attributes attr{nvtx3::rgb{127, 255, 0},
 *                      nvtx3::payload{42},
 *                      "message",
 *                      nvtx3::category{1}};
 *
 * // Arguments can be in any order
 * event_attributes attr{nvtx3::payload{42},
 *                      nvtx3::category{1},
 *                      "message",
 *                      nvtx3::rgb{127, 255, 0}};
 *
 * // "First wins" with multiple arguments of the same type
 * event_attributes attr{ nvtx3::payload{42}, nvtx3::payload{7} }; // payload is
 * 42 \endcode
 *
 * \subsection MESSAGES message
 *
 * A `nvtx3::message` allows associating a custom message string with an NVTX
 * event.
 *
 * Example:
 * \code{.cpp}
 * // Create an `event_attributes` with the custom message "my message"
 * nvtx3::event_attributes attr{nvtx3::Mesage{"my message"}};
 *
 * // strings and string literals implicitly assumed to be a `nvtx3::message`
 * nvtx3::event_attributes attr{"my message"};
 * \endcode
 *
 * \subsubsection REGISTERED_MESSAGE Registered Messages
 *
 * Associating a `nvtx3::message` with an event requires copying the contents of
 * the message every time the message is used, i.e., copying the entire message
 * string. This may cause non-trivial overhead in performance sensitive code.
 *
 * To eliminate this overhead, NVTX allows registering a message string,
 * yielding a "handle" that is inexpensive to copy that may be used in place of
 * a message string. When visualizing the events, tools such as Nsight Systems
 * will take care of mapping the message handle to its string.
 *
 * A message should be registered once and the handle reused throughout the rest
 * of the application. This can be done by either explicitly creating static
 * `nvtx3::registered_string` objects, or using the
 * `nvtx3::registered_string::get` construct on first use helper (recommended).
 *
 * Similar to \ref DOMAINS, `nvtx3::registered_string::get` requires defining a
 * custom tag type with a static `message` member whose value will be the
 * contents of the registered string.
 *
 * Example:
 * \code{.cpp}
 * // Explicitly constructed, static `registered_string`
 * static registered_string<my_domain> static_message{"my message"};
 *
 * // Or use construct on first use:
 * // Define a tag type with a `message` member string to register
 * struct my_message{ static constexpr char const* message{ "my message" }; };
 *
 * // Uses construct on first use to register the contents of
 * // `my_message::message`
 * nvtx3::registered_string<my_domain> const& msg =
 * nvtx3::registered_string<my_domain>::get<my_message>(); \endcode
 *
 * \subsection COLOR color
 *
 * Associating a `nvtx3::color` with an event allows controlling how the event
 * is visualized in a tool such as Nsight Systems. This is a convenient way to
 * visually differentiate among different events.
 *
 * \code{.cpp}
 * // Define a color via rgb color values
 * nvtx3::color c{nvtx3::rgb{127, 255, 0}};
 * nvtx3::event_attributes attr{c};
 *
 * // rgb color values can be passed directly to an `event_attributes`
 * nvtx3::event_attributes attr1{nvtx3::rgb{127,255,0}};
 * \endcode
 *
 * \subsection CATEGORY category
 *
 * A `nvtx3::category` is simply an integer id that allows for fine-grain
 * grouping of NVTX events. For example, one might use separate categories for
 * IO, memory allocation, compute, etc.
 *
 * \code{.cpp}
 * nvtx3::event_attributes{nvtx3::category{1}};
 * \endcode
 *
 * \subsubsection NAMED_CATEGORIES Named Categories
 *
 * Associates a `name` string with a category `id` to help differentiate among
 * categories.
 *
 * For any given category id `Id`, a `named_category{Id, "name"}` should only
 * be constructed once and reused throughout an application. This can be done by
 * either explicitly creating static `nvtx3::named_category` objects, or using
 * the `nvtx3::named_category::get` construct on first use helper (recommended).
 *
 * Similar to \ref DOMAINS, `nvtx3::named_category::get` requires defining a
 * custom tag type with static `name` and `id` members.
 *
 * \code{.cpp}
 * // Explicitly constructed, static `named_category`
 * static nvtx3::named_category static_category{42, "my category"};
 *
 * // OR use construct on first use:
 * // Define a tag type with `name` and `id` members
 * struct my_category{
 *    static constexpr char const* name{"my category"}; // category name
 *    static constexpr category::id_type id{42}; // category id
 * };
 *
 * // Use construct on first use to name the category id `42`
 * // with name "my category"
 * nvtx3::named_category const& my_category =
 * named_category<my_domain>::get<my_category>();
 *
 * // Range `r` associated with category id `42`
 * nvtx3::event_attributes attr{my_category};
 * \endcode
 *
 * \subsection PAYLOAD payload
 *
 * Allows associating a user-defined numerical value with an event.
 *
 * ```
 * nvtx3:: event_attributes attr{nvtx3::payload{42}}; // Constructs a payload
 * from
 *                                                 // the `int32_t` value 42
 * ```
 *
 *
 * \section EXAMPLE Example
 *
 * Putting it all together:
 * \code{.cpp}
 * // Define a custom domain tag type
 * struct my_domain{ static constexpr char const* name{"my domain"}; };
 *
 * // Define a named category tag type
 * struct my_category{
 *    static constexpr char const* name{"my category"};
 *    static constexpr uint32_t id{42};
 * };
 *
 * // Define a registered string tag type
 * struct my_message{ static constexpr char const* message{"my message"}; };
 *
 * // For convenience, use aliases for domain scoped objects
 * using my_thread_range = nvtx3::domain_thread_range<my_domain>;
 * using my_registered_string = nvtx3::registered_string<my_domain>;
 * using my_named_category = nvtx3::named_category<my_domain>;
 *
 * // Default values for all attributes
 * nvtx3::event_attributes attr{};
 * my_thread_range r0{attr};
 *
 * // Custom (unregistered) message, and unnamed category
 * nvtx3::event_attributes attr1{"message", nvtx3::category{2}};
 * my_thread_range r1{attr1};
 *
 * // Alternatively, pass arguments of `event_attributes` ctor directly to
 * // `my_thread_range`
 * my_thread_range r2{"message", nvtx3::category{2}};
 *
 * // construct on first use a registered string
 * auto msg = my_registered_string::get<my_message>();
 *
 * // construct on first use a named category
 * auto category = my_named_category::get<my_category>();
 *
 * // Use registered string and named category
 * my_thread_range r3{msg, category, nvtx3::rgb{127, 255, 0},
 *                    nvtx3::payload{42}};
 *
 * // Any number of arguments in any order
 * my_thread_range r{nvtx3::rgb{127, 255,0}, msg};
 *
 * \endcode
 * \section MACROS Convenience Macros
 *
 * Oftentimes users want to quickly and easily add NVTX ranges to their library
 * or application to aid in profiling and optimization.
 *
 * A convenient way to do this is to use the \ref NVTX3_FUNC_RANGE and
 * \ref NVTX3_FUNC_RANGE_IN macros. These macros take care of constructing an
 * `nvtx3::domain_thread_range` with the name of the enclosing function as the
 * range's message.
 *
 * \code{.cpp}
 * void some_function(){
 *    // Automatically generates an NVTX range for the duration of the function
 *    // using "some_function" as the event's message.
 *    NVTX3_FUNC_RANGE();
 * }
 * \endcode
 *
 */

/* Temporary helper #defines, removed with #undef at end of header */

/* Within this header, nvtx3::NVTX3_VERSION_NAMESPACE resolves to nvtx3::vX,
 * where "X" is the major version number. */
/* 中文逐项-宏 M0117：NVTX3_CONCAT
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX3_CONCAT；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX3_CONCAT(A, B) A##B
/* 中文逐项-宏 M0118：NVTX3_NAMESPACE_FOR
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX3_NAMESPACE_FOR；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX3_NAMESPACE_FOR(VERSION) NVTX3_CONCAT(v, VERSION)
/* 中文逐项-常量 X1035：NVTX3_VERSION_NAMESPACE
 * 编译期定义：NVTX3_VERSION_NAMESPACE = NVTX3_NAMESPACE_FOR(NVTX3_CPP_VERSION_MAJOR)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX3_VERSION_NAMESPACE NVTX3_NAMESPACE_FOR(NVTX3_CPP_VERSION_MAJOR)

/* Avoid duplicating #if defined(NVTX3_INLINE_THIS_VERSION) for namespaces
 * in each minor version by making a macro to use unconditionally, which
 * resolves to "inline" or nothing as appropriate. */
#if defined(NVTX3_INLINE_THIS_VERSION)
/* 中文逐项-常量 X1036：NVTX3_INLINE_IF_REQUESTED
 * 编译期定义：NVTX3_INLINE_IF_REQUESTED = inline。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX3_INLINE_IF_REQUESTED inline
#else
/* 中文逐项-常量 X1037：NVTX3_INLINE_IF_REQUESTED
 * 编译期定义：NVTX3_INLINE_IF_REQUESTED = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX3_INLINE_IF_REQUESTED
#endif

/* Enables the use of constexpr when support for C++14 relaxed constexpr
 * is present.
 *
 * Initializing a legacy-C (i.e., no constructor) union member requires
 * initializing in the constructor body. Non-empty constexpr constructors
 * require C++14 relaxed constexpr.  In strict C++11 compilation, fall back
 * to using non-constexpr constructors for classes with union members.
 */
#if __cpp_constexpr >= 201304L
/* 中文逐项-常量 X1038：NVTX3_RELAXED_CONSTEXPR
 * 编译期定义：NVTX3_RELAXED_CONSTEXPR = constexpr。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX3_RELAXED_CONSTEXPR constexpr
#else
/* 中文逐项-常量 X1039：NVTX3_RELAXED_CONSTEXPR
 * 编译期定义：NVTX3_RELAXED_CONSTEXPR = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX3_RELAXED_CONSTEXPR
#endif

/* Implementation sections, enclosed in guard macros for each minor version */

#ifndef NVTX3_CPP_DEFINITIONS_V1_0
/* 中文逐项-常量 X1040：NVTX3_CPP_DEFINITIONS_V1_0
 * 编译期定义：NVTX3_CPP_DEFINITIONS_V1_0 = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX3_CPP_DEFINITIONS_V1_0

namespace nvtx3 {

NVTX3_INLINE_IF_REQUESTED namespace NVTX3_VERSION_NAMESPACE
{

namespace detail {

/**
 * @brief Verifies if a type `T` contains a member `T::name` of type `const
 * char*` or `const wchar_t*`.
 *
 * @tparam T The type to verify
 * @return True if `T` contains a member `T::name` of type `const char*` or
 * `const wchar_t*`.
 */
/* 中文内部逻辑 F0809：has_name_member
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return (std::is_same<char const*, typename std::decay<decltype(T::name)>::type>::value || std::is_same<wchar_t const*, typename std::decay<decltype(T::name)>::type>::value);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0809
 */
/* 中文逐项-函数 F0809：has_name_member
 * 用途与位置：通过模板替换检测命名域类型是否具有要求的静态name成员，编译期拒绝不满足接口的类型。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3.hpp::get。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (std::is_same<char const*, typename std::decay<decltype(T::name)>::type>::value ||
 * std::is_same<wchar_t const*, typename std::decay<d …（完整表达式见紧邻源码）。CHECK类宏还可能提前返回/跳转。
 */
template <typename T>
constexpr auto has_name_member() noexcept -> decltype(T::name, bool())
{
  return (std::is_same<char const*, typename std::decay<decltype(T::name)>::type>::value ||
          std::is_same<wchar_t const*, typename std::decay<decltype(T::name)>::type>::value);
}
}  // namespace detail

/**
 * @brief `domain`s allow for grouping NVTX events into a single scope to
 * differentiate them from events in other `domain`s.
 *
 * By default, all NVTX constructs are placed in the "global" NVTX domain.
 *
 * A custom `domain` may be used in order to differentiate a library's or
 * application's NVTX events from other events.
 *
 * `domain`s are expected to be long-lived and unique to a library or
 * application. As such, it is assumed a domain's name is known at compile
 * time. Therefore, all NVTX constructs that can be associated with a domain
 * require the domain to be specified via a *type* `DomainName` passed as an
 * explicit template parameter.
 *
 * The type `domain::global` may be used to indicate that the global NVTX
 * domain should be used.
 *
 * None of the C++ NVTX constructs require the user to manually construct a
 * `domain` object. Instead, if a custom domain is desired, the user is
 * expected to define a type `DomainName` that contains a member
 * `DomainName::name` which resolves to either a `char const*` or `wchar_t
 * const*`. The value of `DomainName::name` is used to name and uniquely
 * identify the custom domain.
 *
 * Upon the first use of an NVTX construct associated with the type
 * `DomainName`, the "construct on first use" pattern is used to construct a
 * function local static `domain` object. All future NVTX constructs
 * associated with `DomainType` will use a reference to the previously
 * constructed `domain` object. See `domain::get`.
 *
 * Example:
 * ```
 * // The type `my_domain` defines a `name` member used to name and identify
 * the
 * // `domain` object identified by `my_domain`.
 * struct my_domain{ static constexpr char const* name{"my_domain"}; };
 *
 * // The NVTX range `r` will be grouped with all other NVTX constructs
 * // associated with  `my_domain`.
 * nvtx3::domain_thread_range<my_domain> r{};
 *
 * // An alias can be created for a `domain_thread_range` in the custom domain
 * using my_thread_range = nvtx3::domain_thread_range<my_domain>;
 * my_thread_range my_range{};
 *
 * // `domain::global` indicates that the global NVTX domain is used
 * nvtx3::domain_thread_range<domain::global> r2{};
 *
 * // For convenience, `nvtx3::thread_range` is an alias for a range in the
 * // global domain
 * nvtx3::thread_range r3{};
 * ```
 */
/* 中文逐项-类型 T0346：domain
 * 类型职责：本文件的class类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：_domain。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
class domain {
 public:
  domain(domain const&) = delete;
  domain& operator=(domain const&) = delete;
  domain(domain&&)                 = delete;
  domain& operator=(domain&&) = delete;

  /**
   * @brief Returns reference to an instance of a function local static
   * `domain` object.
   *
   * Uses the "construct on first use" idiom to safely ensure the `domain`
   * object is initialized exactly once upon first invocation of
   * `domain::get<DomainName>()`. All following invocations will return a
   * reference to the previously constructed `domain` object. See
   * https://isocpp.org/wiki/faq/ctors#static-init-order-on-first-use
   *
   * None of the constructs in this header require the user to directly invoke
   * `domain::get`. It is automatically invoked when constructing objects like
   * a `domain_thread_range` or `category`. Advanced users may wish to use
   * `domain::get` for the convenience of the "construct on first use" idiom
   * when using domains with their own use of the NVTX C API.
   *
   * This function is threadsafe as of C++11. If two or more threads call
   * `domain::get<DomainName>` concurrently, exactly one of them is guaranteed
   * to construct the `domain` object and the other(s) will receive a
   * reference to the object after it is fully constructed.
   *
   * The domain's name is specified via the type `DomainName` pass as an
   * explicit template parameter. `DomainName` is required to contain a
   * member `DomainName::name` that resolves to either a `char const*` or
   * `wchar_t const*`. The value of `DomainName::name` is used to name and
   * uniquely identify the `domain`.
   *
   * Example:
   * ```
   * // The type `my_domain` defines a `name` member used to name and identify
   * // the `domain` object identified by `my_domain`.
   * struct my_domain{ static constexpr char const* name{"my domain"}; };
   *
   * auto D = domain::get<my_domain>(); // First invocation constructs a
   *                                    // `domain` with the name "my domain"
   *
   * auto D1 = domain::get<my_domain>(); // Simply returns reference to
   *                                     // previously constructed `domain`.
   * ```
   *
   * @tparam DomainName Type that contains a `DomainName::name` member used to
   * name the `domain` object.
   * @return Reference to the `domain` corresponding to the type `DomainName`.
   */
/* 中文内部逻辑 F0810：get
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 源码语法片段：static_assert(detail::has_name_member<DomainName>(), "Type used to identify a domain must contain a name member of" "type const char* or const wchar_t*");；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 2. 准备局部状态：static domain const d{DomainName::name};；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 函数返回：return d;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0810
 */
/* 中文逐项-函数 F0810：get
 * 用途与位置：NVTX包装的construct-on-first-use入口，按DomainName/类别/字符串模板创建并复用对应的静态注册对象。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3.hpp::named_category；src/include/nvtx3.hpp::domain_thread_range；src/include/nvtx3.hpp::~domain_thread_range；src/include/nvtx3.hpp::start_range；src/include/nvtx3.hpp::mark。
 * 本体直接出现的调用/包装：detail::has_name_member<DomainName>。
 * 源码返回表达式：return d;。CHECK类宏还可能提前返回/跳转。
 */
  template <typename DomainName>
  static domain const& get()
  {
    static_assert(detail::has_name_member<DomainName>(),
                  "Type used to identify a domain must contain a name member of"
                  "type const char* or const wchar_t*");
/* 中文逐项-变量 V3023：d
 * d（domain；函数 get 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 d = {DomainName::name}；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
    static domain const d{DomainName::name};
    return d;
  }

  /**
   * @brief Conversion operator to `nvtxDomainHandle_t`.
   *
   * Allows transparently passing a domain object into an API expecting a
   * native `nvtxDomainHandle_t` object.
   */
  operator nvtxDomainHandle_t() const noexcept { return _domain; }

  /**
   * @brief Tag type for the "global" NVTX domain.
   *
   * This type may be passed as a template argument to any function/class
   * expecting a type to identify a domain to indicate that the global domain
   * should be used.
   *
   * All NVTX events in the global domain across all libraries and
   * applications will be grouped together.
   *
   */
/* 中文逐项-类型 T0347：global
 * 类型职责：本文件的struct类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 */
  struct global {
  };

 private:
  /**
   * @brief Construct a new domain with the specified `name`.
   *
   * This constructor is private as it is intended that `domain` objects only
   * be created through the `domain::get` function.
   *
   * @param name A unique name identifying the domain
   */
/* 中文内部逻辑 F0811：domain
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0811
 */
/* 中文逐项-函数 F0811：domain
 * 用途与位置：构造NVTX性能标记域，按传入文本创建/引用工具可识别的域句柄，与NCCL communicator不同。
 * 参数 name（char const* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  explicit domain(char const* name) noexcept : _domain{nvtxDomainCreateA(name)} {}

  /**
   * @brief Construct a new domain with the specified `name`.
   *
   * This constructor is private as it is intended that `domain` objects only
   * be created through the `domain::get` function.
   *
   * @param name A unique name identifying the domain
   */
/* 中文内部逻辑 F0812：domain
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0812
 */
/* 中文逐项-函数 F0812：domain
 * 用途与位置：构造NVTX性能标记域，按传入文本创建/引用工具可识别的域句柄，与NCCL communicator不同。
 * 参数 name（wchar_t const* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  explicit domain(wchar_t const* name) noexcept : _domain{nvtxDomainCreateW(name)} {}

  /**
   * @brief Construct a new domain with the specified `name`.
   *
   * This constructor is private as it is intended that `domain` objects only
   * be created through the `domain::get` function.
   *
   * @param name A unique name identifying the domain
   */
/* 中文内部逻辑 F0813：domain
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0813
 */
/* 中文逐项-函数 F0813：domain
 * 用途与位置：构造NVTX性能标记域，按传入文本创建/引用工具可识别的域句柄，与NCCL communicator不同。
 * 参数 <未命名参数>（std::string const& name）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  explicit domain(std::string const& name) noexcept : domain{name.c_str()} {}

  /**
   * @brief Construct a new domain with the specified `name`.
   *
   * This constructor is private as it is intended that `domain` objects only
   * be created through the `domain::get` function.
   *
   * @param name A unique name identifying the domain
   */
/* 中文内部逻辑 F0814：domain
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0814
 */
/* 中文逐项-函数 F0814：domain
 * 用途与位置：构造NVTX性能标记域，按传入文本创建/引用工具可识别的域句柄，与NCCL communicator不同。
 * 参数 <未命名参数>（std::wstring const& name）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  explicit domain(std::wstring const& name) noexcept : domain{name.c_str()} {}

  /**
   * @brief Default constructor creates a `domain` representing the
   * "global" NVTX domain.
   *
   * All events not associated with a custom `domain` are grouped in the
   * "global" NVTX domain.
   *
   */
  domain() = default;

  /**
   * @brief Intentionally avoid calling nvtxDomainDestroy on the `domain` object.
   *
   * No currently-available tools attempt to free domain resources when the
   * nvtxDomainDestroy function is called, due to the thread-safety and
   * efficiency challenges of freeing thread-local storage for other threads.
   * Since libraries may be disallowed from introducing static destructors,
   * and destroying the domain is likely to have no effect, the destructor
   * for `domain` intentionally chooses to not destroy the domain.
   *
   * In a situation where domain destruction is necessary, either manually
   * call nvtxDomainDestroy on the domain's handle, or make a class that
   * derives from `domain` and calls nvtxDomainDestroy in its destructor.
   */
  ~domain() = default;

 private:
/* 中文逐项-变量 V3024：_domain
 * _domain（nvtxDomainHandle_t；成员 domain）：NVTX域/注册对象的不透明句柄，工具API使用它定位标记域或字符串，不是GPU内存地址。
 */
  nvtxDomainHandle_t const _domain{};  ///< The `domain`s NVTX handle
};

/**
 * @brief Returns reference to the `domain` object that represents the global
 * NVTX domain.
 *
 * This specialization for `domain::global` returns a default constructed,
 * `domain` object for use when the "global" domain is desired.
 *
 * All NVTX events in the global domain across all libraries and applications
 * will be grouped together.
 *
 * @return Reference to the `domain` corresponding to the global NVTX domain.
 *
 */
/* 中文内部逻辑 F0815：domain::get<domain::global>
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：static domain const d{};；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 函数返回：return d;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0815
 */
/* 中文逐项-函数 F0815：domain::get<domain::global>
 * 用途与位置：全局域特化，返回默认NVTX域表示，避免为默认作用域创建重复命名域。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return d;。CHECK类宏还可能提前返回/跳转。
 */
template <>
inline domain const& domain::get<domain::global>()
{
/* 中文逐项-变量 V3025：d
 * d（domain；函数 domain::get<domain::global> 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 d = {}；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  static domain const d{};
  return d;
}

/**
 * @brief Indicates the values of the red, green, blue color channels for
 * a rgb color code.
 *
 */
/* 中文逐项-类型 T0348：rgb
 * 类型职责：本文件的struct类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：red、green、blue。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct rgb {
  /// Type used for component values
  using component_type = uint8_t;

  /**
   * @brief Construct a rgb with red, green, and blue channels
   * specified by `red_`, `green_`, and `blue_`, respectively.
   *
   * Valid values are in the range `[0,255]`.
   *
   * @param red_ Value of the red channel
   * @param green_ Value of the green channel
   * @param blue_ Value of the blue channel
   */
/* 中文内部逻辑 F0816：rgb
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0816
 */
/* 中文逐项-函数 F0816：rgb
 * 用途与位置：构造颜色分量描述，RGB未显式给alpha的部分按对应包装约定处理，不影响张量数据。
 * 参数 red_（component_type red_）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 green_（component_type green_）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 blue_（component_type blue_）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  constexpr rgb(component_type red_, component_type green_, component_type blue_) noexcept
    : red{red_}, green{green_}, blue{blue_}
  {
  }

/* 中文逐项-变量 V3026：red
 * red（component_type；成员 rgb）：ARGB颜色的单字节分量，打包后仅控制性能工具显示颜色。
 */
  component_type const red{};    ///< Red channel value
/* 中文逐项-变量 V3027：green
 * green（component_type；成员 rgb）：ARGB颜色的单字节分量，打包后仅控制性能工具显示颜色。
 */
  component_type const green{};  ///< Green channel value
/* 中文逐项-变量 V3028：blue
 * blue（component_type；成员 rgb）：ARGB颜色的单字节分量，打包后仅控制性能工具显示颜色。
 */
  component_type const blue{};   ///< Blue channel value
};

/**
 * @brief Indicates the value of the alpha, red, green, and blue color
 * channels for an argb color code.
 *
 */
/* 中文逐项-类型 T0349：argb
 * 类型职责：本文件的struct类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：alpha。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct argb final : rgb {
  /**
   * @brief Construct an argb with alpha, red, green, and blue channels
   * specified by `alpha_`, `red_`, `green_`, and `blue_`, respectively.
   *
   * Valid values are in the range `[0,255]`.
   *
   * @param alpha_  Value of the alpha channel (opacity)
   * @param red_  Value of the red channel
   * @param green_  Value of the green channel
   * @param blue_  Value of the blue channel
   *
   */
/* 中文内部逻辑 F0817：argb
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0817
 */
/* 中文逐项-函数 F0817：argb
 * 用途与位置：构造颜色分量描述，RGB未显式给alpha的部分按对应包装约定处理，不影响张量数据。
 * 参数 alpha_（component_type alpha_）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 red_（component_type red_）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 green_（component_type green_）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 blue_（component_type blue_）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  constexpr argb(component_type alpha_,
                 component_type red_,
                 component_type green_,
                 component_type blue_) noexcept
    : rgb{red_, green_, blue_}, alpha{alpha_}
  {
  }

/* 中文逐项-变量 V3029：alpha
 * alpha（component_type；成员 argb）：ARGB颜色的单字节分量，打包后仅控制性能工具显示颜色。
 */
  component_type const alpha{};  ///< Alpha channel value
};

/**
 * @brief Represents a custom color that can be associated with an NVTX event
 * via it's `event_attributes`.
 *
 * Specifying colors for NVTX events is a convenient way to visually
 * differentiate among different events in a visualization tool such as Nsight
 * Systems.
 *
 */
/* 中文逐项-类型 T0350：color
 * 类型职责：本文件的class类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：_value、_type。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
class color {
 public:
  /// Type used for the color's value
  using value_type = uint32_t;

  /**
   * @brief Constructs a `color` using the value provided by `hex_code`.
   *
   * `hex_code` is expected to be a 4 byte argb hex code.
   *
   * The most significant byte indicates the value of the alpha channel
   * (opacity) (0-255)
   *
   * The next byte indicates the value of the red channel (0-255)
   *
   * The next byte indicates the value of the green channel (0-255)
   *
   * The least significant byte indicates the value of the blue channel
   * (0-255)
   *
   * @param hex_code The hex code used to construct the `color`
   */
/* 中文内部逻辑 F0818：color
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0818
 */
/* 中文逐项-函数 F0818：color
 * 用途与位置：构造NVTX颜色属性，保存颜色种类和打包值以供事件属性使用。
 * 参数 hex_code（value_type hex_code）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  constexpr explicit color(value_type hex_code) noexcept : _value{hex_code} {}

  /**
   * @brief Construct a `color` using the alpha, red, green, blue components
   * in `argb`.
   *
   * @param argb The alpha, red, green, blue components of the desired `color`
   */
/* 中文内部逻辑 F0819：color
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0819
 */
/* 中文逐项-函数 F0819：color
 * 用途与位置：构造NVTX颜色属性，保存颜色种类和打包值以供事件属性使用。
 * 参数 argb（argb argb）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  constexpr color(argb argb) noexcept
    : color{from_bytes_msb_to_lsb(argb.alpha, argb.red, argb.green, argb.blue)}
  {
  }

  /**
   * @brief Construct a `color` using the red, green, blue components in
   * `rgb`.
   *
   * Uses maximum value for the alpha channel (opacity) of the `color`.
   *
   * @param rgb The red, green, blue components of the desired `color`
   */
/* 中文内部逻辑 F0820：color
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0820
 */
/* 中文逐项-函数 F0820：color
 * 用途与位置：构造NVTX颜色属性，保存颜色种类和打包值以供事件属性使用。
 * 参数 rgb（rgb rgb）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  constexpr color(rgb rgb) noexcept
    : color{from_bytes_msb_to_lsb(0xFF, rgb.red, rgb.green, rgb.blue)}
  {
  }

  /**
   * @brief Returns the `color`s argb hex code
   *
   */
/* 中文内部逻辑 F0821：get_value
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return _value;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0821
 */
/* 中文逐项-函数 F0821：get_value
 * 用途与位置：返回NVTX包装内部保存的颜色/消息/载荷union值，不复制或传输GPU张量。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return _value;。CHECK类宏还可能提前返回/跳转。
 */
  constexpr value_type get_value() const noexcept { return _value; }

  /**
   * @brief Return the NVTX color type of the color.
   *
   */
/* 中文内部逻辑 F0822：get_type
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return _type;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0822
 */
/* 中文逐项-函数 F0822：get_type
 * 用途与位置：返回NVTX消息/颜色/载荷的类型标签，使消费者选择正确union视图。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return _type;。CHECK类宏还可能提前返回/跳转。
 */
  constexpr nvtxColorType_t get_type() const noexcept { return _type; }

  color()             = delete;
  ~color()            = default;
  color(color const&) = default;
  color& operator=(color const&) = default;
  color(color&&)                 = default;
  color& operator=(color&&) = default;

 private:
  /**
   * @brief Constructs an unsigned, 4B integer from the component bytes in
   * most to least significant byte order.
   *
   */
/* 中文内部逻辑 F0823：from_bytes_msb_to_lsb
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return uint32_t{byte3} << 24 | uint32_t{byte2} << 16 | uint32_t{byte1} << 8 | uint32_t{byte0};；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0823
 */
/* 中文逐项-函数 F0823：from_bytes_msb_to_lsb
 * 用途与位置：按从高到低字节顺序打包颜色分量，形成NVTX事件颜色值。
 * 参数 byte3（uint8_t byte3）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 byte2（uint8_t byte2）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 byte1（uint8_t byte1）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 byte0（uint8_t byte0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return uint32_t{byte3} << 24 | uint32_t{byte2} << 16 | uint32_t{byte1} << 8 |
 * uint32_t{byte0};。CHECK类宏还可能提前返回/跳转。
 */
  constexpr static value_type from_bytes_msb_to_lsb(uint8_t byte3,
                                                    uint8_t byte2,
                                                    uint8_t byte1,
                                                    uint8_t byte0) noexcept
  {
    return uint32_t{byte3} << 24 | uint32_t{byte2} << 16 | uint32_t{byte1} << 8 | uint32_t{byte0};
  }

/* 中文逐项-变量 V3030：_value
 * _value（value_type；成员 color）：NVTX包装保存的标量/union值，配合类型字段解释。
 */
  value_type const _value{};                     ///< color's argb color code
/* 中文逐项-变量 V3031：_type
 * _type（nvtxColorType_t；成员 color）：NVTX颜色/消息/载荷的union类型标签，读取值前先看对应标签。
 */
  nvtxColorType_t const _type{NVTX_COLOR_ARGB};  ///< NVTX color type code
};

/**
 * @brief Object for intra-domain grouping of NVTX events.
 *
 * A `category` is simply an integer id that allows for fine-grain grouping of
 * NVTX events. For example, one might use separate categories for IO, memory
 * allocation, compute, etc.
 *
 * Example:
 * \code{.cpp}
 * nvtx3::category cat1{1};
 *
 * // Range `r1` belongs to the category identified by the value `1`.
 * nvtx3::thread_range r1{cat1};
 *
 * // Range `r2` belongs to the same category as `r1`
 * nvtx3::thread_range r2{nvtx3::category{1}};
 * \endcode
 *
 * To associate a name string with a category id, see `named_category`.
 *
 */
/* 中文逐项-类型 T0351：category
 * 类型职责：本文件的class类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：id_。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
class category {
 public:
  /// Type used for `category`s integer id.
  using id_type = uint32_t;

  /**
   * @brief Construct a `category` with the specified `id`.
   *
   * The `category` will be unnamed and identified only by its `id` value.
   *
   * All `category` objects sharing the same `id` are equivalent.
   *
   * @param[in] id The `category`'s identifying value
   */
/* 中文内部逻辑 F0824：category
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0824
 */
/* 中文逐项-函数 F0824：category
 * 用途与位置：保存NVTX事件类别整数标识，供性能工具分组显示。
 * 参数 id（id_type id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  constexpr explicit category(id_type id) noexcept : id_{id} {}

  /**
   * @brief Returns the id of the category.
   *
   */
/* 中文内部逻辑 F0825：get_id
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return id_;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0825
 */
/* 中文逐项-函数 F0825：get_id
 * 用途与位置：读取当前NVTX类别或范围标识，不等于NCCL rank/channel编号。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3.hpp::named_category。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return id_;。CHECK类宏还可能提前返回/跳转。
 */
  constexpr id_type get_id() const noexcept { return id_; }

  category()                = delete;
  ~category()               = default;
  category(category const&) = default;
  category& operator=(category const&) = default;
  category(category&&)                 = default;
  category& operator=(category&&) = default;

 private:
/* 中文逐项-变量 V3032：id_
 * id_（id_type；成员 category）：NVTX范围或类别id，供start/end或类别分组匹配，与NCCL rank无关。
 */
  id_type const id_{};  ///< category's unique identifier
};

/**
 * @brief A `category` with an associated name string.
 *
 * Associates a `name` string with a category `id` to help differentiate among
 * categories.
 *
 * For any given category id `Id`, a `named_category(Id, "name")` should only
 * be constructed once and reused throughout an application. This can be done
 * by either explicitly creating static `named_category` objects, or using the
 * `named_category::get` construct on first use helper (recommended).
 *
 * Creating two or more `named_category` objects with the same value for `id`
 * in the same domain results in undefined behavior.
 *
 * Similarly, behavior is undefined when a `named_category` and `category`
 * share the same value of `id`.
 *
 * Example:
 * \code{.cpp}
 * // Explicitly constructed, static `named_category`
 * static nvtx3::named_category static_category{42, "my category"};
 *
 * // Range `r` associated with category id `42`
 * nvtx3::thread_range r{static_category};
 *
 * // OR use construct on first use:
 *
 * // Define a type with `name` and `id` members
 * struct my_category{
 *    static constexpr char const* name{"my category"}; // category name
 *    static constexpr category::id_type id{42}; // category id
 * };
 *
 * // Use construct on first use to name the category id `42`
 * // with name "my category"
 * auto my_category = named_category<my_domain>::get<my_category>();
 *
 * // Range `r` associated with category id `42`
 * nvtx3::thread_range r{my_category};
 * \endcode
 *
 * `named_category`'s association of a name to a category id is local to the
 * domain specified by the type `D`. An id may have a different name in
 * another domain.
 *
 * @tparam D Type containing `name` member used to identify the `domain` to
 * which the `named_category` belongs. Else, `domain::global` to  indicate
 * that the global NVTX domain should be used.
 */
/* 中文逐项-类型 T0352：named_category
 * 类型职责：本文件的class类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 */
template <typename D = domain::global>
class named_category final : public category {
 public:
  /**
   * @brief Returns a global instance of a `named_category` as a
   * function-local static.
   *
   * Creates a `named_category` with name and id specified by the contents of
   * a type `C`. `C::name` determines the name and `C::id` determines the
   * category id.
   *
   * This function is useful for constructing a named `category` exactly once
   * and reusing the same instance throughout an application.
   *
   * Example:
   * \code{.cpp}
   * // Define a type with `name` and `id` members
   * struct my_category{
   *    static constexpr char const* name{"my category"}; // category name
   *    static constexpr uint32_t id{42}; // category id
   * };
   *
   * // Use construct on first use to name the category id `42`
   * // with name "my category"
   * auto cat = named_category<my_domain>::get<my_category>();
   *
   * // Range `r` associated with category id `42`
   * nvtx3::thread_range r{cat};
   * \endcode
   *
   * Uses the "construct on first use" idiom to safely ensure the `category`
   * object is initialized exactly once. See
   * https://isocpp.org/wiki/faq/ctors#static-init-order-on-first-use
   *
   * @tparam C Type containing a member `C::name` that resolves  to either a
   * `char const*` or `wchar_t const*` and `C::id`.
   */
/* 中文内部逻辑 F0826：get
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 源码语法片段：static_assert(detail::has_name_member<C>(), "Type used to name a category must contain a name member.");；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 2. 准备局部状态：static named_category<D> const category{C::id, C::name};；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 函数返回：return category;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0826
 */
/* 中文逐项-函数 F0826：get
 * 用途与位置：NVTX包装的construct-on-first-use入口，按DomainName/类别/字符串模板创建并复用对应的静态注册对象。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3.hpp::named_category；src/include/nvtx3.hpp::domain_thread_range；src/include/nvtx3.hpp::~domain_thread_range；src/include/nvtx3.hpp::start_range；src/include/nvtx3.hpp::mark。
 * 本体直接出现的调用/包装：detail::has_name_member<C>。
 * 源码返回表达式：return category;。CHECK类宏还可能提前返回/跳转。
 */
  template <typename C>
  static named_category<D> const& get() noexcept
  {
    static_assert(detail::has_name_member<C>(),
                  "Type used to name a category must contain a name member.");
/* 中文逐项-变量 V3033：category
 * category（named_category<D>；函数 get 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 category = {C::id, C::name}；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
    static named_category<D> const category{C::id, C::name};
    return category;
  }
  /**
   * @brief Construct a `category` with the specified `id` and `name`.
   *
   * The name `name` will be registered with `id`.
   *
   * Every unique value of `id` should only be named once.
   *
   * @param[in] id The category id to name
   * @param[in] name The name to associated with `id`
   */
/* 中文内部逻辑 F0827：named_category
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 调用 / 计算 / 写入：nvtxDomainNameCategoryA(domain::get<D>(), get_id(), name);；为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 调用 / 计算 / 写入：(void)id;；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 *     5. 调用 / 计算 / 写入：(void)name;；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0827
 */
/* 中文逐项-函数 F0827：named_category
 * 用途与位置：在指定NVTX域中为类别id注册可读名字，后续事件可引用该类别。
 * 参数 id（id_type id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 name（char const* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：domain::get<D>；get_id；nvtxDomainNameCategoryA。
 */
  named_category(id_type id, char const* name) noexcept : category{id}
  {
#ifndef NVTX_DISABLE
    nvtxDomainNameCategoryA(domain::get<D>(), get_id(), name);
#else
    (void)id;
    (void)name;
#endif
  };

  /**
   * @brief Construct a `category` with the specified `id` and `name`.
   *
   * The name `name` will be registered with `id`.
   *
   * Every unique value of `id` should only be named once.
   *
   * @param[in] id The category id to name
   * @param[in] name The name to associated with `id`
   */
/* 中文内部逻辑 F0828：named_category
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 调用 / 计算 / 写入：nvtxDomainNameCategoryW(domain::get<D>(), get_id(), name);；为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 调用 / 计算 / 写入：(void)id;；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 *     5. 调用 / 计算 / 写入：(void)name;；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0828
 */
/* 中文逐项-函数 F0828：named_category
 * 用途与位置：在指定NVTX域中为类别id注册可读名字，后续事件可引用该类别。
 * 参数 id（id_type id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 name（wchar_t const* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：domain::get<D>；get_id；nvtxDomainNameCategoryW。
 */
  named_category(id_type id, wchar_t const* name) noexcept : category{id}
  {
#ifndef NVTX_DISABLE
    nvtxDomainNameCategoryW(domain::get<D>(), get_id(), name);
#else
    (void)id;
    (void)name;
#endif
  };
};

/**
 * @brief A message registered with NVTX.
 *
 * Normally, associating a `message` with an NVTX event requires copying the
 * contents of the message string. This may cause non-trivial overhead in
 * highly performance sensitive regions of code.
 *
 * message registration is an optimization to lower the overhead of
 * associating a message with an NVTX event. Registering a message yields a
 * handle that is inexpensive to copy that may be used in place of a message
 * string.
 *
 * A particular message should only be registered once and the handle
 * reused throughout the rest of the application. This can be done by either
 * explicitly creating static `registered_string` objects, or using the
 * `registered_string::get` construct on first use helper (recommended).
 *
 * Example:
 * \code{.cpp}
 * // Explicitly constructed, static `registered_string`
 * static registered_string<my_domain> static_message{"message"};
 *
 * // "message" is associated with the range `r`
 * nvtx3::thread_range r{static_message};
 *
 * // Or use construct on first use:
 *
 * // Define a type with a `message` member that defines the contents of the
 * // registered string
 * struct my_message{ static constexpr char const* message{ "my message" }; };
 *
 * // Uses construct on first use to register the contents of
 * // `my_message::message`
 * auto msg = registered_string<my_domain>::get<my_message>();
 *
 * // "my message" is associated with the range `r`
 * nvtx3::thread_range r{msg};
 * \endcode
 *
 * `registered_string`s are local to a particular domain specified via
 * the type `D`.
 *
 * @tparam D Type containing `name` member used to identify the `domain` to
 * which the `registered_string` belongs. Else, `domain::global` to  indicate
 * that the global NVTX domain should be used.
 */
/* 中文逐项-类型 T0353：registered_string
 * 类型职责：本文件的class类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：handle_。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template <typename D = domain::global>
class registered_string {
 public:
  /**
   * @brief Returns a global instance of a `registered_string` as a function
   * local static.
   *
   * Provides a convenient way to register a message with NVTX without having
   * to explicitly register the message.
   *
   * Upon first invocation, constructs a `registered_string` whose contents
   * are specified by `message::message`.
   *
   * All future invocations will return a reference to the object constructed
   * in the first invocation.
   *
   * Example:
   * \code{.cpp}
   * // Define a type with a `message` member that defines the contents of the
   * // registered string
   * struct my_message{ static constexpr char const* message{ "my message" };
   * };
   *
   * // Uses construct on first use to register the contents of
   * // `my_message::message`
   * auto msg = registered_string<my_domain>::get<my_message>();
   *
   * // "my message" is associated with the range `r`
   * nvtx3::thread_range r{msg};
   * \endcode
   *
   * @tparam M Type required to contain a member `M::message` that
   * resolves to either a `char const*` or `wchar_t const*` used as the
   * registered string's contents.
   * @return Reference to a `registered_string` associated with the type `M`.
   */
/* 中文内部逻辑 F0829：get
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：static registered_string<D> const registered_string{M::message};；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 函数返回：return registered_string;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0829
 */
/* 中文逐项-函数 F0829：get
 * 用途与位置：NVTX包装的construct-on-first-use入口，按DomainName/类别/字符串模板创建并复用对应的静态注册对象。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3.hpp::named_category；src/include/nvtx3.hpp::domain_thread_range；src/include/nvtx3.hpp::~domain_thread_range；src/include/nvtx3.hpp::start_range；src/include/nvtx3.hpp::mark。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return registered_string;。CHECK类宏还可能提前返回/跳转。
 */
  template <typename M>
  static registered_string<D> const& get() noexcept
  {
/* 中文逐项-变量 V3034：registered_string
 * registered_string（registered_string<D>；函数 get 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 registered_string = {M::message}；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
    static registered_string<D> const registered_string{M::message};
    return registered_string;
  }

  /**
   * @brief Constructs a `registered_string` from the specified `msg` string.
   *
   * Registers `msg` with NVTX and associates a handle with the registered
   * message.
   *
   * A particular message should should only be registered once and the handle
   * reused throughout the rest of the application.
   *
   * @param msg The contents of the message
   */
/* 中文内部逻辑 F0830：registered_string
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0830
 */
/* 中文逐项-函数 F0830：registered_string
 * 用途与位置：在NVTX域注册可复用消息字符串，减少频繁标记时重复文本处理。
 * 参数 msg（char const* msg）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  explicit registered_string(char const* msg) noexcept
    : handle_{nvtxDomainRegisterStringA(domain::get<D>(), msg)}
  {
  }

  /**
   * @brief Constructs a `registered_string` from the specified `msg` string.
   *
   * Registers `msg` with NVTX and associates a handle with the registered
   * message.
   *
   * A particular message should should only be registered once and the handle
   * reused throughout the rest of the application.
   *
   * @param msg The contents of the message
   */
/* 中文内部逻辑 F0831：registered_string
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0831
 */
/* 中文逐项-函数 F0831：registered_string
 * 用途与位置：在NVTX域注册可复用消息字符串，减少频繁标记时重复文本处理。
 * 参数 <未命名参数>（std::string const& msg）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  explicit registered_string(std::string const& msg) noexcept : registered_string{msg.c_str()} {}

  /**
   * @brief Constructs a `registered_string` from the specified `msg` string.
   *
   * Registers `msg` with NVTX and associates a handle with the registered
   * message.
   *
   * A particular message should should only be registered once and the handle
   * reused throughout the rest of the application.
   *
   * @param msg The contents of the message
   */
/* 中文内部逻辑 F0832：registered_string
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0832
 */
/* 中文逐项-函数 F0832：registered_string
 * 用途与位置：在NVTX域注册可复用消息字符串，减少频繁标记时重复文本处理。
 * 参数 msg（wchar_t const* msg）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  explicit registered_string(wchar_t const* msg) noexcept
    : handle_{nvtxDomainRegisterStringW(domain::get<D>(), msg)}
  {
  }

  /**
   * @brief Constructs a `registered_string` from the specified `msg` string.
   *
   * Registers `msg` with NVTX and associates a handle with the registered
   * message.
   *
   * A particular message should only be registered once and the handle
   * reused throughout the rest of the application.
   *
   * @param msg The contents of the message
   */
/* 中文内部逻辑 F0833：registered_string
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0833
 */
/* 中文逐项-函数 F0833：registered_string
 * 用途与位置：在NVTX域注册可复用消息字符串，减少频繁标记时重复文本处理。
 * 参数 <未命名参数>（std::wstring const& msg）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  explicit registered_string(std::wstring const& msg) noexcept : registered_string{msg.c_str()} {}

  /**
   * @brief Returns the registered string's handle
   *
   */
/* 中文内部逻辑 F0834：get_handle
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return handle_;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0834
 */
/* 中文逐项-函数 F0834：get_handle
 * 用途与位置：读取包装保存的不透明NVTX注册字符串/域等句柄，必须交给对应NVTX API使用。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return handle_;。CHECK类宏还可能提前返回/跳转。
 */
  nvtxStringHandle_t get_handle() const noexcept { return handle_; }

  registered_string()                         = delete;
  ~registered_string()                        = default;
  registered_string(registered_string const&) = default;
  registered_string& operator=(registered_string const&) = default;
  registered_string(registered_string&&)                 = default;
  registered_string& operator=(registered_string&&) = default;

 private:
/* 中文逐项-变量 V3035：handle_
 * handle_（nvtxStringHandle_t；成员 registered_string）：NVTX域/注册对象的不透明句柄，工具API使用它定位标记域或字符串，不是GPU内存地址。
 */
  nvtxStringHandle_t const handle_{};  ///< The handle returned from
                                       ///< registering the message with NVTX
};

/**
 * @brief Allows associating a message string with an NVTX event via
 * its `EventAttribute`s.
 *
 * Associating a `message` with an NVTX event through its `event_attributes`
 * allows for naming events to easily differentiate them from other events.
 *
 * Every time an NVTX event is created with an associated `message`, the
 * contents of the message string must be copied.  This may cause non-trivial
 * overhead in highly performance sensitive sections of code. Use of a
 * `nvtx3::registered_string` is recommended in these situations.
 *
 * Example:
 * \code{.cpp}
 * // Creates an `event_attributes` with message "message 0"
 * nvtx3::event_attributes attr0{nvtx3::message{"message 0"}};
 *
 * // `range0` contains message "message 0"
 * nvtx3::thread_range range0{attr0};
 *
 * // `std::string` and string literals are implicitly assumed to be
 * // the contents of an `nvtx3::message`
 * // Creates an `event_attributes` with message "message 1"
 * nvtx3::event_attributes attr1{"message 1"};
 *
 * // `range1` contains message "message 1"
 * nvtx3::thread_range range1{attr1};
 *
 * // `range2` contains message "message 2"
 * nvtx3::thread_range range2{nvtx3::Mesage{"message 2"}};
 *
 * // `std::string` and string literals are implicitly assumed to be
 * // the contents of an `nvtx3::message`
 * // `range3` contains message "message 3"
 * nvtx3::thread_range range3{"message 3"};
 * \endcode
 */
/* 中文内部逻辑 F0835：get_value
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return value_;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0835
 */
/* 中文逐项-函数 F0835：get_value
 * 用途与位置：返回NVTX包装内部保存的颜色/消息/载荷union值，不复制或传输GPU张量。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return value_;。CHECK类宏还可能提前返回/跳转。
 */
/* 中文逐项-类型 T0354：message
 * 类型职责：本文件的class类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：registered。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
class message {
 public:
  using value_type = nvtxMessageValue_t;

  /**
   * @brief Construct a `message` whose contents are specified by `msg`.
   *
   * @param msg The contents of the message
   */
/* 中文内部逻辑 F0836：message
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：value_.ascii = msg;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0836
 */
/* 中文逐项-函数 F0836：message
 * 用途与位置：把ASCII、Unicode或注册字符串统一包装成带type标签的NVTX消息。
 * 参数 msg（char const* msg）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  NVTX3_RELAXED_CONSTEXPR message(char const* msg) noexcept : type_{NVTX_MESSAGE_TYPE_ASCII}
  {
    value_.ascii = msg;
  }

  /**
   * @brief Construct a `message` whose contents are specified by `msg`.
   *
   * @param msg The contents of the message
   */
/* 中文内部逻辑 F0837：message
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0837
 */
/* 中文逐项-函数 F0837：message
 * 用途与位置：把ASCII、Unicode或注册字符串统一包装成带type标签的NVTX消息。
 * 参数 <未命名参数>（std::string const& msg）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  message(std::string const& msg) noexcept : message{msg.c_str()} {}

  /**
   * @brief Disallow construction for `std::string` r-value
   *
   * `message` is a non-owning type and therefore cannot take ownership of an
   * r-value. Therefore, constructing from an r-value is disallowed to prevent
   * a dangling pointer.
   *
   */
  message(std::string&&) = delete;

  /**
   * @brief Construct a `message` whose contents are specified by `msg`.
   *
   * @param msg The contents of the message
   */
/* 中文内部逻辑 F0838：message
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：value_.unicode = msg;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0838
 */
/* 中文逐项-函数 F0838：message
 * 用途与位置：把ASCII、Unicode或注册字符串统一包装成带type标签的NVTX消息。
 * 参数 msg（wchar_t const* msg）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  NVTX3_RELAXED_CONSTEXPR message(wchar_t const* msg) noexcept : type_{NVTX_MESSAGE_TYPE_UNICODE}
  {
    value_.unicode = msg;
  }

  /**
   * @brief Construct a `message` whose contents are specified by `msg`.
   *
   * @param msg The contents of the message
   */
/* 中文内部逻辑 F0839：message
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0839
 */
/* 中文逐项-函数 F0839：message
 * 用途与位置：把ASCII、Unicode或注册字符串统一包装成带type标签的NVTX消息。
 * 参数 <未命名参数>（std::wstring const& msg）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  message(std::wstring const& msg) noexcept : message{msg.c_str()} {}

  /**
   * @brief Disallow construction for `std::wstring` r-value
   *
   * `message` is a non-owning type and therefore cannot take ownership of an
   * r-value. Therefore, constructing from an r-value is disallowed to prevent
   * a dangling pointer.
   *
   */
  message(std::wstring&&) = delete;

  /**
   * @brief Construct a `message` from a `registered_string`.
   *
   * @tparam D Type containing `name` member used to identify the `domain`
   * to which the `registered_string` belongs. Else, `domain::global` to
   * indicate that the global NVTX domain should be used.
   * @param msg The message that has already been registered with NVTX.
   */
  template <typename D>
/* 中文逐项-变量 V3036：registered
 * registered（NVTX3_RELAXED_CONSTEXPR；成员 message）：消息union中的ASCII/Unicode文本地址或已注册字符串句柄，只使用type指定的视图。
 * 初值/来源 registered = msg.get_handle()；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  NVTX3_RELAXED_CONSTEXPR message(registered_string<D> const& msg) noexcept
    : type_{NVTX_MESSAGE_TYPE_REGISTERED}
  {
    value_.registered = msg.get_handle();
  }

  /**
   * @brief Return the union holding the value of the message.
   *
   */
  NVTX3_RELAXED_CONSTEXPR value_type get_value() const noexcept { return value_; }

  /**
   * @brief Return the type information about the value the union holds.
   *
   */
/* 中文内部逻辑 F0840：nvtxMessageType_t get_type
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return type_;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0840
 */
/* 中文逐项-函数 F0840：nvtxMessageType_t get_type
 * 用途与位置：返回NVTX消息/颜色/载荷的类型标签，使消费者选择正确union视图。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return type_;。CHECK类宏还可能提前返回/跳转。
 */
  NVTX3_RELAXED_CONSTEXPR nvtxMessageType_t get_type() const noexcept { return type_; }

 private:
/* 中文逐项-变量 V3037：type_
 * type_（nvtxMessageType_t；文件/静态存储作用域）：NVTX颜色/消息/载荷的union类型标签，读取值前先看对应标签。
 * 初值/来源 type_ = {}；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  nvtxMessageType_t const type_{};  ///< message type
/* 中文逐项-变量 V3038：value_
 * value_（nvtxMessageValue_t；文件/静态存储作用域）：NVTX包装保存的标量/union值，配合类型字段解释。
 * 初值/来源 value_ = {}；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  nvtxMessageValue_t value_{};      ///< message contents
};

/**
 * @brief A numerical value that can be associated with an NVTX event via
 * its `event_attributes`.
 *
 * Example:
 * ```
 * nvtx3:: event_attributes attr{nvtx3::payload{42}}; // Constructs a payload
 * from
 *                                                 // the `int32_t` value 42
 *
 * // `range0` will have an int32_t payload of 42
 * nvtx3::thread_range range0{attr};
 *
 * // range1 has double payload of 3.14
 * nvtx3::thread_range range1{ nvtx3::payload{3.14} };
 * ```
 */
/* 中文逐项-类型 T0355：payload
 * 类型职责：本文件的class类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：explicit、explicit、explicit、explicit、explicit、explicit、nvtxPayloadType_t、type_、value_。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
class payload {
 public:
  using value_type = typename nvtxEventAttributes_v2::payload_t;

  /**
   * @brief Construct a `payload` from a signed, 8 byte integer.
   *
   * @param value Value to use as contents of the payload
   */
/* 中文逐项-变量 V3039：explicit
 * explicit（NVTX3_RELAXED_CONSTEXPR；成员 payload）：NVTX显式范围包装相关声明的解析项，真正语义以紧邻C++声明为准，不把关键字当运行态数据。
 */
  NVTX3_RELAXED_CONSTEXPR explicit /* 中文内部逻辑 F0841：payload
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：value_.llValue = value;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0841
 */
/* 中文逐项-函数 F0841：payload
 * 用途与位置：把某种标量保存成带类型标签的NVTX事件载荷，仅用于性能标记附加信息。
 * 参数 value（int64_t value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */payload(int64_t value) noexcept
    : type_{NVTX_PAYLOAD_TYPE_INT64}, value_{}
  {
    value_.llValue = value;
  }

  /**
   * @brief Construct a `payload` from a signed, 4 byte integer.
   *
   * @param value Value to use as contents of the payload
   */
/* 中文逐项-变量 V3040：explicit
 * explicit（NVTX3_RELAXED_CONSTEXPR；成员 payload）：NVTX显式范围包装相关声明的解析项，真正语义以紧邻C++声明为准，不把关键字当运行态数据。
 */
  NVTX3_RELAXED_CONSTEXPR explicit /* 中文内部逻辑 F0842：payload
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：value_.iValue = value;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0842
 */
/* 中文逐项-函数 F0842：payload
 * 用途与位置：把某种标量保存成带类型标签的NVTX事件载荷，仅用于性能标记附加信息。
 * 参数 value（int32_t value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */payload(int32_t value) noexcept
    : type_{NVTX_PAYLOAD_TYPE_INT32}, value_{}
  {
    value_.iValue = value;
  }

  /**
   * @brief Construct a `payload` from an unsigned, 8 byte integer.
   *
   * @param value Value to use as contents of the payload
   */
/* 中文逐项-变量 V3041：explicit
 * explicit（NVTX3_RELAXED_CONSTEXPR；成员 payload）：NVTX显式范围包装相关声明的解析项，真正语义以紧邻C++声明为准，不把关键字当运行态数据。
 */
  NVTX3_RELAXED_CONSTEXPR explicit /* 中文内部逻辑 F0843：payload
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：value_.ullValue = value;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0843
 */
/* 中文逐项-函数 F0843：payload
 * 用途与位置：把某种标量保存成带类型标签的NVTX事件载荷，仅用于性能标记附加信息。
 * 参数 value（uint64_t value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */payload(uint64_t value) noexcept
    : type_{NVTX_PAYLOAD_TYPE_UNSIGNED_INT64}, value_{}
  {
    value_.ullValue = value;
  }

  /**
   * @brief Construct a `payload` from an unsigned, 4 byte integer.
   *
   * @param value Value to use as contents of the payload
   */
/* 中文逐项-变量 V3042：explicit
 * explicit（NVTX3_RELAXED_CONSTEXPR；成员 payload）：NVTX显式范围包装相关声明的解析项，真正语义以紧邻C++声明为准，不把关键字当运行态数据。
 */
  NVTX3_RELAXED_CONSTEXPR explicit /* 中文内部逻辑 F0844：payload
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：value_.uiValue = value;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0844
 */
/* 中文逐项-函数 F0844：payload
 * 用途与位置：把某种标量保存成带类型标签的NVTX事件载荷，仅用于性能标记附加信息。
 * 参数 value（uint32_t value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */payload(uint32_t value) noexcept
    : type_{NVTX_PAYLOAD_TYPE_UNSIGNED_INT32}, value_{}
  {
    value_.uiValue = value;
  }

  /**
   * @brief Construct a `payload` from a single-precision floating point
   * value.
   *
   * @param value Value to use as contents of the payload
   */
/* 中文逐项-变量 V3043：explicit
 * explicit（NVTX3_RELAXED_CONSTEXPR；成员 payload）：NVTX显式范围包装相关声明的解析项，真正语义以紧邻C++声明为准，不把关键字当运行态数据。
 */
  NVTX3_RELAXED_CONSTEXPR explicit /* 中文内部逻辑 F0845：payload
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：value_.fValue = value;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0845
 */
/* 中文逐项-函数 F0845：payload
 * 用途与位置：把某种标量保存成带类型标签的NVTX事件载荷，仅用于性能标记附加信息。
 * 参数 value（float value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */payload(float value) noexcept
    : type_{NVTX_PAYLOAD_TYPE_FLOAT}, value_{}
  {
    value_.fValue = value;
  }

  /**
   * @brief Construct a `payload` from a double-precision floating point
   * value.
   *
   * @param value Value to use as contents of the payload
   */
/* 中文逐项-变量 V3044：explicit
 * explicit（NVTX3_RELAXED_CONSTEXPR；成员 payload）：NVTX显式范围包装相关声明的解析项，真正语义以紧邻C++声明为准，不把关键字当运行态数据。
 */
  NVTX3_RELAXED_CONSTEXPR explicit /* 中文内部逻辑 F0846：payload
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：value_.dValue = value;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0846
 */
/* 中文逐项-函数 F0846：payload
 * 用途与位置：把某种标量保存成带类型标签的NVTX事件载荷，仅用于性能标记附加信息。
 * 参数 value（double value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */payload(double value) noexcept
    : type_{NVTX_PAYLOAD_TYPE_DOUBLE}, value_{}
  {
    value_.dValue = value;
  }

  /**
   * @brief Return the union holding the value of the payload
   *
   */
/* 中文内部逻辑 F0847：get_value
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return value_;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0847
 */
/* 中文逐项-函数 F0847：get_value
 * 用途与位置：返回NVTX包装内部保存的颜色/消息/载荷union值，不复制或传输GPU张量。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return value_;。CHECK类宏还可能提前返回/跳转。
 */
  NVTX3_RELAXED_CONSTEXPR value_type get_value() const noexcept { return value_; }

  /**
   * @brief Return the information about the type the union holds.
   *
   */
/* 中文逐项-变量 V3045：nvtxPayloadType_t
 * nvtxPayloadType_t（NVTX3_RELAXED_CONSTEXPR；成员 payload）：NVTX载荷类型标签的声明/解析项，具体使用仍由payloadType和union对应。
 */
  NVTX3_RELAXED_CONSTEXPR nvtxPayloadType_t /* 中文内部逻辑 F0848：get_type
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return type_;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0848
 */
/* 中文逐项-函数 F0848：get_type
 * 用途与位置：返回NVTX消息/颜色/载荷的类型标签，使消费者选择正确union视图。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return type_;。CHECK类宏还可能提前返回/跳转。
 */get_type() const noexcept { return type_; }

 private:
/* 中文逐项-变量 V3046：type_
 * type_（nvtxPayloadType_t；成员 payload）：NVTX颜色/消息/载荷的union类型标签，读取值前先看对应标签。
 */
  nvtxPayloadType_t const type_;  ///< Type of the payload value
/* 中文逐项-变量 V3047：value_
 * value_（value_type；成员 payload）：NVTX包装保存的标量/union值，配合类型字段解释。
 */
  value_type value_;              ///< Union holding the payload value
};

/**
 * @brief Describes the attributes of a NVTX event.
 *
 * NVTX events can be customized via four "attributes":
 *
 * - color:    color used to visualize the event in tools such as Nsight
 *             Systems. See `color`.
 * - message:  Custom message string. See `message`.
 * - payload:  User-defined numerical value. See `payload`.
 * - category: Intra-domain grouping. See `category`.
 *
 * These component attributes are specified via an `event_attributes` object.
 * See `nvtx3::color`, `nvtx3::message`, `nvtx3::payload`, and
 * `nvtx3::category` for how these individual attributes are constructed.
 *
 * While it is possible to specify all four attributes, it is common to want
 * to only specify a subset of attributes and use default values for the
 * others. For convenience, `event_attributes` can be constructed from any
 * number of attribute components in any order.
 *
 * Example:
 * \code{.cpp}
 * event_attributes attr{}; // No arguments, use defaults for all attributes
 *
 * event_attributes attr{"message"}; // Custom message, rest defaulted
 *
 * // Custom color & message
 * event_attributes attr{"message", nvtx3::rgb{127, 255, 0}};
 *
 * /// Custom color & message, can use any order of arguments
 * event_attributes attr{nvtx3::rgb{127, 255, 0}, "message"};
 *
 *
 * // Custom color, message, payload, category
 * event_attributes attr{nvtx3::rgb{127, 255, 0},
 *                      "message",
 *                      nvtx3::payload{42},
 *                      nvtx3::category{1}};
 *
 * // Custom color, message, payload, category, can use any order of arguments
 * event_attributes attr{nvtx3::payload{42},
 *                      nvtx3::category{1},
 *                      "message",
 *                      nvtx3::rgb{127, 255, 0}};
 *
 * // Multiple arguments of the same type are allowed, but only the first is
 * // used. All others are ignored
 * event_attributes attr{ nvtx3::payload{42}, nvtx3::payload{7} }; // payload
 * is 42
 *
 * // Range `r` will be customized according the attributes in `attr`
 * nvtx3::thread_range r{attr};
 *
 * // For convenience, the arguments that can be passed to the
 * `event_attributes`
 * // constructor may be passed to the `domain_thread_range` contructor where
 * // they will be forwarded to the `EventAttribute`s constructor
 * nvtx3::thread_range r{nvtx3::payload{42}, nvtx3::category{1}, "message"};
 * \endcode
 *
 */
/* 中文逐项-类型 T0356：event_attributes
 * 类型职责：本文件的class类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：attributes_。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
class event_attributes {
 public:
  using value_type = nvtxEventAttributes_t;

  /**
   * @brief Default constructor creates an `event_attributes` with no
   * category, color, payload, nor message.
   */
/* 中文内部逻辑 F0849：event_attributes
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0849
 */
/* 中文逐项-函数 F0849：event_attributes
 * 用途与位置：将颜色、类别、消息、载荷等参数组合成版本化NVTX事件属性结构，默认字段按库契约初始化。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  constexpr event_attributes() noexcept
    : attributes_{
        NVTX_VERSION,                   // version
        sizeof(nvtxEventAttributes_t),  // size
        0,                              // category
        NVTX_COLOR_UNKNOWN,             // color type
        0,                              // color value
        NVTX_PAYLOAD_UNKNOWN,           // payload type
        0,                              // reserved 4B
        0,                              // payload value (union)
        NVTX_MESSAGE_UNKNOWN,           // message type
        0                               // message value (union)
      }
  {
  }

  /**
   * @brief Variadic constructor where the first argument is a `category`.
   *
   * Sets the value of the `EventAttribute`s category based on `c` and
   * forwards the remaining variadic parameter pack to the next constructor.
   *
   */
/* 中文内部逻辑 F0850：event_attributes
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：attributes_.category = c.get_id();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0850
 */
/* 中文逐项-函数 F0850：event_attributes
 * 用途与位置：将颜色、类别、消息、载荷等参数组合成版本化NVTX事件属性结构，默认字段按库契约初始化。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：c.get_id。
 */
  template <typename... Args>
  NVTX3_RELAXED_CONSTEXPR explicit event_attributes(category const& c, Args const&... args) noexcept
    : event_attributes(args...)
  {
    attributes_.category = c.get_id();
  }

  /**
   * @brief Variadic constructor where the first argument is a `color`.
   *
   * Sets the value of the `EventAttribute`s color based on `c` and forwards
   * the remaining variadic parameter pack to the next constructor.
   *
   */
/* 中文内部逻辑 F0851：event_attributes
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：attributes_.color = c.get_value();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：attributes_.colorType = c.get_type();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0851
 */
/* 中文逐项-函数 F0851：event_attributes
 * 用途与位置：将颜色、类别、消息、载荷等参数组合成版本化NVTX事件属性结构，默认字段按库契约初始化。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：c.get_type；c.get_value。
 */
  template <typename... Args>
  NVTX3_RELAXED_CONSTEXPR explicit event_attributes(color const& c, Args const&... args) noexcept
    : event_attributes(args...)
  {
    attributes_.color     = c.get_value();
    attributes_.colorType = c.get_type();
  }

  /**
   * @brief Variadic constructor where the first argument is a `payload`.
   *
   * Sets the value of the `EventAttribute`s payload based on `p` and forwards
   * the remaining variadic parameter pack to the next constructor.
   *
   */
/* 中文内部逻辑 F0852：event_attributes
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：attributes_.payload = p.get_value();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：attributes_.payloadType = p.get_type();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0852
 */
/* 中文逐项-函数 F0852：event_attributes
 * 用途与位置：将颜色、类别、消息、载荷等参数组合成版本化NVTX事件属性结构，默认字段按库契约初始化。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：p.get_type；p.get_value。
 */
  template <typename... Args>
  NVTX3_RELAXED_CONSTEXPR explicit event_attributes(payload const& p, Args const&... args) noexcept
    : event_attributes(args...)
  {
    attributes_.payload     = p.get_value();
    attributes_.payloadType = p.get_type();
  }

  /**
   * @brief Variadic constructor where the first argument is a `message`.
   *
   * Sets the value of the `EventAttribute`s message based on `m` and forwards
   * the remaining variadic parameter pack to the next constructor.
   *
   */
/* 中文内部逻辑 F0853：event_attributes
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：attributes_.message = m.get_value();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：attributes_.messageType = m.get_type();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0853
 */
/* 中文逐项-函数 F0853：event_attributes
 * 用途与位置：将颜色、类别、消息、载荷等参数组合成版本化NVTX事件属性结构，默认字段按库契约初始化。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：m.get_type；m.get_value。
 */
  template <typename... Args>
  NVTX3_RELAXED_CONSTEXPR explicit event_attributes(message const& m, Args const&... args) noexcept
    : event_attributes(args...)
  {
    attributes_.message     = m.get_value();
    attributes_.messageType = m.get_type();
  }

  ~event_attributes()                       = default;
  event_attributes(event_attributes const&) = default;
  event_attributes& operator=(event_attributes const&) = default;
  event_attributes(event_attributes&&)                 = default;
  event_attributes& operator=(event_attributes&&) = default;

  /**
   * @brief Get raw pointer to underlying NVTX attributes object.
   *
   */
/* 中文内部逻辑 F0854：get
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return &attributes_;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0854
 */
/* 中文逐项-函数 F0854：get
 * 用途与位置：NVTX包装的construct-on-first-use入口，按DomainName/类别/字符串模板创建并复用对应的静态注册对象。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3.hpp::named_category；src/include/nvtx3.hpp::domain_thread_range；src/include/nvtx3.hpp::~domain_thread_range；src/include/nvtx3.hpp::start_range；src/include/nvtx3.hpp::mark。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return &attributes_;。CHECK类宏还可能提前返回/跳转。
 */
  constexpr value_type const* get() const noexcept { return &attributes_; }

 private:
/* 中文逐项-变量 V3048：attributes_
 * attributes_（value_type；成员 event_attributes）：NVTX C++包装合成的事件属性对象，传给底层标记API；不是NCCL操作参数。
 */
  value_type attributes_{};  ///< The NVTX attributes structure
};

/**
 * @brief A RAII object for creating a NVTX range local to a thread within a
 * domain.
 *
 * When constructed, begins a nested NVTX range on the calling thread in the
 * specified domain. Upon destruction, ends the NVTX range.
 *
 * Behavior is undefined if a `domain_thread_range` object is
 * created/destroyed on different threads.
 *
 * `domain_thread_range` is neither moveable nor copyable.
 *
 * `domain_thread_range`s may be nested within other ranges.
 *
 * The domain of the range is specified by the template type parameter `D`.
 * By default, the `domain::global` is used, which scopes the range to the
 * global NVTX domain. The convenience alias `thread_range` is provided for
 * ranges scoped to the global domain.
 *
 * A custom domain can be defined by creating a type, `D`, with a static
 * member `D::name` whose value is used to name the domain associated with
 * `D`. `D::name` must resolve to either `char const*` or `wchar_t const*`
 *
 * Example:
 * ```
 * // Define a type `my_domain` with a member `name` used to name the domain
 * // associated with the type `my_domain`.
 * struct my_domain{
 *    static constexpr const char * name{"my domain"};
 * };
 * ```
 *
 * Usage:
 * ```
 * nvtx3::domain_thread_range<> r0{"range 0"}; // Range in global domain
 *
 * nvtx3::thread_range r1{"range 1"}; // Alias for range in global domain
 *
 * nvtx3::domain_thread_range<my_domain> r2{"range 2"}; // Range in custom
 * domain
 *
 * // specify an alias to a range that uses a custom domain
 * using my_thread_range = nvtx3::domain_thread_range<my_domain>;
 *
 * my_thread_range r3{"range 3"}; // Alias for range in custom domain
 * ```
 */
/* 中文逐项-类型 T0357：domain_thread_range
 * 类型职责：本文件的class类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 */
template <class D = domain::global>
class domain_thread_range {
 public:
  /**
   * @brief Construct a `domain_thread_range` with the specified
   * `event_attributes`
   *
   * Example:
   * ```
   * nvtx3::event_attributes attr{"msg", nvtx3::rgb{127,255,0}};
   * nvtx3::domain_thread_range<> range{attr}; // Creates a range with message
   * contents
   *                                    // "msg" and green color
   * ```
   *
   * @param[in] attr `event_attributes` that describes the desired attributes
   * of the range.
   */
/* 中文内部逻辑 F0855：domain_thread_range
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 调用 / 计算 / 写入：nvtxDomainRangePushEx(domain::get<D>(), attr.get());；在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 调用 / 计算 / 写入：(void)attr;；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0855
 */
/* 中文逐项-函数 F0855：domain_thread_range
 * 用途与位置：RAII范围对象：构造时在当前线程push一个NVTX范围，析构时配对pop。
 * 参数 <未命名参数>（event_attributes const& attr）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：attr.get；domain::get<D>；nvtxDomainRangePushEx。
 */
  explicit domain_thread_range(event_attributes const& attr) noexcept
  {
#ifndef NVTX_DISABLE
    nvtxDomainRangePushEx(domain::get<D>(), attr.get());
#else
    (void)attr;
#endif
  }

  /**
   * @brief Constructs a `domain_thread_range` from the constructor arguments
   * of an `event_attributes`.
   *
   * Forwards the arguments `first, args...` to construct an
   * `event_attributes` object. The `event_attributes` object is then
   * associated with the `domain_thread_range`.
   *
   * For more detail, see `event_attributes` documentation.
   *
   * Example:
   * ```
   * // Creates a range with message "message" and green color
   * nvtx3::domain_thread_range<> r{"message", nvtx3::rgb{127,255,0}};
   * ```
   *
   * @note To prevent making needless copies of `event_attributes` objects,
   * this constructor is disabled when the first argument is an
   * `event_attributes` object, instead preferring the explicit
   * `domain_thread_range(event_attributes const&)` constructor.
   *
   * @param[in] first First argument to forward to the `event_attributes`
   * constructor.
   * @param[in] args Variadic parameter pack of additional arguments to
   * forward.
   *
   */
/* 中文内部逻辑 F0856：domain_thread_range
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0856
 */
/* 中文逐项-函数 F0856：domain_thread_range
 * 用途与位置：RAII范围对象：构造时在当前线程push一个NVTX范围，析构时配对pop。
 * 参数 <未命名参数>（First const& first）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  template <typename First,
            typename... Args,
            typename = typename std::enable_if<
              !std::is_same<event_attributes, typename std::decay<First>>::value>>
  explicit domain_thread_range(First const& first, Args const&... args) noexcept
    : domain_thread_range{event_attributes{first, args...}}
  {
  }

  /**
   * @brief Default constructor creates a `domain_thread_range` with no
   * message, color, payload, nor category.
   *
   */
/* 中文内部逻辑 F0857：domain_thread_range
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0857
 */
/* 中文逐项-函数 F0857：domain_thread_range
 * 用途与位置：RAII范围对象：构造时在当前线程push一个NVTX范围，析构时配对pop。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  domain_thread_range() : domain_thread_range{event_attributes{}} {}

  /**
   * @brief Delete `operator new` to disallow heap allocated objects.
   *
   * `domain_thread_range` must follow RAII semantics to guarantee proper push/pop semantics.
   *
   */
  void* operator new(std::size_t) = delete;

  domain_thread_range(domain_thread_range const&) = delete;
  domain_thread_range& operator=(domain_thread_range const&) = delete;
  domain_thread_range(domain_thread_range&&)                 = delete;
  domain_thread_range& operator=(domain_thread_range&&) = delete;

  /**
   * @brief Destroy the domain_thread_range, ending the NVTX range event.
   */
/* 中文内部逻辑 F0858：~domain_thread_range
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 调用 / 计算 / 写入：nvtxDomainRangePop(domain::get<D>());；从当前线程NVTX范围栈pop结束最近区间。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0858
 */
/* 中文逐项-函数 F0858：~domain_thread_range
 * 用途与位置：结束当前线程RAII对象管理的NVTX范围，保证离开C++作用域时标记配对。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：domain::get<D>；nvtxDomainRangePop。
 */
  ~domain_thread_range() noexcept
  {
#ifndef NVTX_DISABLE
    nvtxDomainRangePop(domain::get<D>());
#endif
  }
};

/**
 * @brief Alias for a `domain_thread_range` in the global NVTX domain.
 *
 */
using thread_range = domain_thread_range<>;

/**
 * @brief Handle used for correlating explicit range start and end events.
 *
 */
/* 中文逐项-类型 T0358：range_handle
 * 类型职责：本文件的struct类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：_range_id。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct range_handle {
  /// Type used for the handle's value
  using value_type = nvtxRangeId_t;

  /**
   * @brief Construct a `range_handle` from the given id.
   *
   */
/* 中文内部逻辑 F0859：range_handle
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0859
 */
/* 中文逐项-函数 F0859：range_handle
 * 用途与位置：保存跨线程/进程范围start返回的NVTX id及域，以便之后显式结束。
 * 参数 id（value_type id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  constexpr explicit range_handle(value_type id) noexcept : _range_id{id} {}

  /**
   * @brief Returns the `range_handle`'s value
   *
   * @return value_type The handle's value
   */
/* 中文内部逻辑 F0860：get_value
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return _range_id;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0860
 */
/* 中文逐项-函数 F0860：get_value
 * 用途与位置：返回NVTX包装内部保存的颜色/消息/载荷union值，不复制或传输GPU张量。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return _range_id;。CHECK类宏还可能提前返回/跳转。
 */
  constexpr value_type get_value() const noexcept { return _range_id; }

 private:
/* 中文逐项-变量 V3049：_range_id
 * _range_id（value_type；成员 range_handle）：NVTX范围或类别id，供start/end或类别分组匹配，与NCCL rank无关。
 */
  value_type _range_id{};  ///< The underlying NVTX range id
};

/**
 * @brief Manually begin an NVTX range.
 *
 * Explicitly begins an NVTX range and returns a unique handle. To end the
 * range, pass the handle to `end_range()`.
 *
 * `start_range/end_range` are the most explicit and lowest level APIs provided
 * for creating ranges.  Use of `nvtx3::domain_process_range` should be
 * preferred unless one is unable to tie the range to the lifetime of an object.
 *
 * Example:
 * ```
 * nvtx3::event_attributes attr{"msg", nvtx3::rgb{127,255,0}};
 * nvtx3::range_handle h = nvxt3::start_range(attr); // Manually begins a range
 * ...
 * nvtx3::end_range(h); // Ends the range
 * ```
 *
 * @tparam D Type containing `name` member used to identify the `domain`
 * to which the range belongs. Else, `domain::global` to indicate that the
 * global NVTX domain should be used.
 * @param[in] attr `event_attributes` that describes the desired attributes
 * of the range.
 * @return Unique handle to be passed to `end_range` to end the range.
 */
/* 中文内部逻辑 F0861：start_range
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 函数返回：return range_handle{nvtxDomainRangeStartEx(domain::get<D>(), attr.get())};；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 调用 / 计算 / 写入：(void)attr;；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 *     5. 函数返回：return range_handle{};；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0861
 */
/* 中文逐项-函数 F0861：start_range
 * 用途与位置：开始可显式结束的NVTX范围并返回句柄，标记被测时间区间而不执行通信。
 * 参数 <未命名参数>（event_attributes const& attr）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3.hpp::start_range。
 * 本体直接出现的调用/包装：attr.get；domain::get<D>；nvtxDomainRangeStartEx。
 * 源码返回表达式：return range_handle{nvtxDomainRangeStartEx(domain::get<D>(), attr.get())};；return
 * range_handle{};。CHECK类宏还可能提前返回/跳转。
 */
template <typename D = domain::global>
range_handle start_range(event_attributes const& attr) noexcept
{
#ifndef NVTX_DISABLE
  return range_handle{nvtxDomainRangeStartEx(domain::get<D>(), attr.get())};
#else
  (void)attr;
  return range_handle{};
#endif
}

/**
 * @brief Manually begin an NVTX range.
 *
 * Explicitly begins an NVTX range and returns a unique handle. To end the
 * range, pass the handle to `end_range()`.
 *
 * Forwards the arguments `first, args...` to construct an  `event_attributes`
 * object. The `event_attributes` object is then  associated with the range.
 *
 * For more detail, see `event_attributes` documentation.
 *
 * Example:
 * ```
 * nvtx3::range_handle h = nvxt3::start_range("msg", nvtx3::rgb{127,255,0}); //
 * Begin range
 * ...
 * nvtx3::end_range(h); // Ends the range
 * ```
 *
 * `start_range/end_range` are the most explicit and lowest level APIs provided
 * for creating ranges.  Use of `nvtx3::domain_process_range` should be
 * preferred unless one is unable to tie the range to the lifetime of an object.
 *
 * @param first[in] First argument to pass to an `event_attributes`
 * @param args[in] Variadiac parameter pack of the rest of the arguments for an
 * `event_attributes`.
 * @return Unique handle to be passed to `end_range` to end the range.
 */
/* 中文内部逻辑 F0862：start_range
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 函数返回：return start_range(event_attributes{first, args...});；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 调用 / 计算 / 写入：(void)first;；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 *     5. 函数返回：return range_handle{};；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0862
 */
/* 中文逐项-函数 F0862：start_range
 * 用途与位置：开始可显式结束的NVTX范围并返回句柄，标记被测时间区间而不执行通信。
 * 参数 <未命名参数>（First const& first）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3.hpp::start_range。
 * 本体直接出现的调用/包装：start_range。
 * 源码返回表达式：return start_range(event_attributes{first, args...});；return range_handle{};。CHECK类宏还可能提前返回/跳转。
 */
template <typename First,
          typename... Args,
          typename = typename std::enable_if<
            !std::is_same<event_attributes, typename std::decay<First>>::value>>
range_handle start_range(First const& first, Args const&... args) noexcept
{
#ifndef NVTX_DISABLE
  return start_range(event_attributes{first, args...});
#else
  (void)first;
  return range_handle{};
#endif
}

/**
 * @brief Manually end the range associated with the handle `r`.
 *
 * Explicitly ends the NVTX range indicated by the handle `r` returned from a
 * prior call to `start_range`. The range may end on a different thread from
 * where it began.
 *
 * This function does not have a Domain tag type template parameter as the
 * handle `r` already indicates the domain to which the range belongs.
 *
 * @param r Handle to a range started by a prior call to `start_range`.
 */
/* 中文内部逻辑 F0863：end_range
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 调用 / 计算 / 写入：nvtxRangeEnd(r.get_value());；按id结束显式NVTX时间范围。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 调用 / 计算 / 写入：(void)r;；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0863
 */
/* 中文逐项-函数 F0863：end_range
 * 用途与位置：根据保存的域和范围id结束NVTX范围，与start_range配对。
 * 参数 r（range_handle r）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3.hpp::~domain_process_range。
 * 本体直接出现的调用/包装：nvtxRangeEnd；r.get_value。
 */
inline void end_range(range_handle r)
{
#ifndef NVTX_DISABLE
  nvtxRangeEnd(r.get_value());
#else
  (void)r;
#endif
}

/**
 * @brief A RAII object for creating a NVTX range within a domain that can
 * be created and destroyed on different threads.
 *
 * When constructed, begins a NVTX range in the specified domain. Upon
 * destruction, ends the NVTX range.
 *
 * Similar to `nvtx3::domain_thread_range`, the only difference being that
 * `domain_process_range` can start and end on different threads.
 *
 * Use of `nvtx3::domain_thread_range` should be preferred unless one needs
 * the ability to start and end a range on different threads.
 *
 * `domain_process_range` is moveable, but not copyable.
 *
 * @tparam D Type containing `name` member used to identify the `domain`
 * to which the `domain_process_range` belongs. Else, `domain::global` to
 * indicate that the global NVTX domain should be used.
 */
/* 中文逐项-类型 T0359：domain_process_range
 * 类型职责：本文件的class类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：handle_。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template <typename D = domain::global>
class domain_process_range {
 public:
  /**
   * @brief Construct a new domain process range object
   *
   * @param attr
   */
/* 中文内部逻辑 F0864：domain_process_range
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0864
 */
/* 中文逐项-函数 F0864：domain_process_range
 * 用途与位置：RAII包装显式NVTX范围，构造start、析构end；范围不要求只依赖线程push/pop栈。
 * 参数 <未命名参数>（event_attributes const& attr）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  explicit domain_process_range(event_attributes const& attr) noexcept
    : handle_{new range_handle{start_range<D>(attr)}}
  {
  }

  /**
   * @brief Construct a new domain process range object
   *
   * @param first
   * @param args
   */
/* 中文内部逻辑 F0865：domain_process_range
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0865
 */
/* 中文逐项-函数 F0865：domain_process_range
 * 用途与位置：RAII包装显式NVTX范围，构造start、析构end；范围不要求只依赖线程push/pop栈。
 * 参数 <未命名参数>（First const& first）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  template <typename First,
            typename... Args,
            typename = typename std::enable_if<
              !std::is_same<event_attributes, typename std::decay<First>>::value>>
  explicit domain_process_range(First const& first, Args const&... args) noexcept
    : domain_process_range{event_attributes{first, args...}}
  {
  }

  /**
   * @brief Construct a new domain process range object
   *
   */
/* 中文内部逻辑 F0866：domain_process_range
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0866
 */
/* 中文逐项-函数 F0866：domain_process_range
 * 用途与位置：RAII包装显式NVTX范围，构造start、析构end；范围不要求只依赖线程push/pop栈。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  constexpr domain_process_range() noexcept : domain_process_range{event_attributes{}} {}

  /**
   * @brief Destroy the `domain_process_range` ending the range.
   *
   */
/* 中文内部逻辑 F0867：~domain_process_range
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(handle_)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：end_range(*handle_);；根据保存的域和范围id结束NVTX范围，与start_range配对。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0867
 */
/* 中文逐项-函数 F0867：~domain_process_range
 * 用途与位置：以保存的NVTX域和id结束该RAII范围，不等待NCCL GPU通信完成。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：end_range。
 * 需要对照的边界/推进条件：handle_。各条件所属分支见紧邻函数体。
 */
  ~domain_process_range()
  {
    if (handle_) { end_range(*handle_); }
  }

  /**
   * @brief Move constructor allows taking ownership of the NVTX range from
   * another `domain_process_range`.
   *
   * @param other
   */
  domain_process_range(domain_process_range&& other) = default;

  /**
   * @brief Move assignment operator allows taking ownership of an NVTX range
   * from another `domain_process_range`.
   *
   * @param other
   * @return domain_process_range&
   */
  domain_process_range& operator=(domain_process_range&& other) = default;

  /// Copy construction is not allowed to prevent multiple objects from owning
  /// the same range handle
  domain_process_range(domain_process_range const&) = delete;

  /// Copy assignment is not allowed to prevent multiple objects from owning the
  /// same range handle
  domain_process_range& operator=(domain_process_range const&) = delete;

 private:
/* 中文逐项-变量 V3050：handle_
 * handle_（std::unique_ptr<range_handle>；成员 domain_process_range）：NVTX域/注册对象的不透明句柄，工具API使用它定位标记域或字符串，不是GPU内存地址。
 */
  std::unique_ptr<range_handle> handle_;  ///< Range handle used to correlate
                                          ///< the start/end of the range
};

/**
 * @brief Alias for a `domain_process_range` in the global NVTX domain.
 *
 */
using process_range = domain_process_range<>;

/**
 * @brief Annotates an instantaneous point in time with the attributes specified
 * by `attr`.
 *
 * Unlike a "range", a mark is an instantaneous event in an application, e.g.,
 * locking/unlocking a mutex.
 *
 * \code{.cpp}
 * std::mutex global_lock;
 * void lock_mutex(){
 *    global_lock.lock();
 *    nvtx3::mark("lock_mutex");
 * }
 * \endcode
 *
 * @tparam D Type containing `name` member used to identify the `domain`
 * to which the `domain_process_range` belongs. Else, `domain::global` to
 * indicate that the global NVTX domain should be used.
 * @param[in] attr `event_attributes` that describes the desired attributes
 * of the mark.
 */
/* 中文内部逻辑 F0868：mark
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 调用 / 计算 / 写入：nvtxDomainMarkEx(domain::get<D>(), attr.get());；向性能工具记录一个瞬时标记。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 调用 / 计算 / 写入：(void)(attr);；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3_hpp.md#f0868
 */
/* 中文逐项-函数 F0868：mark
 * 用途与位置：提交一个瞬时时间点NVTX事件，不建立持续区间、不做张量传输。
 * 参数 <未命名参数>（event_attributes const& attr）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：attr.get；domain::get<D>；nvtxDomainMarkEx。
 */
template <typename D = domain::global>
inline void mark(event_attributes const& attr) noexcept
{
#ifndef NVTX_DISABLE
  nvtxDomainMarkEx(domain::get<D>(), attr.get());
#else
  (void)(attr);
#endif
}

}  // namespace NVTX3_VERSION_NAMESPACE

}  // namespace nvtx3

/**
 * @brief Convenience macro for generating a range in the specified `domain`
 * from the lifetime of a function
 *
 * This macro is useful for generating an NVTX range in `domain` from
 * the entry point of a function to its exit. It is intended to be the first
 * line of the function.
 *
 * Constructs a static `registered_string` using the name of the immediately
 * enclosing function returned by `__func__` and constructs a
 * `nvtx3::thread_range` using the registered function name as the range's
 * message.
 *
 * Example:
 * ```
 * struct my_domain{static constexpr char const* name{"my_domain"};};
 *
 * void foo(...){
 *    NVTX3_FUNC_RANGE_IN(my_domain); // Range begins on entry to foo()
 *    // do stuff
 *    ...
 * } // Range ends on return from foo()
 * ```
 *
 * @param[in] D Type containing `name` member used to identify the
 * `domain` to which the `registered_string` belongs. Else,
 * `domain::global` to  indicate that the global NVTX domain should be used.
 */
#ifndef NVTX_DISABLE
/* 中文逐项-宏 M0119：NVTX3_V1_FUNC_RANGE_IN
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX3_V1_FUNC_RANGE_IN；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX3_V1_FUNC_RANGE_IN(D)                                                  \
  static ::nvtx3::v1::registered_string<D> const nvtx3_func_name__{__func__};      \
  static ::nvtx3::v1::event_attributes const nvtx3_func_attr__{nvtx3_func_name__}; \
  ::nvtx3::v1::domain_thread_range<D> const nvtx3_range__{nvtx3_func_attr__};
#else
/* 中文逐项-宏 M0120：NVTX3_V1_FUNC_RANGE_IN
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX3_V1_FUNC_RANGE_IN；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX3_V1_FUNC_RANGE_IN(D)
#endif

/**
 * @brief Convenience macro for generating a range in the global domain from the
 * lifetime of a function.
 *
 * This macro is useful for generating an NVTX range in the global domain from
 * the entry point of a function to its exit. It is intended to be the first
 * line of the function.
 *
 * Constructs a static `registered_string` using the name of the immediately
 * enclosing function returned by `__func__` and constructs a
 * `nvtx3::thread_range` using the registered function name as the range's
 * message.
 *
 * Example:
 * ```
 * void foo(...){
 *    NVTX3_FUNC_RANGE(); // Range begins on entry to foo()
 *    // do stuff
 *    ...
 * } // Range ends on return from foo()
 * ```
 */
/* 中文逐项-宏 M0121：NVTX3_V1_FUNC_RANGE
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX3_V1_FUNC_RANGE；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX3_V1_FUNC_RANGE() NVTX3_V1_FUNC_RANGE_IN(::nvtx3::v1::domain::global)

/* When inlining this version, versioned macros must have unversioned aliases.
 * For each NVTX3_Vx_ #define, make an NVTX3_ alias of it here.*/
#if defined(NVTX3_INLINE_THIS_VERSION)
/* clang format off */
/* 中文逐项-常量 X1041：NVTX3_FUNC_RANGE_IN
 * 编译期定义：NVTX3_FUNC_RANGE_IN = NVTX3_V1_FUNC_RANGE_IN。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX3_FUNC_RANGE_IN   NVTX3_V1_FUNC_RANGE_IN
/* 中文逐项-常量 X1042：NVTX3_FUNC_RANGE
 * 编译期定义：NVTX3_FUNC_RANGE = NVTX3_V1_FUNC_RANGE。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX3_FUNC_RANGE      NVTX3_V1_FUNC_RANGE
/* clang format on */
#endif

#endif  // NVTX3_CPP_DEFINITIONS_V1_0

/* Add functionality for new minor versions here, by copying the above section enclosed
 * in #ifndef NVTX3_CPP_DEFINITIONS_Vx_y, and incrementing the minor version.  This code
 * is an example of how additions for version 1.2 would look, indented for clarity.  Note
 * that the versioned symbols and macros are always provided, and the unversioned symbols
 * are only provided if NVTX3_INLINE_THIS_VERSION was defined at the top of this header.
 *
 * \code{.cpp}
 * #ifndef NVTX3_CPP_DEFINITIONS_V1_2
 * #define NVTX3_CPP_DEFINITIONS_V1_2
 *     namespace nvtx3 {
 *         NVTX3_INLINE_IF_REQUESTED namespace NVTX3_VERSION_NAMESPACE {
 *             class new_class {};
 *             inline void new_function() {}
 *         }
 *     }
 *
 *     // Macros must have the major version in their names:
 *     #define NVTX3_V1_NEW_MACRO_A() ...
 *     #define NVTX3_V1_NEW_MACRO_B() ...
 *
 *     // If inlining, make aliases for the macros with the version number omitted
 *     #if defined(NVTX3_INLINE_THIS_VERSION)
 *         #define NVTX3_NEW_MACRO_A NVTX3_V1_NEW_MACRO_A
 *         #define NVTX3_NEW_MACRO_B NVTX3_V1_NEW_MACRO_B
 *     #endif
 * #endif // NVTX3_CPP_DEFINITIONS_V1_2
 * \endcode
 */

/* Undefine all temporarily-defined unversioned macros, which would conflict with
 * subsequent includes of different versions of this header. */
#undef NVTX3_CPP_VERSION_MAJOR
#undef NVTX3_CPP_VERSION_MINOR
#undef NVTX3_CONCAT
#undef NVTX3_NAMESPACE_FOR
#undef NVTX3_VERSION_NAMESPACE
#undef NVTX3_INLINE_IF_REQUESTED
#undef NVTX3_RELAXED_CONSTEXPR

#if defined(NVTX3_INLINE_THIS_VERSION)
#undef NVTX3_INLINE_THIS_VERSION
#endif
