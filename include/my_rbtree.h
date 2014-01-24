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

#ifndef __MY_RBTREE_H
#define __MY_RBTREE_H

#include "my_global_exports.h"

C_MODE_START

/* Worst case tree is half full. This gives use 2^(MAX_rbtree_HEIGHT/2) leafs */
#define MAX_rbtree_HEIGHT	64

#define rbtree_NO_DUPLICATES 1

#define ELEMENT_KEY(tree,element)\
(tree->offset ? (void*)((uchar*) element+tree->offset) :\
			*((void**) (element+1)))

#define tree_set_pointer(element,ptr) *((uchar **) (element+1))=((uchar*) (ptr))

#define ELEMENT_CHILD(element, offs) (*(rbtree_element**)((char*)element + offs))

#define rbtree_element_EXTRA_SIZE (sizeof(rbtree_element) + sizeof(void*))

  /* similar to delete tree, except we do not my_free() blocks in mem_root
   */
#define is_tree_inited(tree) ((tree)->root != 0)

typedef enum { LEFT_ROOT_RIGHT, RIGHT_ROOT_LEFT } rbtree_walk_type;

typedef int (*rbtree_walk_action)(void* ,unsigned long,void* );

typedef enum { FREE_INIT, FREE_FREE, FREE_UNINIT } rbtree_free_stat;

typedef void (*rbtree_element_free)(void*, rbtree_free_stat, const void* );

typedef enum {
  HA_READ_KEY_EXACT,              /* Find first record else error */
  HA_READ_KEY_OR_NEXT,            /* Record or next record */
  HA_READ_KEY_OR_PREV,            /* Record or previous */
  HA_READ_AFTER_KEY,              /* Find next rec. after key-record */
  HA_READ_BEFORE_KEY,             /* Find next rec. before key-record */
  HA_READ_PREFIX,                 /* Key which as same prefix */
  HA_READ_PREFIX_LAST,            /* Last key with the same prefix */
  HA_READ_PREFIX_LAST_OR_PREV,    /* Last or prev key with the same prefix */
  HA_READ_MBR_CONTAIN,            /* Minimum Bounding Rectangle contains */
  HA_READ_MBR_INTERSECT,          /* Minimum Bounding Rectangle intersect */
  HA_READ_MBR_WITHIN,             /* Minimum Bounding Rectangle within */
  HA_READ_MBR_DISJOINT,           /* Minimum Bounding Rectangle disjoint */
  HA_READ_MBR_EQUAL               /* Minimum Bounding Rectangle equal */
} ha_read_func;

typedef struct rbtree_element_t {
    struct rbtree_element_t* left;
    struct rbtree_element_t* right;
    unsigned long count:31;
    unsigned long colour:1;			/* black is marked as 1 */
} rbtree_element;

typedef struct rbtree_t {
    rbtree_element* root;
    rbtree_element null_element;
    rbtree_element** parents[MAX_rbtree_HEIGHT];
    unsigned int offset;
    unsigned int elements;
    unsigned int size;
    unsigned long memory_limit;
    unsigned long allocated;
    my_qsort_cmp compare;
    const void* context;
    MEM_ROOT mem_root;
    bool is_delete;
    rbtree_element_free free;
    unsigned int flag;
} rbtree;

	/* Functions on whole tree */
MY_GLOBAL_API int rbtree_init(rbtree* tree, unsigned long default_alloc_size, unsigned long memory_limit,
                        int size, qsort_cmp2 compare, bool is_delete,
	                    rbtree_element_free free_element, const void* context);
						
MY_GLOBAL_API void rbtree_uninit(rbtree*);

MY_GLOBAL_API void rbtree_reset(rbtree*);

	/* Functions on leafs */
MY_GLOBAL_API rbtree_element *rbtree_insert(rbtree* tree,void* key, unsigned int key_size, const void* context);

MY_GLOBAL_API int rbtree_delete(rbtree* tree, void* key, unsigned int key_size, const void* context);

MY_GLOBAL_API void* rbtree_search(rbtree* tree, void* key, const void* context);

MY_GLOBAL_API int rbtree_walk(rbtree* tree, rbtree_walk_action action, void* argument, rbtree_walk_type visit);

MY_GLOBAL_API void* rbtree_search_key(rbtree* tree, const void* key, rbtree_element** parents, 
                        rbtree_element*** last_pos, ha_read_func flag, const void* context);
						
MY_GLOBAL_API void* rbtree_search_edge(rbtree* tree, rbtree_element** parents, rbtree_element*** last_pos, int child_offs);

MY_GLOBAL_API void* rbtree_search_next(rbtree* tree, rbtree_element*** last_pos, int l_offs, int r_offs);

MY_GLOBAL_API unsigned long long rbtree_record_pos(rbtree* tree, const void* key, ha_read_func flag, const void* context);

C_MODE_END

#endif /* __MY_RBTREE_H */
