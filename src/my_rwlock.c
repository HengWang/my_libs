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
#include "my_rwlock.h"

#ifdef _WIN32

static BOOL have_srwlock= FALSE;
/* Prototypes and function pointers for windows  functions */
typedef VOID (WINAPI* srw_func)(PSRWLOCK SRWLock);
typedef BOOLEAN (WINAPI* srw_bool_func)(PSRWLOCK SRWLock);

static srw_func my_InitializeSRWLock;
static srw_func my_AcquireSRWLockExclusive;
static srw_func my_ReleaseSRWLockExclusive;
static srw_func my_AcquireSRWLockShared;
static srw_func my_ReleaseSRWLockShared;

static srw_bool_func my_TryAcquireSRWLockExclusive;
static srw_bool_func my_TryAcquireSRWLockShared;

/**
  Check for presence of Windows slim reader writer lock function.
  Load function pointers.
*/
static void check_srwlock_availability(void)
{
    HMODULE module= GetModuleHandle("kernel32");    
    my_InitializeSRWLock = (srw_func)GetProcAddress(module, "InitializeSRWLock");
    my_AcquireSRWLockExclusive = (srw_func)GetProcAddress(module, "AcquireSRWLockExclusive");
    my_AcquireSRWLockShared = (srw_func)GetProcAddress(module, "AcquireSRWLockShared");
    my_ReleaseSRWLockExclusive = (srw_func)GetProcAddress(module, "ReleaseSRWLockExclusive");
    my_ReleaseSRWLockShared = (srw_func)GetProcAddress(module, "ReleaseSRWLockShared");
    my_TryAcquireSRWLockExclusive =  (srw_bool_func)GetProcAddress(module, "TryAcquireSRWLockExclusive");
    my_TryAcquireSRWLockShared =  (srw_bool_func)GetProcAddress(module, "TryAcquireSRWLockShared");
    /*
      We currently require TryAcquireSRWLockExclusive. This API is missing on 
      Vista, this means SRWLock are only used starting with Win7.
    
      If "trylock" usage for rwlocks is eliminated from server codebase (it is used 
      in a single place currently, in query cache), then SRWLock can be enabled on 
      Vista too. In this case  condition below needs to be changed to  e.g check 
      for my_InitializeSRWLock.
    */
    if (my_TryAcquireSRWLockExclusive)
        have_srwlock= TRUE;
}


static int srw_init(my_rw_lock_t* rwlock)
{
    my_InitializeSRWLock(&rwlock->srwlock);
    rwlock->have_exclusive_srwlock = FALSE;
	
    return 0;
}


static int srw_rdlock(my_rw_lock_t* rwlock)
{
    my_AcquireSRWLockShared(&rwlock->srwlock);
	
    return 0;
}


static int srw_tryrdlock(my_rw_lock_t* rwlock)
{
    if (!my_TryAcquireSRWLockShared(&rwlock->srwlock))
        return EBUSY;
	  
    return 0;
}


static int srw_wrlock(my_rw_lock_t* rwlock)
{
    my_AcquireSRWLockExclusive(&rwlock->srwlock);
    rwlock->have_exclusive_srwlock= TRUE;
	
    return 0;
}


static int srw_trywrlock(my_rw_lock_t* rwlock)
{
    if (!my_TryAcquireSRWLockExclusive(&rwlock->srwlock))
        return EBUSY;
    rwlock->have_exclusive_srwlock= TRUE;
	
    return 0;
}


static int srw_unlock(my_rw_lock_t* rwlock)
{
    if (rwlock->have_exclusive_srwlock)
    {
      rwlock->have_exclusive_srwlock= FALSE;
      my_ReleaseSRWLockExclusive(&rwlock->srwlock);
    }
    else
    {
      my_ReleaseSRWLockShared(&rwlock->srwlock);
    }
	
    return 0;
}

