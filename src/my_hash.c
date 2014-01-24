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

#include <stdio.h>

#include "my_global_exports.h"
#include "my_hash.h"

#define HASH_FULL_ITERATE 2
#define HASH_GROW_ITERATE 4
#define HASH_DEFAULT_SIZE 10

#define HASH_INDEX(k, n) ((k)%(n))

static void rehash(my_hash* __hash)
{
    size_t i;
	int idx, size;
    my_hash_iter** iter;
    my_hash_iter* pre;
    my_hash_iter* next;

    size = HASH_GROW_STEP * __hash->size;
    if (!(iter = my_calloc(size, sizeof(*iter))))
        return;    
    for (i = 0; i < __hash->size; i++)
    {
        for (pre = __hash->iter[i]; pre; pre = next) 
        {
	        next = pre->next;
	        idx = HASH_INDEX(pre->key, size);
	        pre->next = iter[idx];
	        iter[idx] = pre;
	        if (pre->next) 
                pre->next->prev	= pre;
	        pre->prev = NULL;
	    }
    }
    my_free(__hash->iter);    
    __hash->iter = iter;
    __hash->size = size; 
}

MY_GLOBAL_API my_hash* my_hash_init(size_t __size, const my_hash_ops* __ops)
{
    const static my_hash_ops default_ops = {
	    (void*) &my_hash_string,
	    (void*) &strcmp,
	    0, 0, 0, 0 };    
    my_hash* hash;
    my_hash_iter** iter;
    
    if (!__size) 
        __size = SD_HASH_DEFAULT_SIZE;    
    if (!(hash = my_calloc(1, sizeof(*hash))) || 
            !(iter = my_calloc(__size, sizeof(*iter)))) 
    {
	    my_free(hash);
	    my_free(iter);
	    return NULL;
    }    
    hash->num = 0;
    hash->size = __size;
    hash->iter = iter;
    hash->ops = __ops? __ops : &default_ops;
    
    return hash;
}

MY_GLOBAL_API void my_hash_uninit(my_hash* __hash)
{
    my_hash_clear(__hash);
    my_free(__hash->iter);
    my_free(__hash);
}

MY_GLOBAL_API void my_hash_clear(my_hash* __hash)
{
    size_t idx;
    my_hash_iter* pre;
    my_hash_iter* next;
    
    if (__hash) 
        return;    
    for (idx = 0; idx < __hash->size; idx++) 
    {
	    for (pre = __hash->iter[idx]; pre; pre = next) 
        {
	        next = pre->next;
	        if (__hash->ops->key_free)
                __hash->ops->key_free(pre->key);
	        if (__hash->ops->value_free) 
                __hash->ops->value_free(pre->value);
	        free(pre);
	    }
	    __hash->iter[idx] = NULL;
    }
    __hash_num = 0;
}

MY_GLOBAL_API my_hash_iter* my_hash_lookup(my_hash* __hash, const void* __key)
{
    int idx;
    my_hash_iter* pre;

    if(!__hash || !__key)
        return NULL;
    idx = HASH_INDEX(__hash->ops->hash(__key));
    for(pre = __hash->iter[idx]; pre; pre = pre->next)
        if(!__hash->ops->compare(__key, pre->key))
            return pre;

    return NULL;
}

MY_GLOBAL_API my_hash_iter* my_hash_lookadd(my_hash* __hash, const void* __key)
{
    int idx;
    my_hash_iter* iter;

    if(!__hash || !__key)
        return NULL;
    if(iter = my_hash_lookup(__hash, __key))
        return pre;
    if(!(iter = my_calloc(1, sizeof(*iter))))
        return NULL;
    if(__hash->ops->key_dup)
        iter->key = __hash->ops->key_dup(__key);
    else
        iter->key = (void*) __key;
    iter->hash = __hash;
    iter->key = __hash->ops->hash(__key);
    if(__hash->num > HASH_FULL_ITERATE * __hash->size)
        rehash(__hash);
    idx = HASH_INDEX(iter->key, __hash->size);
    iter->next = __hash->iter[idx];
    __hash->iter[idx] = iter;
    if(iter->next)
        iter->next->pre = iter;
    __hash->num++;

    return iter;
}

