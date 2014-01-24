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

#include "my_hash.h"

#define NO_RECORD	((unsigned int) -1)
#define LOWFIND 1
#define LOWUSED 2
#define HIGHFIND 4
#define HIGHUSED 8

typedef struct my_hash_link_t {
    unsigned int next;					/* index to next key */
    unsigned char* data;					/* data for current entry */
} my_hash_link;

static unsigned int my_hash_mask(my_hash_value_type hashnr, size_t buffmax, size_t maxlength);
static void movelink(my_hash_link* array,unsigned int pos,unsigned int next_link,unsigned int newlink);
static int hashcmp(const my_hash* hash, my_hash_link* pos, const unsigned char* key, size_t length);

static my_hash_value_type hash_calc(const my_hash* hash, const unsigned char* key, size_t length)
{
  return hash->hash_function(hash, key, length);
}


/**
  Adaptor function which allows to use hash function from character
  set with my_hash.
*/

static my_hash_value_type cset_hash_sort_adapter(const my_hash* hash, const unsigned char* key, size_t length)
{
    unsigned long nr1 = 1, nr2 = 4;
	/* Todo list*/
    hash->charset->coll->hash_sort(hash->charset,(unsigned char*)key, length, &nr1, &nr2);
    return (my_hash_value_type)nr1;
}


/**
  @brief Initialize the hash
  
  @details

  Initialize the hash, by defining and giving valid values for
  its elements. The failure to allocate memory for the
  hash->array element will not result in a fatal failure. The
  dynamic array that is part of the hash will allocate memory
  as required during insertion.

  @param[in,out] hash         The hash that is initialized
  @param[in]     charset      The charater set information
  @param[in]     hash_function Hash function to be used. NULL -
                               use standard hash from character
                               set.
  @param[in]     size         The hash size
  @param[in]     key_offest   The key offset for the hash
  @param[in]     key_length   The length of the key used in
                              the hash
  @param[in]     get_key      get the key for the hash
  @param[in]     free_element pointer to the function that
                              does cleanup
  @param[in]     flags        flags set in the hash
  @return        inidicates success or failure of initialization
    @retval 0 success
    @retval 1 failure
*/
MY_GLOBAL_API int my_hash_init(my_hash* hash, unsigned int growth_size, 
                        my_hash_function hash_function,
                        unsigned long size, size_t key_offset, size_t key_length,
                        my_hash_get_key get_key,
                        void (*free_element)(void*), unsigned int flags)
{
    hash->records = 0;
    hash->key_offset = key_offset;
    hash->key_length = key_length;
    hash->blength = 1;
    hash->get_key = get_key;
    hash->free = free_element;
    hash->flags = flags;
    hash->hash_function = hash_function ? hash_function : cset_hash_sort_adapter;
    return my_array_init(&hash->array, sizeof(my_hash_link), size, growth_size));
}


/*
  Call hash->free on all elements in hash.

  SYNOPSIS
    my_hash_free_elements()
    hash   hash table

  NOTES:
    Sets records to 0
*/

static inline void my_hash_free_elements(my_hash* hash)
{
    my_hash_link* data = NULL;
	my_hash_link* end = NULL;
    if(hash->free)
    {
        data = my_array_element(&hash->array, 0, my_hash_link*)
        end = data + hash->records;
        while (data < end)
            (*hash->free)((data++)->data);
    }
    hash->records=0;
}


/*
  Free memory used by hash.

  SYNOPSIS
    my_hash_free()
    hash   the hash to delete elements of

  NOTES: Hash can't be reused without calling my_hash_init again.
*/

MY_GLOBAL_API void my_hash_free(my_hash* hash)
{
    my_hash_free_elements(hash);
    hash->free = 0;
    my_array_uninit(&hash->array);
    hash->blength = 0;
}


/*
  Delete all elements from the hash (the hash itself is to be reused).

  SYNOPSIS
    my_hash_reset()
    hash   the hash to delete elements of
*/

MY_GLOBAL_API void my_hash_reset(my_hash* hash)
{
    my_hash_free_elements(hash);
    my_array_reset(&hash->array);
    /* Set row pointers so that the hash can be reused at once */
    hash->blength = 1;
}

/* some helper functions */

