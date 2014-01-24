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
 
#ifndef __MY_PR_RWLOCK_H
#define __MY_PR_RWLOCK_H

#include "my_global_exports.h"

C_MODE_START
/**
  Portable implementation of special type of read-write locks.

  These locks have two properties which are unusual for rwlocks:
  1) They "prefer readers" in the sense that they do not allow
     situations in which rwlock is rd-locked and there is a
     pending rd-lock which is blocked (e.g. due to pending
     request for wr-lock).
     This is a stronger guarantee than one which is provided for
     PTHREAD_RWLOCK_PREFER_READER_NP rwlocks in Linux.
     MDL subsystem deadlock detector relies on this property for
     its correctness.
  2) They are optimized for uncontended wr-lock/unlock case.
     This is scenario in which they are most oftenly used
     within MDL subsystem. Optimizing for it gives significant
     performance improvements in some of tests involving many
     connections.

  Another important requirement imposed on this type of rwlock
  by the MDL subsystem is that it should be OK to destroy rwlock
  object which is in unlocked state even though some threads might
  have not yet fully left unlock operation for it (of course there
  is an external guarantee that no thread will try to lock rwlock
  which is destroyed).
  Putting it another way the unlock operation should not access
  rwlock data after changing its state to unlocked.

  TODO/FIXME: We should consider alleviating this requirement as
  it blocks us from doing certain performance optimizations.
*/

typedef struct st_rw_pr_lock_t {
    /**
      Lock which protects the structure.
      Also held for the duration of wr-lock.
    */
    pthread_mutex_t lock;
    /**
      Condition variable which is used to wake-up
      writers waiting for readers to go away.
    */
    pthread_cond_t no_active_readers;
    /** Number of active readers. */
    uint active_readers;
    /** Number of writers waiting for readers to go away. */
    uint writers_waiting_readers;
    /** Indicates whether there is an active writer. */
    my_bool active_writer;
} rw_pr_lock_t;

MY_GLOBAL_API int rw_pr_init(rw_pr_lock_t *);
MY_GLOBAL_API int rw_pr_rdlock(rw_pr_lock_t *);
MY_GLOBAL_API int rw_pr_wrlock(rw_pr_lock_t *);
MY_GLOBAL_API int rw_pr_unlock(rw_pr_lock_t *);
MY_GLOBAL_API int rw_pr_destroy(rw_pr_lock_t *);

C_MODE_START

#endif  /* __MY_PR_RWLOCK_H */

