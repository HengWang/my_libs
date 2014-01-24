/*
 * Copyright (c) 2013, Heng Wang personal. All rights reserved.
 * 
 * Generic list object. It is implemented as an array of doubly-linked
 * lists of iterators. 
 *
 * @Author:  Heng.Wang
 * @Date  :  12/24/2013
 * @Email :  wangheng.king@gmail.com
 *           king_wangheng@163.com
 * @Github:  https://github.com/HengWang/
 * @Blog  :  http://hengwang.blog.chinaunix.net
 * */

#ifndef __MY_ARRAY_H
#define __MY_ARRAY_H

#include "my_global_exports.h"

C_MODE_START

#define MALLOC_OVERHEAD      8
#define ARRAT_BLOCK_SIZE     8096
#define ARRAY_INIT_INCREMENT 16
#define ARRAY_INIT_NUMBER    8

struct my_array_t
{
  unsigned void* buffer;  /**/
  unsigned int elements;  /**/
  unsigned int number;    /**/
  unsigned int increment; /**/
  unsigned int size;      /**/
};

typedef struct my_array_t my_array;

#define my_array_reset(array) ((array)->elements= 0
#define my_array_element(array, index, type) ((type)((array)->buffer) + (index))

MY_GLOBAL_API my_array* my_array_init(void*__buffer, unsigned __number, 
                                        unsigned int __increment, unsigned int __size);

MY_GLOBAL_API void my_array_uninit(my_array* __array);

MY_GLOBAL_API void* my_array_alloc(my_array* __array);

MY_GLOBAL_API void my_array_insert(my_array* __array);

MY_GLOBAL_API void* my_array_pop(my_array* __array);

MY_GLOBAL_API void my_array_delete(my_array* __array, unsigned int __idx);

MY_GLOBAL_API int my_array_get(my_array* __array, void* __element, unsigned int __idx);

MY_GLOBAL_API int my_array_set(my_array* __array, const void* __element, unsigned int __idx);

MY_GLOBAL_API void my_array_free(my_array* __array);	
								
C_MODE_END

#endif //__MY_ARRAY_H