/*
  This function is char* instead of unsigned char* as HPUX11 compiler can't
  handle inline functions that are not defined as native types
*/

static inline char* my_hash_key(const my_hash* hash, const unsigned char* record, size_t *length, int first)
{
    if(hash->get_key)
        return (char*)(*hash->get_key)(record, length, first);
    *length = hash->key_length;
    return (char*)(record + hash->key_offset);
}

	/* Calculate pos according to keys */

static unsigned int my_hash_mask(my_hash_value_type hashnr, size_t buffmax, size_t maxlength)
{
    if((hashnr & (buffmax - 1)) < maxlength) 
	    return (hashnr & (buffmax-1));
    return (hashnr & ((buffmax >> 1) - 1));
}

static unsigned int my_hash_rec_mask(const my_hash* hash, my_hash_link* pos, size_t buffmax, size_t maxlength)
{
    size_t length;
    unsigned char* key = (unsigned char*)my_hash_key(hash, pos->data, &length, 0);
    return my_hash_mask(hash_calc(hash, key, length), buffmax, maxlength);
}



static inline my_hash_value_type rec_hashnr(my_hash* hash, const unsigned char* record)
{
    size_t length;
    unsigned char* key = (unsigned char*)my_hash_key(hash, record, &length, 0);
    return hash_calc(hash,key,length);
}


MY_GLOBAL_API unsigned char* my_hash_search(const my_hash* hash, const unsigned char* key, size_t length)
{
    unsigned int state;
    return my_hash_first(hash, key, length, &state);
}

MY_GLOBAL_API unsigned char* my_hash_search_using_hash_value(const my_hash* hash, 
                        my_hash_value_type hash_value, const unsigned char* key, size_t length)
{
    unsigned int state;
    return my_hash_first_from_hash_value(hash, hash_value, key, length, &state);
}

MY_GLOBAL_API my_hash_value_type my_hash_calc(const my_hash* hash, const unsigned char* key, size_t length)
{
    return hash_calc(hash, key, length ? length : hash->key_length);
}


/*
  Search after a record based on a key

  NOTE
   Assigns the number of the found record to unsigned int state
*/

MY_GLOBAL_API unsigned char* my_hash_first(const my_hash* hash, const unsigned char* key, 
                        size_t length, unsigned int* current_record)
{
    unsigned char* res;
    if(hash->blength)
        res = my_hash_first_from_hash_value(hash, 
	                    hash_calc(hash, key, length ? length : hash->key_length), 
	  					key, length, current_record);
    else
        res = 0;
    return res;
}


MY_GLOBAL_API unsigned char* my_hash_first_from_hash_value(const my_hash* hash,
                        my_hash_value_type hash_value, const unsigned char* key,
                        size_t length, unsigned int* current_record)
{
    my_hash_link* pos;
    unsigned int flag,idx;
    
    flag = 1;
    if(hash->records)
    {
        idx = my_hash_mask(hash_value, hash->blength, hash->records);
        do
        {
            pos = my_array_element(&hash->array,idx,my_hash_link*);
            if(!hashcmp(hash,pos,key,length))
            {
	            *current_record = idx;
	            return pos->data;
            }
            if(flag)
            {
	            flag = 0;					/* Reset flag */
	            if(my_hash_rec_mask(hash, pos, hash->blength, hash->records) != idx)
	                break;				/* Wrong link */
            }
        }while ((idx=pos->next) != NO_RECORD);
    }
    *current_record= NO_RECORD;
	
    return 0;
}

	/* Get next record with identical key */
	/* Can only be called ifprevious calls was my_hash_search */

MY_GLOBAL_API unsigned char* my_hash_next(const my_hash* hash, const unsigned char* key, size_t length, unsigned int* current_record)
{
    my_hash_link* pos;
    unsigned int idx;
    
    if(*current_record != NO_RECORD)
    {
        my_hash_link* data = my_array_element(&hash->array, 0, my_hash_link*);
        for (idx = data[*current_record].next; idx != NO_RECORD; idx = pos->next)
        {
            pos = data + idx;
            if(!hashcmp(hash,pos,key,length))
            {
	            *current_record = idx;
	            return pos->data;
            }
        }
        *current_record = NO_RECORD;
    }
    return 0;
}


	/* Change link from pos to new_link */

