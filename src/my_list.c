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

#include <stdlib.h>
 
#include "my_list.h"
#include "my_malloc.h"



/******************************************************************************/
MY_GLOBAL_API my_list* my_list_init()
{
    my_list* list;
 
    if(!(list = my_malloc(sizeof(my_list))))
	    return NULL;
    list->head = 0;
    list->tail = 0;
    list->length = 0;
	
    return list;
}

/******************************************************************************/
MY_GLOBAL_API void my_list_uninit(my_list* __list)
{
    my_list_iter *__next;
    my_list_iter *__current;
    
    if (!__list)
        return;
    /* Free the iterators */
    if (__list->length > 0){
        __current = __list->head;
        do {
            __next = __current->__next;
            my_free(__current);
            __current = __next;
       } while (__current);
    }
    my_free(__list);
}

/******************************************************************************/
MY_GLOBAL_API void my_list_clear(my_list* __list)
{
    __list->head = 0;
    __list->tail = 0;
    __list->length = 0;
}

/******************************************************************************/
MY_GLOBAL_API size_t my_list_get_length(my_list* __list)
{
    return (__list ? __list->length : 0);
}

/******************************************************************************/
MY_GLOBAL_API my_list_iter* my_list_begin(my_list* __list)
{
    return (__list ? __list->head : 0);
}

/******************************************************************************/
MY_GLOBAL_API my_list_iter* my_list_end(my_list* __list)
{
    return 0;
}

/******************************************************************************/
MY_GLOBAL_API my_list_iter* my_list_rbegin(my_list* __list)
{
    return (__list ? __list->tail : 0);
}

/******************************************************************************/
MY_GLOBAL_API my_list_iter* my_list_rend(my_list* __list)
{
    return 0;
}

/******************************************************************************/
MY_GLOBAL_API my_list_iter* my_list_lookup(my_list* __list, void* __data)
{
    my_list_iter* item;
 
    if (! __list) 
	    return 0; 
    for (item = __list->head; item; item = item->__next)
        if (__data == item->data)
            return item;
    
    return 0;
}

/******************************************************************************/
MY_GLOBAL_API my_list_iter* my_list_lookadd(my_list* __list, void* __data)
{
    my_list_iter* item;
 
    if (! __list) 
	    return 0; 
    if ((item = my_list_lookup(__list, __data)) != 0)
        return item;

    return my_list_add(__list, __data);
}

/******************************************************************************/
MY_GLOBAL_API my_list_iter* my_list_add(my_list* __list, void* __data)
{
    my_list_iter* item;
    
    if (! __list) 
	    return 0;    
    if ((item = my_malloc(sizeof(*item))) == 0)
        return 0;    
    item->data = __data;
    item->list = __list; 
    item->__next = __list->head;
    item->__prev = 0;
    __list->head = item;    
    if (item->__next) 
	    item->__next->__prev = item;
    if (!__list->tail) 
	    __list->tail = item;    
    __list->length++;
    
    return item;
}

/******************************************************************************/
MY_GLOBAL_API my_list_iter* my_list_sortadd(my_list* __list, my_list_func_t __func, void* __data)
{
    my_list_iter* item;
    
    if (! __list || ! __func) 
	    return 0;    
    for (item = __list->head; item; item = item->__next)
        if ((*__func)(item->data, __data) > 0)
            break;    
    if (item)
        return my_list_iter_insert(item, __data);
    else
        return my_list_append(__list, __data);
}

/******************************************************************************/
MY_GLOBAL_API int my_list_delete(my_list* __list, void* __data)
{
    my_list_iter* item;
    
    if (!__list)
        return -1;
    
    for (item = __list->head; item; item = item->__next)
        if (__data == item->data)
            break;    
    if (!item)
        return -1;    
    my_list_iter_delete(item);
    
	return 0;
}

/******************************************************************************/
MY_GLOBAL_API void my_list_foreach(my_list* __list, my_list_func_t __func, void* __userdata)
{
    my_list_iter* item;
    my_list_iter* next;
	int ret;
    
    if (!__list || !__func)
        return;    
    for (item = __list->head; item; item = next) 
	{ 
        item->__foreach = 1;
        ret = (*__func)(item->data, __userdata);
        next = item->__next;        
        if (item->__foreach == 0)
            my_list_iter_delete(item);
        else
            item->__foreach = 0;        
        if (ret) 
		    return;
    }
}

/******************************************************************************/
MY_GLOBAL_API void my_list_rforeach(my_list* __list, my_list_func_t __func, void* __userdata)
{
    my_list_iter* item;
    my_list_iter* pre;
	int ret;
    
    if (!__list || !__func)
        return;
    
    for (item = __list->tail; item; item = pre) 
	{
        item->__foreach = 1;
        ret = (*__func)(item->data, __userdata);
        pre = item->__prev;        
        if (item->__foreach == 0)
            my_list_iter_delete(item);
        else
            item->__foreach = 0;        
        if (ret) 
		    return;
    }
}

/******************************************************************************/
MY_GLOBAL_API my_list_iter* my_list_iter_next(my_list_iter* __list)
{
    return (__list ? __list->__next : 0);
}

/******************************************************************************/
MY_GLOBAL_API my_list_iter* my_list_iter_prev(my_list_iter* __list)
{
    return (__list ? __list->__prev : 0);
}

/******************************************************************************/
MY_GLOBAL_API void my_list_iter_delete(my_list_iter* __list)
{
    if (!__list)
       return;    
    if (__list->__foreach == 1) 
	{
        __list->__foreach = 0;
        return;
    }    
    if (__list->__next)
        __list->__next->__prev = __list->__prev;
    else
        __list->list->tail = __list->__prev;    
    if (__list->__prev)
        __list->__prev->__next = __list->__next;
    else
        __list->list->head = __list->__next;    
    __list->list->length--;    
    my_free(__list);
}

/******************************************************************************/
MY_GLOBAL_API my_list_iter* my_list_iter_insert(my_list_iter* __list, void* __data)
{
    my_list_iter* item;
    
    if (! __list) 
	    return 0;    
    if (__list->list->head == __list)
        return my_list_prepend(__list->list, __data);    
    if ((item = my_malloc(sizeof(*item))) == 0)
        return 0;    
    item->data = __data;
    item->list = __list->list;
    item->__prev = __list->__prev;
    item->__next = __list;    
    __list->__prev->__next = item; 
    __list->__prev = item;    
    __list->list->length++;
    
    return item;
}

/******************************************************************************/
MY_GLOBAL_API my_list_iter* my_list_prepend(my_list* __list, void* __data)
{
    my_list_iter* item;
    
    if (! __list) 
	    return 0;    
    if ((item = my_malloc(sizeof(*item))) == 0)
        return 0;    
    item->list = __list;
    item->data = __data;
    item->__prev = 0;
    item->__next = __list->head;
    __list->head = item;    
    if (item->__next)
        item->__next->__prev = item;
    else
        __list->tail = item;    
    __list->length++;
    
    return item;
}

/******************************************************************************/
MY_GLOBAL_API my_list_iter* my_list_append(my_list* __list, void* __data)
{
    my_list_iter* item;
    
    if (! __list) 
	    return 0;    
    if ((item = my_malloc(sizeof(*item))) == 0)
        return 0;
    item->list = __list;
    item->data = __data;
    item->__prev = __list->tail;
    item->__next = 0;
    __list->tail = item;    
    if (item->__prev)
        item->__prev->__next = item;
    else
        __list->head = item;    
    __list->length++;
    
    return item;
}
