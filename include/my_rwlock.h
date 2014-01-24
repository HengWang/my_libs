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
 
#ifndef __MY_RWLOCK_H
#define __MY_RWLOCK_H

#include "my_global_exports.h"

C_MODE_START

/* READ-WRITE thread locking */

#ifndef _WIN32  /* read/write locks using pthread */

#define rw_lock_t pthread_rwlock_t
#define my_rwlock_init(A,B) pthread_rwlock_init((A),(B))
#define rw_rdlock(A) pthread_rwlock_rdlock(A)
#define rw_wrlock(A) pthread_rwlock_wrlock(A)
#define rw_tryrdlock(A) pthread_rwlock_tryrdlock((A))
#define rw_trywrlock(A) pthread_rwlock_trywrlock((A))
#define rw_unlock(A) pthread_rwlock_unlock(A)
#define rwlock_destroy(A) pthread_rwlock_destroy(A)

#else /* _WIN32 */

/* Use our own version of read/write locks */
#define rw_lock_t my_rw_lock
#define my_rwlock_init(A,B) my_rw_init((A))
#define rw_rdlock(A) my_rw_rdlock((A))
#define rw_wrlock(A) my_rw_wrlock((A))
#define rw_tryrdlock(A) my_rw_tryrdlock((A))
#define rw_trywrlock(A) my_rw_trywrlock((A))
#define rw_unlock(A) my_rw_unlock((A))
#define rwlock_destroy(A) my_rw_destroy((A))

/**
  Implementation of Windows rwlock.

  We use native (slim) rwlocks on Win7 and later, and fallback to  portable
  implementation on earlier Windows.

  slim rwlock are also available on Vista/WS2008, but we do not use it
  ("trylock" APIs are missing on Vista)
*/

typedef union
{
    /* Native rwlock (is_srwlock == TRUE) */
    struct
    {
      SRWLOCK srwlock;             /* native reader writer lock */
      BOOL have_exclusive_srwlock; /* used for unlock */
    };
    
    /*
      Portable implementation (is_srwlock == FALSE)
      Fields are identical with Unix my_rw_lock_t fields.
    */
    struct
    {
      pthread_mutex_t lock;       /* lock for structure		*/
      pthread_cond_t  readers;    /* waiting readers		*/
      pthread_cond_t  writers;    /* waiting writers		*/
      int state;                  /* -1:writer,0:free,>0:readers	*/
      int waiters;                /* number of waiting writers	*/
    };
} my_rw_lock_t;

MY_GLOBAL_API int my_rw_init(my_rw_lock_t *);
MY_GLOBAL_API int my_rw_destroy(my_rw_lock_t *);
MY_GLOBAL_API int my_rw_rdlock(my_rw_lock_t *);
MY_GLOBAL_API int my_rw_wrlock(my_rw_lock_t *);
MY_GLOBAL_API int my_rw_unlock(my_rw_lock_t *);
MY_GLOBAL_API int my_rw_tryrdlock(my_rw_lock_t *);
MY_GLOBAL_API int my_rw_trywrlock(my_rw_lock_t *);

#endif /* _WIN32 */

C_MODE_START

#endif  /* __MY_RWLOCK_H */