static void movelink(my_hash_link* array, unsigned int find, unsigned int next_link, unsigned int newlink)
{
    my_hash_link* old_link;
    do
    {
        old_link = array + next_link;
    }while ((next_link = old_link->next) != find);
    old_link->next = newlink;
}

/*
  Compare a key in a record to a whole key. Return 0 ifidentical

  SYNOPSIS
    hashcmp()
    hash   hash table
    pos    position of hash record to use in comparison
    key    key for comparison
    length length of key

  NOTES:
    If length is 0, comparison is done using the length of the
    record being compared against.

  RETURN
    = 0  key of record == key
    != 0 key of record != key
 */

static int hashcmp(const my_hash* hash, my_hash_link* pos, const unsigned char* key,
                   size_t length)
{
    size_t key_length;
    unsigned char* rec_key = (unsigned char*) my_hash_key(hash, pos->data, &key_length, 1);
	/* Todo list.*/
    return ((length && length != key_length) ||
        my_strnncoll(hash->charset, (unsigned char*) rec_key, key_length, (unsigned char*) key, key_length));
}


	/* Write a hash-key to the hash-index */

MY_GLOBAL_API int my_hash_insert(my_hash* info, const unsigned char* record)
{
    int flag;
    size_t idx;
	size_t halfbuff;
    size_t first_index;
    my_hash_value_type hash_nr;
    unsigned char* ptr_to_rec = NULL;
	unsigned char* ptr_to_rec2 = NULL;
    my_hash_link* data;
    my_hash_link* empty;
	my_hash_link*  pos;
    my_hash_link* gpos = NULL;
	my_hasn_link* gpos2 = NULL;
    
    if(my_hash_UNIQUE & info->flags)
    {
        unsigned char* key = (unsigned char*)my_hash_key(info, record, &idx, 1);
        if(my_hash_search(info, key, idx))
            return 1;				/* Duplicate entry */
    }    
    flag=0;
    if(!(empty = (my_hash_link*)my_array_alloc(&info->array)))
        return 1;				/* No more memory */
    
    data = my_array_element(&info->array, 0, my_hash_link*);
    halfbuff = info->blength >> 1;
    
    idx = first_index = info->records-halfbuff;
    if(idx != info->records)				/* If some records */
    {
        do
        {
            pos = data + idx;
            hash_nr = rec_hashnr(info, pos->data);
            if(flag == 0)				/* First loop; Check ifok */
	            if(my_hash_mask(hash_nr, info->blength, info->records) != first_index)
	                break;
            if(!(hash_nr & halfbuff))
            {						/* Key will not move */
	            if(!(flag & LOWFIND))
	            {
	                if(flag & HIGHFIND)
	                {
	                    flag = LOWFIND | HIGHFIND;
	                    /* key shall be moved to the current empty position */
	                    gpos = empty;
	                    ptr_to_rec = pos->data;
	                    empty = pos;				/* This place is now free */
	                } else {
	                    flag = LOWFIND | LOWUSED;		/* key isn't changed */
	                    gpos = pos;
	                    ptr_to_rec = pos->data;
	                }
	            } else{
	                if(!(flag & LOWUSED))
	                {
	                    /* Change link of previous LOW-key */
	                    gpos->data = ptr_to_rec;
	                    gpos->next = (unsigned int) (pos - data);
	                    flag = (flag & HIGHFIND) | (LOWFIND | LOWUSED);
	                }
	                gpos = pos;
	                ptr_to_rec = pos->data;
	            }
            } else {						/* key will be moved */
	            if(!(flag & HIGHFIND))
	            {
	                flag = (flag & LOWFIND) | HIGHFIND;
	                /* key shall be moved to the last (empty) position */
	                gpos2 = empty; 
					empty = pos;
	                ptr_to_rec2 = pos->data;
	            } else {
	                if(!(flag & HIGHUSED))
	                {
	                    /* Change link of previous hash-key and save */
	                    gpos2->data = ptr_to_rec2;
	                    gpos2->next = (unsigned int) (pos-data);
	                    flag = (flag & LOWFIND) | (HIGHFIND | HIGHUSED);
	                }
	                gpos2 = pos;
	                ptr_to_rec2 = pos->data;
	            }
            }
        } while ((idx = pos->next) != NO_RECORD);        
        if((flag & (LOWFIND | LOWUSED)) == LOWFIND)
        {
            gpos->data = ptr_to_rec;
            gpos->next = NO_RECORD;
        }
        if((flag & (HIGHFIND | HIGHUSED)) == HIGHFIND)
        {
            gpos2->data = ptr_to_rec2;
            gpos2->next = NO_RECORD;
        }
    }
    /* Check ifwe are at the empty position */    
    idx = my_hash_mask(rec_hashnr(info, record), info->blength, info->records + 1);
    pos = data + idx;
    if(pos == empty)
    {
        pos->data = (unsigned char*) record;
        pos->next = NO_RECORD;
    } else{
        /* Check ifmore records in same hash-nr family */
        empty[0] = pos[0];
        gpos = data + my_hash_rec_mask(info, pos, info->blength, info->records + 1);
        if(pos == gpos)
        {
            pos->data = (unsigned char*) record;
            pos->next = (unsigned int) (empty - data);
        } else{
            pos->data = (unsigned char*) record;
            pos->next = NO_RECORD;
            movelink(data,(unsigned int)(pos - data), (unsigned int)(gpos-data), (unsigned int)(empty - data));
        }
    }
    if(++info->records == info->blength)
        info->blength += info->blength;
    return 0;
}


