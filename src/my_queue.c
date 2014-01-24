/*
 * Copyright (c) 2013, Heng Wang personal. All rights reserved.
 * 
 * 
 *
 * @Author:  Heng.Wang
 * @Date  :  12/24/2013
 * @Email :  wangheng.king@gmail.com
 *           king_wangheng@163.com
 * @Github:  https://github.com/HengWang/
 * @Blog  :  http://hengwang.blog.chinaunix.net
 * */

/*
  Code for generell handling of priority Queues.
  Implemention of queues from "Algoritms in C" by Robert Sedgewick.
  An optimisation of _downheap suggested in Exercise 7.51 in "Data
  Structures & Algorithms in C++" by Mark Allen Weiss, Second Edition
  was implemented by Mikael Ronstrom 2005. Also the O(N) algorithm
  of queue_fix was implemented.
*/

#include "my_queues.h"

/*
  Init queue

  SYNOPSIS
    queue_init()
    queue		Queue to initialise
    max_elements	Max elements that will be put in queue
    offset_to_key	Offset to key in element stored in queue
			Used when sending pointers to compare function
    max_at_top		Set to 1 if you want biggest element on top.
    compare		Compare function for elements, takes 3 arguments.
    first_cmp_arg	First argument to compare function

  NOTES
    Will allocate max_element pointers for queue array

  RETURN
    0	ok
    1	Could not allocate memory
*/

MY_GLOBAL_API int queue_init(my_queue* queue, unsigned int max_elements, unsigned int offset_to_key,
	       bool max_at_top, int (*compare)(void*, unsigned char*, unsigned char*), void* first_cmp_arg)
{
    if (!(queue->root = (unsigned char **) my_malloc((max_elements+1)*sizeof(void*))))
      return 1;
    queue->elements = 0;
    queue->compare = compare;
    queue->first_cmp_arg = first_cmp_arg;
    queue->max_elements = max_elements;
    queue->offset_to_key = offset_to_key;
    queue_set_max_at_top(queue, max_at_top);
	
    return 0;
}



/*
  Init queue, uses queue_init internally for init work but also accepts
  auto_extent as parameter

  SYNOPSIS
    queue_init_ex()
    queue		Queue to initialise
    max_elements	Max elements that will be put in queue
    offset_to_key	Offset to key in element stored in queue
			Used when sending pointers to compare function
    max_at_top		Set to 1 if you want biggest element on top.
    compare		Compare function for elements, takes 3 arguments.
    first_cmp_arg	First argument to compare function
    auto_extent         When the queue is full and there is insert operation
                        extend the queue.

  NOTES
    Will allocate max_element pointers for queue array

  RETURN
    0	ok
    1	Could not allocate memory
*/

MY_GLOBAL_API int queue_init_ex(my_queue* queue, unsigned int max_elements, unsigned int offset_to_key,
	       bool max_at_top, int (*compare) (void*, unsigned char*, unsigned char*), 
		   void* first_cmp_arg, unsigned int auto_extent)
{
    int ret;
    
    if ((ret = queue_init(queue, max_elements, offset_to_key, max_at_top, compare, first_cmp_arg)))
        return ret;    
    queue->auto_extent = auto_extent;
	
    return 0;
}

/*
  Reinitialize queue for other usage

  SYNOPSIS
    queue_reinit()
    queue		Queue to initialise
    max_elements	Max elements that will be put in queue
    offset_to_key	Offset to key in element stored in queue
			Used when sending pointers to compare function
    max_at_top		Set to 1 if you want biggest element on top.
    compare		Compare function for elements, takes 3 arguments.
    first_cmp_arg	First argument to compare function

  NOTES
    This will delete all elements from the queue.  If you don't want this,
    use queue_resize() instead.

  RETURN
    0			ok
    EE_OUTOFMEMORY	Wrong max_elements
*/

MY_GLOBAL_API int queue_reinit(my_queue* queue, unsigned int max_elements, unsigned int offset_to_key,
		 bool max_at_top, int (*compare)(void*, unsigned char*, unsigned char*), void* first_cmp_arg)
{
    queue->elements = 0;
    queue->compare = compare;
    queue->first_cmp_arg = first_cmp_arg;
    queue->offset_to_key = offset_to_key;
    queue_set_max_at_top(queue, max_at_top);
    queue_resize(queue, max_elements);
	
    return 0;
}


