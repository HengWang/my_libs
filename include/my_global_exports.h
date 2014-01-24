/*
 * Copyright (c) 2013, Heng Wang personal. All rights reserved.
 * 
 * Global exports header file.
 *
 * @Author:  Heng.Wang
 * @Date  :  12/24/2013
 * @Email :  wangheng.king@gmail.com
 *           king_wangheng@163.com
 * @Github:  https://github.com/HengWang/
 * @Blog  :  http://hengwang.blog.chinaunix.net
 * */

#ifndef __MY_GLOBAL_EXPORTS_H
#define __MY_GLOBAL_EXPORTS_H

#if defined(_WIN32)

  #if defined(MY_GLOBAL_EXPORTS)
    #define MY_GLOBAL_API __declspec(dllexport)
  #elif defined(MY_GLOBAL_COMPILE_STATIC)	 /* In test cases, define this to prevent linker warnings on Win32 */
    #define MY_GLOBAL_API 
  #else
    #define MY_GLOBAL_API extern __declspec(dllimport)
  #endif

#else

  #define MY_GLOBAL_API		extern

#endif

/* Macros to make switching between C and C++ mode easier.*/
#ifdef __cplusplus
#define C_MODE_START    extern "C" {
#define C_MODE_END	}
#else
#define C_MODE_START
#define C_MODE_END
#endif

/* Define boolean logical constants */
#ifndef HAS_BOOLEAN 
typedef char bool
#endif

#ifndef TRUE
#define TRUE		(1)	/* Logical true */
#define FALSE		(0)	/* Logical false */
#endif

#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define MIN(a, b)	((a) < (b) ? (a) : (b))

#define MY_ALIGN(A, L)	(((A) + (L) - 1) & ~((L) - 1))

#endif  //__MY_GLOBAL_EXPORTS_H