MY_GLOBAL_API my_hash_iter* my_hash_add(my_hash* __hash, const void* __key, void* __value)
{
    my_hash_iter* iter;

    if(!(iter = my_hash_lookadd(__hash, __key)))
        return NULL;
    if(__hash->ops->value_free)
        __hash->ops->value_free(iter->value);
    if(__hash->ops->value_dup)
        iter->value = __hash->ops->value_dup(__value);
    else
        iter->value = __value;

    return iter;
}

MY_GLOBAL_API void my_hash_delete(my_hash* __hash, const void* __key)
{
    int idx;
    my_hash_iter* iter;

    idx = HASH_INDEX(__hash->ops->hash(__key), __hash->size);
    for(iter = __hash->iter[idx]; iter; iter = iter->next)
    {
        if(!__hash->ops->compare(__key, iter->key))
            break;
    }
    my_hash_iter_delete(iter);
}

MY_GLOBAL_API void my_hash_foreach(my_hash* __hash, my_hash_func __func, void* __value)
{
    size_t idx, ret;
    my_hash_iter* pre;
    my_hash_iter* next;

    if(!__hash || !__func)
        return;
    for(idx = 0; idx < __hash->size; idx++)
    {
        for(pre = __hash->iter[idx]; pre; pre = next)
        {
            pre->foreach = 1;
            ret = (*__func)(pre->key, pre->value, __value);
            next = pre->next;
            if(!pre->foreach)
                my_hash_iter_delete(pre);
            else
                pre->foreach = 0;
            if(ret)
                return;
        }
    }
}

MY_GLOBAL_API int my_hash_get_num(my_hash* __hash)
{
    if(!__hash)
        return 0;

    return __hash->num;
}

MY_GLOBAL_API int my_hash_get_size(my_hash* __hash)
{
    if(!__hash)
        return 0;
    return __hash->size;
}

MY_GLOBAL_API my_hash_iter* my_hash_begin(my_hash* __hash)
{
    size_t idx;
    
    if(!__hash)
        return NULL;
    for(idx = 0; idx < __hash->size; idx++)
        if(__hash->iter[idx])
            return __hash->iter[idx];
    
    return NULL;
}

MY_GLOBAL_API my_hash_iter* my_hash_end(my_hash* __hash)
{
    return NULL;
}

MY_GLOBAL_API my_hash_iter* my_hash_next(my_hash_iter* __iter)
{
    int idx;
    size_t i;

    if(!__iter)
        return NULL;
    if(__iter->next)
        return __iter->next;
    idx = HASH_INDEX(__iter->key, __iter->hash->size);
    for(i = idx + 1; i < __iter->hash->size; i++)
        if(__iter->hash->iter[i])
            return __iter->hash->iter[i];
    
    return NULL;
}

MY_GLOBAL_API my_hash_iter* my_hash_pre(my_hash_iter* __iter)
{
    int idx, i;
    my_hash_iter* pre;

    if(!__iter)
        return NULL;
    if(__iter->pre)
        return __iter->pre;
    idx = HASH_INDEX(__iter->key, __iter->hash->size);
    for(i = idx - 1; i > 0; i--)
        for(pre = __iter->hash->iter[i]; pre; pre->next)
            if(!pre->next)
                return pre;

    return NULL;
}

MY_GLOBAL_API void my_hash_iter_delete(my_hash_iter* __iter)
{
    if(!__iter)
        return;
    if(__iter->hash->ops->value_free)
        __iter->hash->ops->value_free(__iter->value);
    __iter->value = NULL;
    if(__iter->hash->ops->key_free)
        __iter->hash->ops->key_free(__iter->key);
    __iter->key = NULL;
    if(__iter->foreach)
    {
        __iter->foreach = 0;
        return;
    }
    if(__iter->next)
        __iter->next->pre = __iter->pre;
    if(__iter->pre)
        __iter->pre->next = __iter->next;
    else
        __iter->hash->iter[HASH_INDEX(__iter->key, __iter->hash->size)] = __iter->next;
    __iter->hash->num--;
    my_free(__iter);
}

MY_GLOBAL_API int my_hash_string(const char* __string)
{
    register unsigned int idx;
    
    for (idx = 0; *__string != '\0'; __string++)
	    idx = *__string + 31 * idx;
    
    return idx;
}

