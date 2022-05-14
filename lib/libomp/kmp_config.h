/*
 * kmp_config.h -- Feature macros
 */
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef KMP_CONFIG_H
#define KMP_CONFIG_H

#include "kmp_platform.h"

// cmakedefine01 MACRO will define MACRO as either 0 or 1
// cmakedefine MACRO 1 will define MACRO as 1 or leave undefined
#define DEBUG_BUILD 0
#define RELWITHDEBINFO_BUILD 0
#define LIBOMP_USE_ITT_NOTIFY 1
#define USE_ITT_NOTIFY LIBOMP_USE_ITT_NOTIFY
#if ! LIBOMP_USE_ITT_NOTIFY
# define INTEL_NO_ITTNOTIFY_API
#endif
#define LIBOMP_USE_VERSION_SYMBOLS 1
#if LIBOMP_USE_VERSION_SYMBOLS
# define KMP_USE_VERSION_SYMBOLS
#endif
#define LIBOMP_HAVE_WEAK_ATTRIBUTE 1
#define KMP_HAVE_WEAK_ATTRIBUTE LIBOMP_HAVE_WEAK_ATTRIBUTE
#define LIBOMP_HAVE_PSAPI 0
#define KMP_HAVE_PSAPI LIBOMP_HAVE_PSAPI
#define LIBOMP_STATS 0
#define KMP_STATS_ENABLED LIBOMP_STATS
#define LIBOMP_HAVE_X86INTRIN_H 0
#define KMP_HAVE_X86INTRIN_H LIBOMP_HAVE_X86INTRIN_H
#define LIBOMP_HAVE___BUILTIN_READCYCLECOUNTER 0
#define KMP_HAVE___BUILTIN_READCYCLECOUNTER LIBOMP_HAVE___BUILTIN_READCYCLECOUNTER
#define LIBOMP_HAVE___RDTSC 0
#define KMP_HAVE___RDTSC LIBOMP_HAVE___RDTSC
#define LIBOMP_USE_DEBUGGER 0
#define USE_DEBUGGER LIBOMP_USE_DEBUGGER
#define LIBOMP_OMPT_DEBUG 0
#define OMPT_DEBUG LIBOMP_OMPT_DEBUG
#define LIBOMP_OMPT_SUPPORT 1
#define OMPT_SUPPORT LIBOMP_OMPT_SUPPORT
#define LIBOMP_OMPD_SUPPORT 0
#define OMPD_SUPPORT LIBOMP_OMPD_SUPPORT
#define LIBOMP_PROFILING_SUPPORT 0
#define OMP_PROFILING_SUPPORT LIBOMP_PROFILING_SUPPORT
#define LIBOMP_OMPT_OPTIONAL 1
#define OMPT_OPTIONAL LIBOMP_OMPT_OPTIONAL
#if KMP_ARCH_X86 || KMP_ARCH_X86_64 // FreeBSD customization {
#define LIBOMP_USE_ADAPTIVE_LOCKS 1
#else
#define LIBOMP_USE_ADAPTIVE_LOCKS 0
#endif // } FreeBSD customization
#define KMP_USE_ADAPTIVE_LOCKS LIBOMP_USE_ADAPTIVE_LOCKS
#define KMP_DEBUG_ADAPTIVE_LOCKS 0
#define LIBOMP_USE_INTERNODE_ALIGNMENT 0
#define KMP_USE_INTERNODE_ALIGNMENT LIBOMP_USE_INTERNODE_ALIGNMENT
#define LIBOMP_ENABLE_ASSERTIONS 1
#define KMP_USE_ASSERT LIBOMP_ENABLE_ASSERTIONS
#define LIBOMP_USE_HIER_SCHED 0
#define KMP_USE_HIER_SCHED LIBOMP_USE_HIER_SCHED
#define STUBS_LIBRARY 0
#define LIBOMP_USE_HWLOC 0
#define KMP_USE_HWLOC LIBOMP_USE_HWLOC
#define LIBOMP_ENABLE_SHARED 1
#define KMP_DYNAMIC_LIB LIBOMP_ENABLE_SHARED
#if KMP_ARCH_X86 // FreeBSD customization {
#define KMP_ARCH_STR "IA-32"
#elif KMP_ARCH_X86_64
#define KMP_ARCH_STR "Intel(R) 64"
#elif KMP_ARCH_AARCH64
#define KMP_ARCH_STR "AARCH64"
#elif KMP_ARCH_PPC64
#define KMP_ARCH_STR "PPC64BE"
#elif KMP_ARCH_MIPS
#define KMP_ARCH_STR "MIPS"
#elif KMP_ARCH_MIPS64
#define KMP_ARCH_STR "MIPS64"
#elif KMP_ARCH_RISCV64
#define KMP_ARCH_STR "RISCV64"
#else
#error Unknown KMP_ARCH value
#endif // } FreeBSD customization
#define KMP_LIBRARY_FILE "libomp.so"
#define KMP_VERSION_MAJOR 5
#define KMP_VERSION_MINOR 0
#define MSVC 0
#define KMP_MSVC_COMPAT MSVC
#if KMP_ARCH_X86 || KMP_ARCH_X86_64 // FreeBSD customization {
#define LIBOMP_HAVE_WAITPKG_INTRINSICS 1
#else
#define LIBOMP_HAVE_WAITPKG_INTRINSICS 0
#endif // } FreeBSD customization
#define KMP_HAVE_WAITPKG_INTRINSICS LIBOMP_HAVE_WAITPKG_INTRINSICS
#if KMP_ARCH_X86 || KMP_ARCH_X86_64 // FreeBSD customization {
#define LIBOMP_HAVE_RTM_INTRINSICS 1
#else
#define LIBOMP_HAVE_RTM_INTRINSICS 0
#endif // } FreeBSD customization
#define KMP_HAVE_RTM_INTRINSICS LIBOMP_HAVE_RTM_INTRINSICS
#if KMP_ARCH_X86 || KMP_ARCH_X86_64 // FreeBSD customization {
#define LIBOMP_HAVE_IMMINTRIN_H 1
#else
#define LIBOMP_HAVE_IMMINTRIN_H 0
#endif // } FreeBSD customization
#define KMP_HAVE_IMMINTRIN_H LIBOMP_HAVE_IMMINTRIN_H
#define LIBOMP_HAVE_INTRIN_H 0
#define KMP_HAVE_INTRIN_H LIBOMP_HAVE_INTRIN_H
#if KMP_ARCH_X86 || KMP_ARCH_X86_64 // FreeBSD customization {
#define LIBOMP_HAVE_ATTRIBUTE_WAITPKG 1
#else
#define LIBOMP_HAVE_ATTRIBUTE_WAITPKG 0
#endif // } FreeBSD customization
#define KMP_HAVE_ATTRIBUTE_WAITPKG LIBOMP_HAVE_ATTRIBUTE_WAITPKG
#if KMP_ARCH_X86 || KMP_ARCH_X86_64 // FreeBSD customization {
#define LIBOMP_HAVE_ATTRIBUTE_RTM 1
#else
#define LIBOMP_HAVE_ATTRIBUTE_RTM 0
#endif // } FreeBSD customization
#define KMP_HAVE_ATTRIBUTE_RTM LIBOMP_HAVE_ATTRIBUTE_RTM
#define LIBOMP_ARCH_AARCH64_A64FX 0
#define KMP_ARCH_AARCH64_A64FX LIBOMP_ARCH_AARCH64_A64FX
#if KMP_ARCH_X86 || KMP_ARCH_X86_64 // FreeBSD customization {
#define LIBOMP_HAVE_XMMINTRIN_H 1
#else
#define LIBOMP_HAVE_XMMINTRIN_H 0
#endif // } FreeBSD customization
#define KMP_HAVE_XMMINTRIN_H LIBOMP_HAVE_XMMINTRIN_H
#if KMP_ARCH_X86 || KMP_ARCH_X86_64 // FreeBSD customization {
#define LIBOMP_HAVE__MM_MALLOC 1
#else
#define LIBOMP_HAVE__MM_MALLOC 0
#endif // } FreeBSD customization
#define KMP_HAVE__MM_MALLOC LIBOMP_HAVE__MM_MALLOC
#define LIBOMP_HAVE_ALIGNED_ALLOC 1
#define KMP_HAVE_ALIGNED_ALLOC LIBOMP_HAVE_ALIGNED_ALLOC
#define LIBOMP_HAVE_POSIX_MEMALIGN 1
#define KMP_HAVE_POSIX_MEMALIGN LIBOMP_HAVE_POSIX_MEMALIGN
#define LIBOMP_HAVE__ALIGNED_MALLOC 0
#define KMP_HAVE__ALIGNED_MALLOC LIBOMP_HAVE__ALIGNED_MALLOC

