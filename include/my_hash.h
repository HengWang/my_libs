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

#ifndef __MY_HASH_H
#define __MY_HASH_H

#include "my_global_exports.h"

C_MODE_START

struct my_hash_ops_t {
    unsigned int (*hash)(const void*);
    int	(*compare)(const void*, const void*);
    void* (*key_dup)(const void*);
    void (*key_free)(void*);
    void* (*value_dup)(const void*);
    void (*value_free)(void*);
};

typedef my_hash_ops_t my_hash_ops;

struct my_hash_iter_t{
    void* key;
    void* value;
    struct my_hash*	hash;
    unsigned int key;
    struct my_hash_iter_t*	next;
    struct my_hash_iter_t*	prev;
    int	foreach;
};

typedef my_hash_iter_t my_hash_iter;

struct my_hash_t{
    size_t num;
    size_t size;
    my_hash_iter** iter;
    const my_hash_ops* ops;
};

typedef my_hash_t my_hash;

typedef unsigned int (*my_hash_func)(void* __key, void* __value, void* __data);

MY_GLOBAL_API my_hash* my_hash_init(size_t __size, const my_hash_ops* __ops);

MY_GLOBAL_API void my_hash_uninit(my_hash* __hash);

MY_GLOBAL_API void my_hash_clear(my_hash* __hash);

MY_GLOBAL_API my_hash_iter* my_hash_lookup(my_hash* __hash, const void* __key);

MY_GLOBAL_API my_hash_iter* my_hash_lookadd(my_hash* __hash, const void* __key);

MY_GLOBAL_API my_hash_iter* my_hash_add(my_hash* __hash, const void* __key, void* __value);

MY_GLOBAL_API void my_hash_delete(my_hash* __hash, const void* __key);

MY_GLOBAL_API void my_hash_foreach(my_hash* __hash, my_hash_func __func, void* __value);

MY_GLOBAL_API int my_hash_get_num(my_hash* __hash);

MY_GLOBAL_API int my_hash_get_size(my_hash* __hash);

MY_GLOBAL_API my_hash_iter* my_hash_begin(my_hash* __hash);

MY_GLOBAL_API my_hash_iter* my_hash_end(my_hash* __hash);

MY_GLOBAL_API my_hash_iter* my_hash_next(my_hash_iter* __iter);

MY_GLOBAL_API my_hash_iter* my_hash_pre(my_hash_iter* __iter);

MY_GLOBAL_API void my_hash_iter_delete(my_hash_iter*);

MY_GLOBAL_API int my_hash_string(const char* __string);

C_MODE_END

#endif //__MY_HASH_H


