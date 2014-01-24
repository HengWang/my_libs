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
 
#ifndef __MY_MALLOC_H
#define __MY_MALLOC_H

#include "my_global_exports.h"

C_MODE_START

MY_GLOBAL_API void *my_malloc(size_t __length);  
MY_GLOBAL_API void *my_calloc(size_t __length, size_t __value);  
MY_GLOBAL_API void *my_realloc(void* __data, size_t __length); 
MY_GLOBAL_API void *my_free(void* __data);

C_MODE_END

#endif  //__MY_MALLOC_H
