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

#ifndef __MY_my_hash_H
#define __MY_my_hash_H

#include "my_global_exports.h"
#include "my_array.h"

C_MODE_START

/*
  Overhead to store an element in hash
  Can be used to approximate memory consumption for a hash
 */
#define my_hash_OVERHEAD (sizeof(char*)*2)
/* flags for hash_init */
#define my_hash_UNIQUE     1       /* hash_insert fails on duplicate key */

typedef struct my_hash_t {
    size_t key_offset,key_length;		/* Length of key if const length */
    size_t blength;
    unsigned long records;
    unsigned int flags;
    my_array array;				/* Place for hash_keys */
    my_hash_get_key get_key;
    void (*free)(void *);
    my_hash_function hash_function;
} my_hash;


typedef unsigned int my_hash_value_type;
typedef unsigned char *(*my_hash_get_key)(const unsigned char*, size_t*, int);
typedef void (*my_hash_free_key)(void *);
/**
  Function type representing a hash function to be used with the my_hash
  container.
  Should accept pointer to my_hash, pointer to key buffer and key length
  as parameters.
*/
typedef my_hash_value_type (*my_hash_function)(const struct my_hash*, const unsigned char *, size_t);
		  
MY_GLOBAL_API int my_hash_init(my_hash *hash, unsigned int growth_size, 
                      my_hash_function hash_function,
                      unsigned long default_array_elements, size_t key_offset,
                      size_t key_length, my_hash_get_key get_key,
                      void (*free_element)(void*),
                      unsigned int flags);
MY_GLOBAL_API void my_hash_free(my_hash *tree);
MY_GLOBAL_API void my_hash_reset(my_hash *hash);
MY_GLOBAL_API unsigned char *my_hash_element(my_hash *hash, unsigned long idx);
MY_GLOBAL_API unsigned char *my_hash_search(const my_hash *info, const unsigned char *key, size_t length);
MY_GLOBAL_API unsigned char *my_hash_search_using_hash_value(const my_hash *info,
                                       my_hash_value_type hash_value,
                                       const unsigned char *key, size_t length);
MY_GLOBAL_API my_hash_value_type my_hash_calc(const my_hash *info,
                                const unsigned char *key, size_t length);
MY_GLOBAL_API unsigned char *my_hash_first(const my_hash *info, const unsigned char *key, size_t length,
                     unsigned int *state);
MY_GLOBAL_API unsigned char *my_hash_first_from_hash_value(const my_hash *info,
                                     my_hash_value_type hash_value,
                                     const unsigned char *key,
                                     size_t length,
                                     unsigned int *state);
MY_GLOBAL_API unsigned char *my_hash_next(const my_hash *info, const unsigned char *key, size_t length,
                    unsigned int *state);
MY_GLOBAL_API int my_hash_insert(my_hash *info, const unsigned char *data);
MY_GLOBAL_API int my_hash_delete(my_hash *hash, unsigned char *record);
MY_GLOBAL_API int my_hash_update(my_hash *hash, unsigned char *record, unsigned char *old_key, size_t old_key_length);
MY_GLOBAL_API void my_hash_replace(my_hash *hash, unsigned int *state, unsigned char *new_row);
#define my_hash_clear(H) memset((H), 0, sizeof(*(H)))

C_MODE_END

#endif