MY_GLOBAL_API int my_rw_init(my_rw_lock* rwlock)
{
    pthread_condattr_t	cond_attr;
    
    /*
      Once initialization is used here rather than in my_init(), in order to
      - avoid  my_init() pitfalls- (undefined order in which initialization should
      run)
      - be potentially useful C++ (static constructors) 
      - just to simplify  the API. 
      Also, the overhead is of my_pthread_once is very small.
    */
    static my_pthread_once_t once_control= MY_PTHREAD_ONCE_INIT;
    my_pthread_once(&once_control, check_srwlock_availability);   
    if(have_srwlock)
        return srw_init(rwlock);	
    pthread_mutex_init(&rwlock->lock, MY_MUTEX_INIT_FAST);
    pthread_condattr_init(&cond_attr);
    pthread_cond_init(&rwlock->readers, &cond_attr);
    pthread_cond_init(&rwlock->writers, &cond_attr);
    pthread_condattr_destroy(&cond_attr);    
    rwlock->state = 0;
    rwlock->waiters	= 0;

    return(0);
}


MY_GLOBAL_API int my_rw_destroy(my_rw_lock* rwlock)
{
    if(have_srwlock)
        return 0; /* no destroy function */
    assert(rwlock->state == 0);
    pthread_mutex_destroy(&rwlock->lock);
    pthread_cond_destroy(&rwlock->readers);
    pthread_cond_destroy(&rwlock->writers);
	
    return 0;
}


MY_GLOBAL_API int my_rw_rdlock(my_rw_lock* rwlock)
{
    if(have_srwlock)
        return srw_rdlock(rwlock);
    pthread_mutex_lock(&rwlock->lock);
    /* active or queued writers */
    while ((rwlock->state < 0) || rwlock->waiters)
        pthread_cond_wait(&rwlock->readers, &rwlock->lock);
    rwlock->state++;
    pthread_mutex_unlock(&rwlock->lock);
    return(0);
}

MY_GLOBAL_API int my_rw_tryrdlock(my_rw_lock* rwlock)
{
    int res;

	if (have_srwlock)
        return srw_tryrdlock(rwlock);
    pthread_mutex_lock(&rwlock->lock);
    if ((rwlock->state < 0) || rwlock->waiters)
        res= EBUSY;					/* Can't get lock */
    else
    {
        res=0;
        rwlock->state++;
    }
    pthread_mutex_unlock(&rwlock->lock);
	
    return res;
}


MY_GLOBAL_API int my_rw_wrlock(my_rw_lock* rwlock)
{
    if (have_srwlock)
        return srw_wrlock(rwlock);
    pthread_mutex_lock(&rwlock->lock);
    rwlock->waiters++;				/* another writer queued */    
    my_rw_lock_assert_not_write_owner(rwlock);    
    while (rwlock->state)
        pthread_cond_wait(&rwlock->writers, &rwlock->lock);
    rwlock->state	= -1;
    rwlock->waiters--;
    pthread_mutex_unlock(&rwlock->lock);
	
    return 0;
}


MY_GLOBAL_API int my_rw_trywrlock(my_rw_lock* rwlock)
{
    int res;

	if (have_srwlock)
        return srw_trywrlock(rwlock);
    pthread_mutex_lock(&rwlock->lock);
    if (rwlock->state)
      res= EBUSY;					/* Can't get lock */    
    else
    {
      res=0;
      rwlock->state	= -1;
    }
    pthread_mutex_unlock(&rwlock->lock);
	
    return res;
}


MY_GLOBAL_API int my_rw_unlock(my_rw_lock* rwlock)
{
    if (have_srwlock)
        return srw_unlock(rwlock);
    /*
      The DBUG api uses rw locks to protect global debug settings. Calling into
      the DBUG api from here can cause a deadlock.
    
      DBUG_PRINT("rw_unlock", ("state: %d waiters: %d",
                 rwlock->state, rwlock->waiters));
    */
    pthread_mutex_lock(&rwlock->lock);    
    assert(rwlock->state != 0);
    
    if (rwlock->state == -1)		/* writer releasing */
    {
        my_rw_lock_assert_write_owner(rwlock);
        rwlock->state= 0;		/* mark as available */
        if (rwlock->waiters)		/* writers queued */
            pthread_cond_signal(&rwlock->writers);
        else
            pthread_cond_broadcast(&rwlock->readers);
    }
    else
    {
        if (--rwlock->state == 0 && rwlock->waiters)  /* no more readers */
            pthread_cond_signal(&rwlock->writers);
    }    
    pthread_mutex_unlock(&rwlock->lock);
	
    return(0);
}

#endif /*_WIN32 */

