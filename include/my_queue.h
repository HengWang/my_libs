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
  Copyright Monty Program KB.
  By monty.
*/

#ifndef __MY_QUEUE_H
#define __MY_QUEUE_H

#include "my_global_exports.h"

C_MODE_START

typedef struct my_queue_t {
    unsigned char** root;
    void* first_cmp_arg;
    unsigned int elements;
    unsigned int max_elements;
    unsigned int offset_to_key;	/* compare is done on element+offset */
    int max_at_top;	/* Normally 1, set to -1 if queue_top gives max */
    int (*compare)(void *, unsigned char *,unsigned char *);
    unsigned int auto_extent;
} my_queue;

#define queue_top(queue) ((queue)->root[1])
#define queue_element(queue,index) ((queue)->root[index+1])
#define queue_end(queue) ((queue)->root[(queue)->elements])
#define queue_replaced(queue) _downheap(queue,1)
#define queue_set_cmp_arg(queue, set_arg) (queue)->first_cmp_arg= set_arg
#define queue_set_max_at_top(queue, set_arg) (queue)->max_at_top= set_arg ? -1 : 1

typedef int (*queue_compare)(void*, unsigned char*, unsigned char*);

MY_GLOBAL_API int queue_init(my_queue* queue, unsigned int max_elements, unsigned int offset_to_key,
	       bool max_at_top, queue_compare compare, void* first_cmp_arg);
MY_GLOBAL_API int queue_init_ex(my_queue* queue, unsigned int max_elements, unsigned int offset_to_key,
	       bool max_at_top, queue_compare compare, void* first_cmp_arg, unsigned int auto_extent);
MY_GLOBAL_API int queue_reinit(my_queue* queue,unsigned int max_elements, unsigned int offset_to_key,
                 bool max_at_top, queue_compare compare, void* first_cmp_arg);
MY_GLOBAL_API int queue_resize(my_queue* queue, unsigned int max_elements);
MY_GLOBAL_API void queue_delete(my_queue* queue);
MY_GLOBAL_API void queue_insert(my_queue* queue, unsigned char* element);
MY_GLOBAL_API int queue_insert_safe(my_queue* queue, unsigned char* element);
MY_GLOBAL_API unsigned char* queue_remove(my_queue* queue, unsigned int idx);
#define queue_remove_all(queue) { (queue)->elements = 0; }
#define queue_is_full(queue) (queue->elements == queue->max_elements)
//MY_GLOBAL_API void _downheap(my_queue *queue,unsigned int idx);
MY_GLOBAL_API void queue_fix(my_queue *queue);
#define is_queue_inited(queue) ((queue)->root != 0)

C_MODE_START

#endif  /* __MY_QUEUE_H */
