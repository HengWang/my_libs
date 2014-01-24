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
 
#if defined(_WIN32)

#include <process.h>
#include <sys/timeb.h>
#include <time.h>

struct err_table_t 
{
    unsigned long oscode;   /* OS return value */
    int sysv_errno;  /* System V error code */
};

static struct err_table_t err_table[] = {
    {  ERROR_INVALID_FUNCTION,       EINVAL    },  /* 1 */
    {  ERROR_FILE_NOT_FOUND,         ENOENT    },  /* 2 */
    {  ERROR_PATH_NOT_FOUND,         ENOENT    },  /* 3 */
    {  ERROR_TOO_MANY_OPEN_FILES,    EMFILE    },  /* 4 */
    {  ERROR_ACCESS_DENIED,          EACCES    },  /* 5 */
    {  ERROR_INVALID_HANDLE,         EBADF     },  /* 6 */
    {  ERROR_ARENA_TRASHED,          ENOMEM    },  /* 7 */
    {  ERROR_NOT_ENOUGH_MEMORY,      ENOMEM    },  /* 8 */
    {  ERROR_INVALID_BLOCK,          ENOMEM    },  /* 9 */
    {  ERROR_BAD_ENVIRONMENT,        E2BIG     },  /* 10 */
    {  ERROR_BAD_FORMAT,             ENOEXEC   },  /* 11 */
    {  ERROR_INVALID_ACCESS,         EINVAL    },  /* 12 */
    {  ERROR_INVALID_DATA,           EINVAL    },  /* 13 */
    {  ERROR_INVALID_DRIVE,          ENOENT    },  /* 15 */
    {  ERROR_CURRENT_DIRECTORY,      EACCES    },  /* 16 */
    {  ERROR_NOT_SAME_DEVICE,        EXDEV     },  /* 17 */
    {  ERROR_NO_MORE_FILES,          ENOENT    },  /* 18 */
    {  ERROR_LOCK_VIOLATION,         EACCES    },  /* 33 */
    {  ERROR_BAD_NETPATH,            ENOENT    },  /* 53 */
    {  ERROR_NETWORK_ACCESS_DENIED,  EACCES    },  /* 65 */
    {  ERROR_BAD_NET_NAME,           ENOENT    },  /* 67 */
    {  ERROR_FILE_EXISTS,            EEXIST    },  /* 80 */
    {  ERROR_CANNOT_MAKE,            EACCES    },  /* 82 */
    {  ERROR_FAIL_I24,               EACCES    },  /* 83 */
    {  ERROR_INVALID_PARAMETER,      EINVAL    },  /* 87 */
    {  ERROR_NO_PROC_SLOTS,          EAGAIN    },  /* 89 */
    {  ERROR_DRIVE_LOCKED,           EACCES    },  /* 108 */
    {  ERROR_BROKEN_PIPE,            EPIPE     },  /* 109 */
    {  ERROR_DISK_FULL,              ENOSPC    },  /* 112 */
    {  ERROR_INVALID_TARGET_HANDLE,  EBADF     },  /* 114 */
    {  ERROR_INVALID_NAME,           ENOENT    },  /* 123 */
    {  ERROR_INVALID_HANDLE,         EINVAL    },  /* 124 */
    {  ERROR_WAIT_NO_CHILDREN,       ECHILD    },  /* 128 */
    {  ERROR_CHILD_NOT_COMPLETE,     ECHILD    },  /* 129 */
    {  ERROR_DIRECT_ACCESS_HANDLE,   EBADF     },  /* 130 */
    {  ERROR_NEGATIVE_SEEK,          EINVAL    },  /* 131 */
    {  ERROR_SEEK_ON_DEVICE,         EACCES    },  /* 132 */
    {  ERROR_DIR_NOT_EMPTY,          ENOTEMPTY },  /* 145 */
    {  ERROR_NOT_LOCKED,             EACCES    },  /* 158 */
    {  ERROR_BAD_PATHNAME,           ENOENT    },  /* 161 */
    {  ERROR_MAX_THRDS_REACHED,      EAGAIN    },  /* 164 */
    {  ERROR_LOCK_FAILED,            EACCES    },  /* 167 */
    {  ERROR_ALREADY_EXISTS,         EEXIST    },  /* 183 */
    {  ERROR_FILENAME_EXCED_RANGE,   ENOENT    },  /* 206 */
    {  ERROR_NESTING_NOT_ALLOWED,    EAGAIN    },  /* 215 */
    {  ERROR_NOT_ENOUGH_QUOTA,       ENOMEM    }    /* 1816 */
};

