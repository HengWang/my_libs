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

#include "my_global_exports.h"
#include "my_array.h"
#include "my_malloc.h"

/*
  Initiate dynamic array

  SYNOPSIS
    init_my_array2()
      array		Pointer to an array
      element_size	Size of element
      init_buffer       Initial buffer pointer
      init_alloc	Number of initial elements
      alloc_increment	Increment for adding new elements

  DESCRIPTION
    init_my_array() initiates array and allocate space for 
    init_alloc eilements. 
    Array is usable even if space allocation failed, hence, the
    function never returns TRUE.
    Static buffers must begin immediately after the array structure.

  RETURN VALUE
    FALSE	Ok
*/

MY_GLOBAL_API my_array* my_array_init(void*__buffer, unsigned __number, 
                                        unsigned int __increment, unsigned int __size)
{
    my_array* array;
	if(!(array = my_calloc(sizeof(my_array))))
	    return NULL;
    if(!__increment)
	{
	    __increment = MAX((ARRAT_BLOCK_SIZE - MALLOC_OVERHEAD)/__size, ARRAY_INIT_INCREMENT);
		if(__number > ARRAY_INIT_NUMBER && __increment > __number * 2)
		    __increment = __number * 2
	}
	if(!__number)
	{
	    __number = __increment;
		__buffer = NULL;
	}
	array->elements = 0;
	array->number = __number;
	array->increment = __increment;
	array->size = __size;
	if(array->buffer = __buffer)
	    return array;
	if (!(array->buffer = (void*)my_malloc(__size * __number)))
    {    
		array->__number=0;
	    return NULL;
	}
	return array;
}


/*
  Empty array by freeing all memory

  SYNOPSIS
    delete_dynamic()
      array	Array to be deleted
*/
MY_GLOBAL_API void my_array_uninit(my_array* __array)
{
    if(__array->buffer == (unsigned char*)(__array + 1))
	{
	    __array->elements = 0;
	} else {
	    if(__array->buffer)
		{
		    my_free(__array->buffer);
			__array->buffer = NULL;
			__array->elements = 0;
			__array->number = 0;
		}
	}
}


/*
  Insert element at the end of array. Allocate memory if needed.

  SYNOPSIS
    insert_dynamic()
      array
      element

  RETURN VALUE
    TRUE	Insert failed
    FALSE	Ok
*/
MY_GLOBAL_API int my_array_insert(my_array* __array, const void* __element)
{
    unsigned char* buffer;
	if(__array->elements == __array->number)
	{
	    if(!(buffer = my_array_alloc(__array)))
		    return 1;
	} else {
	    buffer = __array->buffer+(__array->elements * __array->size);
		__array->elements++;
	}
	memcpy(buffer, __element, (size_t)array->size);
	return 0;
}

/*
  Alloc space for next element(s) 

  SYNOPSIS
    alloc_dynamic()
      array

  DESCRIPTION
    alloc_dynamic() checks if there is empty space for at least
    one element if not tries to allocate space for alloc_increment
    elements at the end of array.

  RETURN VALUE
    pointer	Pointer to empty space for element
    0		Error
*/

MY_GLOBAL_API void* my_array_alloc(my_array* __array)
{
    char* buffer;
    if(__array->elements == __array->number)
	{
	    if(__array->buffer == (unsigned char*)(__array + 1))
		{
		    if(!(buffer = (char*) my_malloc((__array->number + __array->increment) * __array->size)))
			    return NULL;
 		    memcpy(buffer, __array->buffer, __array->elements * __array->size);
		} else {
		    if (!(buffer = (char*) my_realloc(__array->buffer, (__array->number + __array->increment) * __array->size)))
                return NULL;
		}
		__array->buffer= (unsigned char*) buffer;
        __array->number += __array->increment;
	}
	return __array->buffer + (__array->elements++ * __array->size);
}

/*
  Pop last element from array.

  SYNOPSIS
    pop_dynamic()
      array
  
  RETURN VALUE    
    pointer	Ok
    0		Array is empty
*/
MY_GLOBAL_API void* my_array_pop(my_array* __array)
{
    if(__array->elements)
        return __array->buffer + (--__array->elements * __array->size);
	return NULL;
}

/*
  Replace element in array with given element and index

  SYNOPSIS
    set_dynamic()
      array
      element	Element to be inserted
      idx	Index where element is to be inserted

  DESCRIPTION
    set_dynamic() replaces element in array. 
    If idx > max_element insert new element. Allocate memory if needed. 
 
  RETURN VALUE
    TRUE	Idx was out of range and allocation of new memory failed
    FALSE	Ok
*/
MY_GLOBAL_API int my_array_set(my_array* __array, const void* __element, unsigned int __idx)
{
    if(__idx >= __array->elements)
    {
	    if(__idx >= __array->number && my_array_allocate(__array, __idx))
		{
		    return 1;
		}
		memset((__array->buffer + __array->elements * __array->size), 0, (__idx - __array->elements) * __array->size);
		array->elements = __idx + 1;
    }
	memcpy(__array->buffer + (__idx * __array->size), __element, (size_t) __array->size);
	return 0;
}


/*
  Ensure that dynamic array has enough elements

  SYNOPSIS
    allocate_dynamic()
    array
    max_elements        Numbers of elements that is needed

  NOTES
   Any new allocated element are NOT initialized

  RETURN VALUE
    FALSE	Ok
    TRUE	Allocation of new memory failed
*/

static int my_array_allocate(my_array* __array, unsigned int __number)
{
    unsigned int size;
	unsigned char* buffer;
    if (__number >= __array->number)
    {
	    size = (__number + __array->increment)/__array->increment;
        size *= __array->increment;
        if (__array->buffer == (unsigned char*)(__array + 1))
        {    
            if(!(buffer= (unsigned char*) my_malloc(size * __array->size)))
                return 1;
            memcpy(buffer, __array->buffer, __array->elements * __array->size);
        } else{		
		    if (!(buffer = (unsigned char*) my_realloc(__array->buffer, size * __array->size)))
                return 1;
		} 
		__array->buffer = buffer;
        __array->number = size;
    }
    return 0;
}


/*
  Get an element from array by given index

  SYNOPSIS
    get_dynamic()
      array	
      uchar*	Element to be returned. If idx > elements contain zeroes.
      idx	Index of element wanted. 
*/
MY_GLOBAL_API int my_array_get(my_array* __array, void* __element, unsigned int __idx)
{
    if(__idx >= __array->elements)
	{
	    memset(__element, 0, __array->size);
		return 1;
	}
	memcpy(__element, __array->buffer + __idx * __array->size, (size_t) __array->size);
	return 0;
}


/*
  Delete element by given index

  SYNOPSIS
    delete_dynamic_element()
      array
      idx        Index of element to be deleted
*/
MY_GLOBAL_API void my_array_delete(my_array* __array, unsigned int __idx)
{
    char* ptr = (char*)__array->buffer + __array->size * __idx;
	__array->elements--;
	memmove(ptr, ptr + __array->size, (__array->elements - idx) * __array->size);
}


/*
  Free unused memory

  SYNOPSIS
    freeze_size()
      array	Array to be freed

*/
MY_GLOBAL_API void my_array_free(my_array* __array)
{
    unsigned int elements = MAX(__array->elements, 1);
	if(__array->buffer == (unsigned char*)(__array + 1))
	    return;
	if(__array->buffer && __array->number != elements)
	{
	    __array->buffer = (unsigned char*) my_realloc(__array->buffer, elements * __array->size);
		__array->number = elements;
	}
}

