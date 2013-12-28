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

  #if defined(mysql_chassis_proxy_EXPORTS)
    #define MY_API __declspec(dllexport)
  #elif defined(MY_GLOBAL_COMPILE_STATIC)	 /* In test cases, define this to prevent linker warnings on Win32 */
    #define MY_API 
  #else
    #define MY_API extern __declspec(dllimport)
  #endif

#else

  #define MY_API		extern

#endif

#endif  //__MY_GLOBAL_EXPORTS_H