/*
  Resize queue

  SYNOPSIS
    queue_resize()
    queue			Queue
    max_elements		New max size for queue

  NOTES
    If you resize queue to be less than the elements you have in it,
    the extra elements will be deleted

  RETURN
    0	ok
    1	Error.  In this case the queue is unchanged
*/

MY_GLOBAL_API int queue_resize(my_queue* queue, unsigned int max_elements)
{
    unsigned char** new_root;
	
    if (queue->max_elements == max_elements)
        return 0;
    if (!(new_root= (unsigned char **) my_realloc((void *)queue->root, (max_elements+1)*sizeof(void*))))
        return 1;
    set_if_smaller(queue->elements, max_elements);
    queue->max_elements = max_elements;
    queue->root= new_root;
	
    return 0;
}


/*
  Delete queue

  SYNOPSIS
   queue_delete()
   queue		Queue to delete

  IMPLEMENTATION
    Just free allocated memory.

  NOTES
    Can be called safely multiple times
*/

MY_GLOBAL_API void queue_delete(my_queue* queue)
{
    my_free(queue->root);
    queue->root= NULL;
}


	/* Code for insert, search and delete of elements */

MY_GLOBAL_API void queue_insert(my_queue* queue, unsigned char* element)
{
    unsigned int idx, next;
	
    assert(queue->elements < queue->max_elements);
    queue->root[0] = element;
    idx = ++queue->elements;
    /* max_at_top swaps the comparison if we want to order by desc */
    while ((queue->compare(queue->first_cmp_arg,
                           element + queue->offset_to_key,
                           queue->root[(next= idx >> 1)] +
                           queue->offset_to_key) * queue->max_at_top) < 0)
    {
        queue->root[idx] = queue->root[next];
        idx = next;
    }
    queue->root[idx] = element;
}

/*
  Does safe insert. If no more space left on the queue resize it.
  Return codes:
    0 - OK
    1 - Cannot allocate more memory
    2 - auto_extend is 0, the operation would
  
*/

MY_GLOBAL_API int queue_insert_safe(my_queue* queue, unsigned char* element)
{
    if (queue->elements == queue->max_elements)
    {
        if (!queue->auto_extent)
            return 2;
        else if (queue_resize(queue, queue->max_elements + queue->auto_extent))
            return 1;
    }    
    queue_insert(queue, element);
	
    return 0;
}


	/* Remove item from queue */
	/* Returns pointer to removed element */

MY_GLOBAL_API unsigned char *queue_remove(my_queue* queue, unsigned int idx)
{
    unsigned char *element;
	
    assert(idx < queue->max_elements);
    element = queue->root[++idx];  /* Intern index starts from 1 */
    queue->root[idx] = queue->root[queue->elements--];
    _downheap(queue, idx);
	
    return element;
}

	/* Fix when element on top has been replaced */

static void _downheap(my_queue* queue, unsigned int idx)
{
    unsigned char *element;
    unsigned int elements, half_queue, offset_to_key, next_index;
    bool first = TRUE;
    unsigned int start_idx = idx;
    
    offset_to_key = queue->offset_to_key;
    element = queue->root[idx];
    half_queue = (elements = queue->elements) >> 1;    
    while(idx <= half_queue)
    {
        next_index=idx+idx;
        if(next_index < elements &&
	            (queue->compare(queue->first_cmp_arg,
	    		 queue->root[next_index]+offset_to_key,
	    		 queue->root[next_index+1]+offset_to_key) *
	             queue->max_at_top) > 0)
            next_index++;
        if(first && 
                (((queue->compare(queue->first_cmp_arg,
                 queue->root[next_index]+offset_to_key,
                 element+offset_to_key) * queue->max_at_top) >= 0)))
        {
            queue->root[idx]= element;
            return;
        }
        queue->root[idx] = queue->root[next_index];
        idx = next_index;
        first = FALSE;
    }    
    next_index = idx >> 1;
    while(next_index > start_idx)
    {
        if((queue->compare(queue->first_cmp_arg,
                           queue->root[next_index]+offset_to_key,
                           element+offset_to_key) *
                           queue->max_at_top) < 0)
            break;
        queue->root[idx]=queue->root[next_index];
        idx=next_index;
        next_index= idx >> 1;
    }
    queue->root[idx]=element;
}

/*
  Fix heap when every element was changed.
*/

MY_GLOBAL_API void queue_fix(my_queue* queue)
{
    unsigned int i;
	
    for (i= queue->elements >> 1; i > 0; i--)
        _downheap(queue, i);
}