// Configured cache line based on architecture
#if KMP_ARCH_PPC64
# define CACHE_LINE 128
#elif KMP_ARCH_AARCH64_A64FX
# define CACHE_LINE 256
#else
# define CACHE_LINE 64
#endif

#if ! KMP_32_BIT_ARCH
# define BUILD_I8 1
#endif

#define KMP_NESTED_HOT_TEAMS 1
#define KMP_ADJUST_BLOCKTIME 1
#define BUILD_PARALLEL_ORDERED 1
#define KMP_ASM_INTRINS 1
#define USE_ITT_BUILD LIBOMP_USE_ITT_NOTIFY
#define INTEL_ITTNOTIFY_PREFIX __kmp_itt_
#if ! KMP_MIC
# define USE_LOAD_BALANCE 1
#endif
#if ! (KMP_OS_WINDOWS || KMP_OS_DARWIN)
# define KMP_TDATA_GTID 1
#endif
#if STUBS_LIBRARY
# define KMP_STUB 1
#endif
#if DEBUG_BUILD || RELWITHDEBINFO_BUILD
# define KMP_DEBUG 1
#endif

#if KMP_OS_WINDOWS
# define KMP_WIN_CDECL
#else
# define BUILD_TV
# define KMP_GOMP_COMPAT
#endif

// use shared memory with dynamic library (except Android, where shm_*
// functions don't exist).
#if KMP_OS_UNIX && KMP_DYNAMIC_LIB && !__ANDROID__
#define KMP_USE_SHM
#endif
#endif // KMP_CONFIG_H