/* size of the table */
#define ERR_TABLE_SIZE (sizeof(err_table)/sizeof(err_table[0]))

/* The following two constants must be the minimum and maximum
values in the (contiguous) range of Exec Failure errors. */
#define MIN_EXEC_ERROR ERROR_INVALID_STARTING_CODESEG
#define MAX_EXEC_ERROR ERROR_INFLOOP_IN_RELOC_CHAIN

/* These are the low and high value in the range of errors that are
access violations */
#define MIN_EACCES_RANGE ERROR_WRITE_PROTECT
#define MAX_EACCES_RANGE ERROR_SHARING_BUFFER_EXCEEDED


static int get_errno_from_oserr(unsigned long oserrno)
{
    int i;
    
    /* check the table for the OS error code */
    for (i= 0; i < ERR_TABLE_SIZE; ++i) 
    {
        if (oserrno == err_table[i].oscode) 
        {
            return  err_table[i].sysv_errno;
        }
    }    
    /* The error code wasn't in the table.  We check for a range of */
    /* EACCES errors or exec failure errors (ENOEXEC).  Otherwise   */
    /* EINVAL is returned.                                      */
    if (oserrno >= MIN_EACCES_RANGE && oserrno <= MAX_EACCES_RANGE)
        return EACCES;
    else if (oserrno >= MIN_EXEC_ERROR && oserrno <= MAX_EXEC_ERROR)
        return ENOEXEC;
    else
        return EINVAL;
}

/* Set errno corresponsing to GetLastError() value */
void my_osmaperr( unsigned long os_errno)
{
    /*
      set my_winerr so that we could return the Windows Error Code
      when it is EINVAL.
    */
    my_winerr = os_errno;
    errno = get_errno_from_oserr(os_errno);
}

/*
  Struct and macros to be used in combination with the
  windows implementation of pthread_cond_timedwait
*/

/*
   Declare a union to make sure FILETIME is properly aligned
   so it can be used directly as a 64 bit value. The value
   stored is in 100ns units.
*/
union ft64 {
    FILETIME ft;
    int64 i64;
};
struct timespec {
    union ft64 tv;
    /* The max timeout value in millisecond for pthread_cond_timedwait*/
    long max_timeout_msec;
};

/**
  Convert abstime to milliseconds
*/
static DWORD get_milliseconds(const struct timespec* abstime)
{
    long long millis; 
    union ft64 now;

    if(abstime == NULL)
        return INFINITE;
    GetSystemTimeAsFileTime(&now.ft);
    /*
      Calculate time left to abstime
      - subtract start time from current time(values are in 100ns units)
      - convert to millisec by dividing with 10000
    */
    millis = (abstime->tv.i64 - now.i64) / 10000;
  
    /* Don't allow the timeout to be negative*/
    if(millis < 0)
        return 0;
    /*
      Make sure the calculated timeout does not exceed original timeout
      value which could cause "wait for ever" if system time changes
    */
    if(millis > abstime->max_timeout_msec)
        millis = abstime->max_timeout_msec;    
    if(millis > UINT_MAX)
        millis = UINT_MAX;
    
    return (DWORD)millis;
}

/*
  Posix API functions using native implementation of condition variables.
*/

int pthread_cond_init(pthread_cond_t* cond, const pthread_condattr_t* attr)
{
    InitializeConditionVariable(cond);
    return 0;
}


int pthread_cond_destroy(pthread_cond_t* cond)
{
    return 0; /* no destroy function */
}


