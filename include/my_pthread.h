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
 
#ifndef __MY_PTHREAD_H
#define __MY_PTHREAD_H

#include "my_global_exports.h"

C_MODE_START

#if defined(_WIN32)

typedef CRITICAL_SECTION pthread_mutex_t;
typedef DWORD pthread_t;
typedef struct thread_attr {
    DWORD dwStackSize ;
    DWORD dwCreatingFlag ;
} pthread_attr_t ;

typedef struct { int dummy; } pthread_condattr_t;

/* Implementation of posix conditions */

typedef struct pthread_link_t {
    DWORD thread_id;
    struct pthread_link_t *next;
} pthread_link;

/**
  Implementation of Windows condition variables.
  We use native conditions as they are available on Vista and later.
*/
typedef CONDITION_VARIABLE pthread_cond_t;

typedef int pthread_mutexattr_t;
#define pthread_self() GetCurrentThreadId()
#define pthread_handler_t EXTERNC void * __cdecl
typedef void * (__cdecl *pthread_handler)(void *);

typedef volatile long my_pthread_once_t;
#define MY_PTHREAD_ONCE_INIT  0
#define MY_PTHREAD_ONCE_INPROGRESS 1
#define MY_PTHREAD_ONCE_DONE 2

/*
  Existing mysql_thread_create() or pthread_create() does not work well
  in windows platform when threads are joined because
  A)during thread creation, thread handle is not stored.
  B)during thread join, thread handle is retrieved using OpenThread().
    OpenThread() does not behave properly when thread to be joined is already
    exited.
  Use pthread_create_get_handle() and pthread_join_with_handle() function
  instead of mysql_thread_create() function for windows joinable threads.
*/
int pthread_create(pthread_t *, const pthread_attr_t *, pthread_handler, void *);
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
               struct timespec *abstime);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_attr_init(pthread_attr_t *connect_att);
int pthread_attr_setstacksize(pthread_attr_t *connect_att,DWORD stack);
int pthread_attr_getstacksize(pthread_attr_t *connect_att, size_t *stack);
int pthread_attr_destroy(pthread_attr_t *connect_att);
int my_pthread_once(my_pthread_once_t *once_control,void (*init_routine)(void));
struct tm *localtime_r(const time_t *timep,struct tm *tmp);
struct tm *gmtime_r(const time_t *timep,struct tm *tmp);

/**
  Create thread.

  Existing mysql_thread_create does not work well in windows platform
  when threads are joined. Use pthread_create_get_handle() and
  pthread_join_with_handle() function instead of mysql_thread_create()
  function for windows.

  @param thread_id    reference to pthread object
  @param attr         reference to pthread attribute
  @param func         pthread handler function
  @param param        parameters to pass to newly created thread
  @param out_handle   output parameter to get newly created thread handle

  @return int
    @retval 0 success
    @retval 1 failure
*/
int pthread_create_get_handle(pthread_t *thread_id,
                              const pthread_attr_t *attr,
                              pthread_handler func, void *param,
                              HANDLE *out_handle);

/**
  Wait for thread termination.

  @param handle       handle of the thread to wait for

  @return  int
    @retval 0 success
    @retval 1 failure
*/
int pthread_join_with_handle(HANDLE handle);

void pthread_exit(void *a);

/*
  Existing pthread_join() does not work well in windows platform when
  threads are joined because
  A)during thread creation thread handle is not stored.
  B)during thread join, thread handle is retrieved using OpenThread().
    OpenThread() does not behave properly when thread to be joined is already
    exited.

  Use pthread_create_get_handle() and pthread_join_with_handle()
  function instead for windows joinable threads.
*/
int pthread_join(pthread_t thread, void **value_ptr);
int pthread_cancel(pthread_t thread);
extern int pthread_dummy(int);

#ifndef ETIMEDOUT
    #define ETIMEDOUT 145           /* Win32 doesn't have this */
#endif