/******************************************************************************
** Remove one record from hash-table. The record with the same record
** ptr is removed.
** ifthere is a free-function it's called for record iffound
******************************************************************************/

MY_GLOBAL_API int my_hash_delete(my_hash* hash, unsigned char* record)
{
    unsigned int blength;
	unsigned int pos2;
	unsigned int idx;
	unsigned int empty_index;
    my_hash_value_type pos_hashnr;
    my_hash_value_type lastpos_hashnr;
    my_hash_link* data;
	my_hash_link* lastpos;
	my_hash_link* gpos;
	my_hash_link* pos;
	my_hash_link* pos3;
	my_hash_link* empty;
	
    if(!hash->records)
        return 1;    
    blength = hash->blength;
    data = my_array_element(&hash->array, 0, my_hash_link*);
    /* Search after record with key */
    pos = data + my_hash_mask(rec_hashnr(hash, record), blength, hash->records);
    gpos = 0;    
    while (pos->data != record)
    {
        gpos = pos;
        if(pos->next == NO_RECORD)
            return 1;			/* Key not found */
        pos = data + pos->next;
    }    
    if(--(hash->records) < hash->blength >> 1) 
	    hash->blength >>= 1;
    lastpos = data + hash->records;    
    /* Remove link to record */
    empty = pos; 
	empty_index = (unsigned int)(empty - data);
    if(gpos)
        gpos->next = pos->next;		/* unlink current ptr */
    else if(pos->next != NO_RECORD)
    {
        empty = data + (empty_index = pos->next);
        pos->data = empty->data;
        pos->next = empty->next;
    }    
    if(empty == lastpos)			/* last key at wrong pos or no next link */
        goto exit;    
    /* Move the last key (lastpos) */
    lastpos_hashnr = rec_hashnr(hash,lastpos->data);
    /* pos is where lastpos should be */
    pos = data + my_hash_mask(lastpos_hashnr, hash->blength, hash->records);
    if(pos == empty)			/* Move to empty position. */
    {
        empty[0] = lastpos[0];
        goto exit;
    }
    pos_hashnr = rec_hashnr(hash,pos->data);
    /* pos3 is where the pos should be */
    pos3 = data + my_hash_mask(pos_hashnr, hash->blength, hash->records);
    if(pos != pos3)
    {					/* pos is on wrong posit */
        empty[0] = pos[0];			/* Save it here */
        pos[0] = lastpos[0];			/* This should be here */
        movelink(data, (unsigned int)(pos-data), (unsigned int)(pos3 - data), empty_index);
        goto exit;
    }
    pos2 = my_hash_mask(lastpos_hashnr, blength, hash->records + 1);
    if(pos2 == my_hash_mask(pos_hashnr, blength, hash->records + 1))
    {					/* Identical key-positions */
        if(pos2 != hash->records)
        {
            empty[0] = lastpos[0];
            movelink(data, (unsigned int)(lastpos - data), (unsigned int)(pos - data), empty_index);
            goto exit;
        }
        idx = (unsigned int)(pos - data);		/* Link pos->next after lastpos */
    } else 
	    idx= NO_RECORD;		/* Different positions merge */    
    empty[0] = lastpos[0];
    movelink(data, idx, empty_index, pos->next);
    pos->next = empty_index;
    
exit:
    (void) my_array_pop(&hash->array);
    if(hash->free)
        (*hash->free)((unsigned char*) record);
    return 0;
}

	/*
	  Update keys when record has changed.
	  This is much more efficent than using a delete & insert.
	  */