int pthread_cond_broadcast(pthread_cond_t* cond)
{
    WakeAllConditionVariable(cond);
    return 0;
}


int pthread_cond_signal(pthread_cond_t* cond)
{
    WakeConditionVariable(cond);
    return 0;
}


int pthread_cond_timedwait(pthread_cond_t* cond, pthread_mutex_t* mutex, struct timespec* abstime)
{
    DWORD timeout = get_milliseconds(abstime);
    if (!SleepConditionVariableCS(cond, mutex, timeout))
        return ETIMEDOUT;
    return 0;
}


int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex)
{
    return pthread_cond_timedwait(cond, mutex, NULL);
}


int pthread_attr_init(pthread_attr_t* connect_att)
{
    connect_att->dwStackSize = 0;
    connect_att->dwCreatingFlag = 0;
    return 0;
}


int pthread_attr_setstacksize(pthread_attr_t* connect_att, DWORD stack)
{
    connect_att->dwStackSize = stack;
    return 0;
}


int pthread_attr_getstacksize(pthread_attr_t* connect_att, size_t* stack)
{
    *stack= (size_t)connect_att->dwStackSize;
    return 0;
}


int pthread_attr_destroy(pthread_attr_t* connect_att)
{
    memset(connect_att, 0, sizeof(*connect_att));
    return 0;
}


int pthread_dummy(int ret)
{
    return ret;
}


/****************************************************************************
** Replacements for localtime_r and gmtime_r
****************************************************************************/

struct tm* localtime_r(const time_t* timep, struct tm* tmp)
{
    localtime_s(tmp, timep);
    return tmp;
}


struct tm* gmtime_r(const time_t* clock, struct tm* res)
{
    gmtime_s(res, clock);
    return res;
}

static void install_sigabrt_handler(void);

struct thread_start_parameter
{
    pthread_handler func;
    void *arg;
};

/**
   Adapter to @c pthread_mutex_trylock()

   @retval 0      Mutex was acquired
   @retval EBUSY  Mutex was already locked by a thread
 */
int win_pthread_mutex_trylock(pthread_mutex_t* mutex)
{
    if(TryEnterCriticalSection(mutex))
    {
        /* Don't allow recursive lock */
        if (mutex->RecursionCount > 1)
		{
            LeaveCriticalSection(mutex);
            return EBUSY;
        }
        return 0;
    }
    return EBUSY;
}

static unsigned int __stdcall pthread_start(void* param)
{
    struct thread_start_parameter* parameter = (struct thread_start_parameter*)param;
    pthread_handler func = parameter->func;
    void* arg = parameter->arg;
    free(param);
    (*func)(arg);
    return 0;
}


/**
  Create thread.
  This function provides combined implementation for
  pthread_create() and pthread_create_get_handle() functions.

  @param thread_id    reference to pthread object
  @param attr         reference to pthread attribute
  @param func         pthread handler function
  @param param        parameters to pass to newly created thread
  @param out_handle   reference to thread handle. This needs to be passed to
                      pthread_join_with_handle() function when it is joined
  @return int
    @retval 0 success
    @retval 1 failure

*/
int pthread_create_base(pthread_t* thread_id, const pthread_attr_t* attr, pthread_handler func, void* param, HANDLE* out_handle)
{
    HANDLE handle= NULL;
    struct thread_start_parameter* parameter;
    unsigned int stack_size;
    
    parameter = (struct thread_start_parameter*)malloc(sizeof(*parameter));
    if(!parameter)
        goto error_return;    
    parameter->func = func;
    parameter->arg = param;
    stack_size = attr?attr->dwStackSize:0;    
    handle= (HANDLE)_beginthreadex(NULL, stack_size, pthread_start, parameter, 0, thread_id);
    if(!handle)
    {
        my_osmaperr(GetLastError());
        free(parameter);
        goto error_return;
    }
    if(!out_handle)
        /* Do not need thread handle, close it */
        CloseHandle(handle);
    else
        /* Save thread handle, it will be used later during join */
        *out_handle= handle;
    return 0;
	
error_return:
    return 1;
}