#define pthread_key(T,V)  DWORD V
#define pthread_key_create(A,B) ((*A=TlsAlloc())==0xFFFFFFFF)
#define pthread_key_delete(A) TlsFree(A)
#define my_pthread_setspecific_ptr(T,V) (!TlsSetValue((T),(V)))
#define pthread_setspecific(A,B) (!TlsSetValue((A),(B)))
#define pthread_getspecific(A) (TlsGetValue(A))
#define my_pthread_getspecific(T,A) ((T) TlsGetValue(A))
#define my_pthread_getspecific_ptr(T,V) ((T) TlsGetValue(V))

#define pthread_equal(A,B) ((A) == (B))
#define pthread_mutex_init(A,B)  (InitializeCriticalSection(A),0)
#define pthread_mutex_lock(A)    (EnterCriticalSection(A),0)
int win_pthread_mutex_trylock(pthread_mutex_t *mutex);
#define pthread_mutex_unlock(A)  (LeaveCriticalSection(A), 0)
#define pthread_mutex_destroy(A) (DeleteCriticalSection(A), 0)
#define pthread_kill(A,B) pthread_dummy((A) ? 0 : ESRCH)



/* Dummy defines for easier code */
#define pthread_attr_setdetachstate(A,B) pthread_dummy(0)
#define pthread_attr_setscope(A,B)
#define pthread_condattr_init(A)
#define pthread_condattr_destroy(A)
#define pthread_yield() SwitchToThread()
#define my_sigset(A,B) signal(A,B)

#else /* Normal threads */

#include <pthread.h>
#ifdef HAVE_SCHED_H
    #include <sched.h>
#endif
#ifdef HAVE_SYNCH_H
    #include <synch.h>
#endif

#define pthread_key(T,V) pthread_key_t V
#define my_pthread_getspecific_ptr(T,V) my_pthread_getspecific(T,(V))
#define my_pthread_setspecific_ptr(T,V) pthread_setspecific(T,(void*) (V))
#define pthread_handler_t EXTERNC void *
typedef void *(* pthread_handler)(void *);

#define my_pthread_once_t pthread_once_t
#if defined(PTHREAD_ONCE_INITIALIZER)
    #define MY_PTHREAD_ONCE_INIT PTHREAD_ONCE_INITIALIZER
#else
    #define MY_PTHREAD_ONCE_INIT PTHREAD_ONCE_INIT
#endif
#define my_pthread_once(C,F) pthread_once(C,F)

/*
  We define my_sigset() and use that instead of the system sigset() so that
  we can favor an implementation based on sigaction(). On some systems, such
  as Mac OS X, sigset() results in flags such as SA_RESTART being set, and
  we want to make sure that no such flags are set.
*/
#if defined(HAVE_SIGACTION) && !defined(my_sigset)
    #define my_sigset(A,B) do { struct sigaction l_s; sigset_t l_set;       \
                            DBUG_ASSERT((A) != 0);                          \
                            sigemptyset(&l_set);                            \
                            l_s.sa_handler = (B);                           \
                            l_s.sa_mask   = l_set;                          \
                            l_s.sa_flags   = 0;                             \
                            sigaction((A), &l_s, NULL);                     \
                          } while (0)
#elif defined(HAVE_SIGSET) && !defined(my_sigset)
    #define my_sigset(A,B) sigset((A),(B))
#elif !defined(my_sigset)
    #define my_sigset(A,B) signal((A),(B))
#endif

#define my_pthread_getspecific(A,B) ((A) pthread_getspecific(B))

#endif /* defined(_WIN32) */

#if !defined(HAVE_PTHREAD_YIELD_ONE_ARG) && !defined(HAVE_PTHREAD_YIELD_ZERO_ARG)
/* no pthread_yield() available */
    #ifdef HAVE_SCHED_YIELD
        #define pthread_yield() sched_yield()
    #elif defined(HAVE_PTHREAD_YIELD_NP) /* can be Mac OS X */
        #define pthread_yield() pthread_yield_np()
    #elif defined(HAVE_THR_YIELD)
        #define pthread_yield() thr_yield()
    #endif
#endif

C_MODE_END

#endif /* __MY_PTHREAD_H */
