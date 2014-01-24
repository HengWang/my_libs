/*
*  Copyright (c) 2013, Heng Wang personal. All rights reserved.
*  
*  
* 
*  @Author:  Heng.Wang
*  @Date  :  12/24/2013
*  @Email :  wangheng.king@gmail.com
*            king_wangheng@163.com
*  @Github:  https://github.com/HengWang/
*  @Blog  :  http://hengwang.blog.chinaunix.net
* */

#include <assert.h>

#include "my_pthread.h"
#include "my_pr_rwlock.h"

MY_GLOBAL_API int rw_pr_init(rw_pr_lock_t* rwlock)
{
    pthread_mutex_init(&rwlock->lock, NULL);
    pthread_cond_init(&rwlock->no_active_readers, NULL);
    rwlock->active_readers= 0;
    rwlock->writers_waiting_readers= 0;
    rwlock->active_writer= FALSE;
    
    return 0;
}


MY_GLOBAL_API int rw_pr_destroy(rw_pr_lock_t *rwlock)
{
    pthread_cond_destroy(&rwlock->no_active_readers);
    pthread_mutex_destroy(&rwlock->lock);
	
    return 0;
}


MY_GLOBAL_API int rw_pr_rdlock(rw_pr_lock_t *rwlock)
{
    pthread_mutex_lock(&rwlock->lock);
    /*
      The fact that we were able to acquire 'lock' mutex means
      that there are no active writers and we can acquire rd-lock.
      Increment active readers counter to prevent requests for
      wr-lock from succeeding and unlock mutex.
    */
    rwlock->active_readers++;
    pthread_mutex_unlock(&rwlock->lock);
	
    return 0;
}


MY_GLOBAL_API int rw_pr_wrlock(rw_pr_lock_t *rwlock)
{
    pthread_mutex_lock(&rwlock->lock);
    
    if (rwlock->active_readers != 0)
    {
        /* There are active readers. We have to wait until they are gone. */
        rwlock->writers_waiting_readers++;        
        while (rwlock->active_readers != 0)
            pthread_cond_wait(&rwlock->no_active_readers, &rwlock->lock);        
        rwlock->writers_waiting_readers--;
    }    
    /*
      We own 'lock' mutex so there is no active writers.
      Also there are no active readers.
      This means that we can grant wr-lock.
      Not releasing 'lock' mutex until unlock will block
      both requests for rd and wr-locks.
      Set 'active_writer' flag to simplify unlock.
    
      Thanks to the fact wr-lock/unlock in the absence of
      contention from readers is essentially mutex lock/unlock
      with a few simple checks make this rwlock implementation
      wr-lock optimized.
    */
    rwlock->active_writer= TRUE;

    return 0;
}


MY_GLOBAL_API int rw_pr_unlock(rw_pr_lock_t *rwlock)
{
    if (rwlock->active_writer)
    {
        /* We are unlocking wr-lock. */
        rwlock->active_writer= FALSE;
        if (rwlock->writers_waiting_readers)
        {
            /*
              Avoid expensive cond signal in case when there is no contention
              or it is wr-only.
            
              Note that from view point of performance it would be better to
              signal on the condition variable after unlocking mutex (as it
              reduces number of contex switches).
            
              Unfortunately this would mean that such rwlock can't be safely
              used by MDL subsystem, which relies on the fact that it is OK
              to destroy rwlock once it is in unlocked state.
            */
            pthread_cond_signal(&rwlock->no_active_readers);
        }
        pthread_mutex_unlock(&rwlock->lock);
    } else{
        /* We are unlocking rd-lock. */
        pthread_mutex_lock(&rwlock->lock);
        rwlock->active_readers--;
        if (rwlock->active_readers == 0 && rwlock->writers_waiting_readers)
        {
            /*
              If we are last reader and there are waiting
              writers wake them up.
            */
            pthread_cond_signal(&rwlock->no_active_readers);
        }
        pthread_mutex_unlock(&rwlock->lock);
    }
    return 0;
}