MY_GLOBAL_API int my_hash_update(my_hash* hash, unsigned char* record, unsigned char* old_key,
                       size_t old_key_length)
{
    unsigned int new_index;
	unsigned int new_pos_index;
	unsigned int blength;
	unsigned int records;
    size_t idx;
	size_t empty;
    my_hash_link org_link;
	my_hash_link* data;
	my_hash_link* previous;
	my_hash_link* pos; 
	
    if(my_hash_UNIQUE & hash->flags)
    {
        unsigned int state;
        unsigned char* found, 
		unsigned char* new_key = (unsigned char*)my_hash_key(hash, record, &idx, 1);
        if((found = my_hash_first(hash, new_key, idx, &state)))
        {
            do 
            {
              if(found != record)
                  return 1;		/* Duplicate entry */
            } while ((found= my_hash_next(hash, new_key, idx, &state)));
        }
    }    
    data = my_array_element(&hash->array,0,my_hash_link*);
    blength = hash->blength; 
	records = hash->records;    
    /* Search after record with key */    
    idx = my_hash_mask(hash_calc(hash, old_key, (old_key_length ? old_key_length : hash->key_length)), blength, records);
    new_index = my_hash_mask(rec_hashnr(hash, record), blength, records);
    if(idx == new_index)
        return 0;			/* Nothing to do (No record check) */
    previous = 0;
    for (;;)
    {    
        if((pos = data + idx)->data == record)
            break;
        previous = pos;
        if((idx = pos->next) == NO_RECORD)
            return 1;			/* Not found in links */
    }
    org_link = *pos;
    empty = idx;    
    /* Relink record from current chain */    
    if(!previous)
    {
        if(pos->next != NO_RECORD)
        {
            empty = pos->next;
            *pos = data[pos->next];
        }
    } else
        previous->next = pos->next;		/* unlink pos */    
    /* Move data to correct position */
    if(new_index == empty)
    {
        /*
          At this point record is unlinked from the old chain, thus it holds
          random position. By the chance this position is equal to position
          for the first element in the new chain. That means updated record
          is the only record in the new chain.
        */
        if(empty != idx)
            /*
              Record was moved while unlinking it from the old chain.
              Copy data to a new position.
            */
            data[empty] = org_link;

        data[empty].next = NO_RECORD;
        return 0;
    }
    pos = data+new_index;
    new_pos_index = my_hash_rec_mask(hash, pos, blength, records);
    if(new_index != new_pos_index)
    {					/* Other record in wrong position */
        data[empty] = *pos;
        movelink(data, new_index, new_pos_index, empty);
        org_link.next = NO_RECORD;
        data[new_index] = org_link;
    } else{					/* Link in chain at right position */
        org_link.next = data[new_index].next;
        data[empty] = org_link;
        data[new_index].next = empty;
    }
    return 0;
}


MY_GLOBAL_API unsigned char* my_hash_element(my_hash* hash, unsigned long idx)
{
    if(idx < hash->records)
        return my_array_element(&hash->array, idx, my_hash_link*)->data;
    return 0;
}


/*
  Replace old row with new row.  This should only be used when key
  isn't changed
*/

MY_GLOBAL_API void my_hash_replace(my_hash* hash, unsigned int* current_record,
                     unsigned char* new_row)
{
    if(*current_record != NO_RECORD)            /* Safety */
        my_array_element(&hash->array, *current_record, my_hash_link*)->data = new_row;
}