int pthread_create(pthread_t* thread_id, const pthread_attr_t* attr, pthread_handler func, void* param)
{
    return pthread_create_base(thread_id, attr, func, param, NULL);
}

/*
  Existing mysql_thread_create does not work well in windows platform
  when threads are joined because
  A)during thread creation thread handle is not stored.
  B)during thread join, thread handle is retrieved using OpenThread().
    OpenThread() does not behave properly when thread to be joined is already
    exited.

  Due to the above issue, pthread_create_get_handle() and
  pthread_join_with_handle() needs to be used
  This function returns the handle in output parameter out_handle.
  Caller is expected to store this value and use it during
  pthread_join_with_handle() function call.
*/
int pthread_create_get_handle(pthread_t* thread_id,
                              const pthread_attr_t* attr,
                              pthread_handler func, void* param,
                              HANDLE* out_handle)
{
    return pthread_create_base(thread_id, attr, func, param, out_handle);
}

void pthread_exit(void* a)
{
    _endthreadex(0);
}

/**
  Join thread.
  This function provides combined implementation for
  pthread_join() and pthread_join_with_handle() functions.

  @param thread      reference to pthread object
  @param handle      thread handle of thread to be joined
  @return int
    @retval 0 success
    @retval 1 failure

*/
int pthread_join_base(pthread_t thread, HANDLE handle)
{
    DWORD ret;
    if(!handle)
    {
        handle = OpenThread(SYNCHRONIZE, FALSE, thread);
        if(!handle)
        {
            my_osmaperr(GetLastError());
            goto error_return;
        }
    }
    ret = WaitForSingleObject(handle, INFINITE);
    if(ret != WAIT_OBJECT_0)
    {
        my_osmaperr(GetLastError());
        goto error_return;
    }
    CloseHandle(handle);
    return 0;

error_return:
    if(handle)
        CloseHandle(handle);
    return 1;
}

/*
  This function is unsafe in windows. Use pthread_create_get_handle()
  and pthread_join_with_handle() instead.
  During thread join, thread handle is retrieved using OpenThread().
  OpenThread() does not behave properly when thread to be joined is already
  exited.
*/
int pthread_join(pthread_t thread, void** value_ptr)
{
    return pthread_join_base(thread, NULL);
}

int pthread_join_with_handle(HANDLE handle)
{
    pthread_t dummy = 0;
    return pthread_join_base(dummy, handle);
}

int pthread_cancel(pthread_t thread)
{
    HANDLE handle = 0;
    bool ok = FALSE;
    
    handle = OpenThread(THREAD_TERMINATE, FALSE, thread);
    if(handle)
    {
        ok = TerminateThread(handle,0);
        CloseHandle(handle);
    }
    if(ok)
        return 0;    
    errno= EINVAL;
	
    return -1;
}

/*
 One time initialization. For simplicity, we assume initializer thread
 does not exit within init_routine().
*/
int my_pthread_once(my_pthread_once_t* once_control, void (*init_routine)(void))
{
    long state;

    /*
      Do "dirty" read to find out if initialization is already done, to
      save an interlocked operation in common case. Memory barriers are ensured by 
      Visual C++ volatile implementation.
    */
    if (*once_control == MY_PTHREAD_ONCE_DONE)
      return 0;    
    state= InterlockedCompareExchange(once_control, MY_PTHREAD_ONCE_INPROGRESS, MY_PTHREAD_ONCE_INIT);    
    switch(state)
    {
        case MY_PTHREAD_ONCE_INIT:
            /* This is initializer thread */
            (*init_routine)();
            *once_control = MY_PTHREAD_ONCE_DONE;
            break;        
        case MY_PTHREAD_ONCE_INPROGRESS:
            /* init_routine in progress. Wait for its completion */
            while(*once_control == MY_PTHREAD_ONCE_INPROGRESS)
            {
              Sleep(1);
            }
            break;
        case MY_PTHREAD_ONCE_DONE:
            /* Nothing to do*/
            break;
    }
	
    return 0;
}

#endif /* _WIN32* /
