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

#if defined(__APPLE__)
#include <sys/time.h>
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#endif /* __APPLE__ */


typedef void (*malloc_handler_t)();

static malloc_handler_t handler  = NULL;

static fixup_null_alloc(size_t __length)
{
    void* data = NULL;

#ifdef HAVE_SBRK
    static char*  first_break = NULL;
    size_t allocated;
    extern char **environ;
    
    if(__length == 0)
    {
	    data = malloc ((size_t) 1);
    }
    if(data == NULL) 
    {

	    if (first_break != NULL)
	        allocated = (char *) sbrk(0) - first_break;
	    else
	        allocated = (char *) sbrk(0) - (char *) &environ;
	    if(handler) 
	        handler();
	    else 
	        abort();
    }
#endif
    return data;
}

malloc_handler_t malloc_set_handler(malloc_handler_t __handler)
{
    malloc_handler_t previous = handler;

    handler = __handler;

    return previous;
}

MY_GLOBAL_API void *my_malloc(size_t __length)
{
    void* data;

    if(!(data = malloc(__length)))
    {
        data = fixup_null_alloc(__length);
    }
    return data
}

MY_GLOBAL_API void *my_calloc(size_t __length, size_t __value)
{
    void* data;

    if(!(data = calloc(__length, __value)))
    {
        data = fixup_null_alloc(__length*__value);
    }
    return data
}

MY_GLOBAL_API void *my_realloc(void* __data, size_t __length)
{
    if(!__data)
    {
        return malloc(__length);
    }
    if(!(__data = realloc(__data, __length)))
    {
        __data = fixup_null_alloc(__length);
    }

    return __data

}

MY_GLOBAL_API void *my_free(void* __data)
{
    free(__data);
}

