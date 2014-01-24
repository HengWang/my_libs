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

#ifndef __MY_LIST_H
#define __MY_LIST_H

#include "my_global_exports.h"

C_MODE_START

/**
 * This is the list object.
 */
struct my_list_t {
  my_list_iter* head;
  my_list_iter* tail;
  size_t length;
};

typedef struct my_list_t my_list;

struct my_list_iter_t {
  void* data;
  struct my_list* list;
  struct my_list_iter* next;
  struct my_list_iter* prev;
  int index;
};

/**
 * This is the elementary container for storing data into the list object.
 */
typedef struct my_list_iter_t my_list_iter;

/**
 * Signature of a "foreach" function.
 */
typedef unsigned int (*my_list_func_t)(void* __data, void* __userdata);

/**
 * Creates a list.
 * @param __capacity initial number of preallocated iterators
 * @return the list object.
 */
MY_GLOBAL_API my_list* my_list_init();

/**
 * Destroys the list object.
 * @todo need a function parameter to destroy list elements.
 */
MY_GLOBAL_API void my_list_uninit(my_list* __list);

/**
 * Adds the given_data at the head of the list.
 */
MY_GLOBAL_API my_list_iter* my_list_prepend(my_list* __list, void* __data);

/**
 * Adds the given data at the tail of the list.
 */
MY_GLOBAL_API my_list_iter* my_list_append(my_list* __list, void* __data);

/**
 * Looks for the iterator associated to the given data in the list object.
 * @param __data the data to find
 * @return a pointer to the found iterator or NULL.
 */
MY_GLOBAL_API my_list_iter* my_list_lookup(my_list* __list, void* __data);

/**
 * Looks for the iterator associated to the given data in the list object and
 * creates it if doesn't exist, using @c my_list_add().
 * @param __data the data to find/add
 * @return a pointer to the found iterator or NULL.
 */
MY_GLOBAL_API my_list_iter* my_list_lookadd(my_list* __list, void* __data);

/**
 * Adds the given data into the list object. If the data already exists,
 * the associated iterator is returned.
 * @warning the element is added at the begining of the list.
 * @param __data the data to add
 * @return a pointer to the created or found iterator.
 */
MY_GLOBAL_API my_list_iter* my_list_add(my_list* __list, void* __data);

/**
 * Applies the given function to all list elements, starting from the
 * first one. As soon as the function returns a non-null value, the
 * given data is inserted in the list (before the element).
 * @param __func the "sort" function.
 * @param __data the data to add
 * @return a pointer to the created iterator.
 */
MY_GLOBAL_API my_list_iter* my_list_sortadd(my_list* __list, my_list_func_t __func, void* __data);

/**
 * Removes an iterator from the list object.
 * @param __data the data associated to the iterator.
 */
MY_GLOBAL_API int my_list_delete(my_list* __list, void* __data);

/**
 * clears the list object.
 */
MY_GLOBAL_API void my_list_clear(my_list* __list);

/**
 * Calls \a __func for each element of the list object, as long as \a __func
 * returns 0.
 * @param __func the "foreach" function.
 * @param __data the user data passed to \a __func.
 */
MY_GLOBAL_API void my_list_foreach(my_list* __list, my_list_func_t __func, void* __userdata);

/**
 * Calls \a __func for each element of the list object, as long as \a __func
 * returns 0.
 * Same as my_list_foreach but from tail to head of list.
 * @param __func the "foreach" function.
 * @param __data the user data passed to \a __func.
 */
MY_GLOBAL_API void my_list_rforeach(my_list* __list, my_list_func_t __func,
         void* __userdata);

/**
 * Gets the number of iterators.
 */
MY_GLOBAL_API size_t my_list_get_nelem(my_list* __list);

/**
 * Gets the iterator pointing to the first element of the list.
 */
MY_GLOBAL_API my_list_iter* my_list_begin(my_list* __list);

/**
 * Gets the past-the-last-element iterator of the list.
 */
MY_GLOBAL_API my_list_iter* my_list_end(my_list* __list);

/**
 * Gets the iterator pointing to the last element of the list.
 */
MY_GLOBAL_API my_list_iter* my_list_rbegin(my_list* __list);

/**
 * Gets the before-the-first-element iterator of the list.
 */
MY_GLOBAL_API my_list_iter* my_list_rend(my_list* __list);

/**
 * Gets a pointer to the next iterator.
 */
MY_GLOBAL_API my_list_iter* my_list_iter_next(my_list_iter* __list);

/**
 * Gets a pointer to the previous iterator.
 */
MY_GLOBAL_API my_list_iter* my_list_iter_prev(my_list_iter* __list);

/**
 * Deletes the iterator from the list.
 */
MY_GLOBAL_API void my_list_iter_delete(my_list_iter* __list);

/**
 * Creates a new iterator and inserts it before @a __list.
 * @param __data the data associated to the iterator.
 */
MY_GLOBAL_API my_list_iter* my_list_iter_insert(my_list_iter* __list, void* __data);

C_MODE_END

#endif //__MY_LIST_H
