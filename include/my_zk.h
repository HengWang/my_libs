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
 
#ifndef __MY_ZK_H
#define __MY_ZK_H

#include "my_global_exports.h"
#include "zookeeper.h"

#define DEFAULT_ZK_TIMEOUT   10000
#define DEFAULT_ZK_FLAG      0
#define DEFAULT_ZK_CONTEXT   "My zk default context."
#define DEFAULT_ZK_CLIENTID  0

MY_GLOBAL_API int zk_init(const char* __host);
MY_GLOBAL_API int zk_uninit(zhandle_t* __handler);

MY_GLOBAL_API int zk_set_debug_level(ZooLogLevel __level);
My_GLOBAL_API int zk_set_log_stream(FILE* __log);
MY_GLOBAL_API int zk_set_timeout(int __timeout);
MY_GLOBAL_API int zk_set_context(zhandle_t* __handler, void* __context);
MY_GLOBAL_API void* zk_get_context(zhandle_t* __handler);
MY_GLOBAL_API int zk_set_clientid(zhandle_t* __handler, const clientid_t* __id);
MY_GLOBAL_API clientid_t* zk_get_clientid(zhandle_t* __handler);

MY_GLOBAL_API int zk_create(zhandle_t* __handler, const char* __path, const char* __value);
MY_GLOBAL_API int zk_acreate(zhandle_t* __handler, const char* __path, const char* __value);
MY_GLOBAL_API int zk_delete(zhandle_t* __handler, const char* __path, const char* __value);
MY_GLOBAL_API int zk_adelete(zhandle_t* __handler, const char* __path, const char* __value);
MY_GLOBAL_API int zk_get(zhandle_t* __handler, const char* __path, char* __value);
MY_GLOBAL_API int zk_aget(zhandle_t* __handler, const char* __path, char* __value);
MY_GLOBAL_API int zk_get_children(zhandle_t* __handler, const char* __path, const char* __value);
MY_GLOBAL_API int zk_aget_children(zhandle_t* __handler, const char* __path, const char* __value);
MY_GLOBAL_API int zk_set(zhandle_t* __handler, const char* __path, const char* __value);
MY_GLOBAL_API int zk_aset(zhandle_t* __handler, const char* __path, const char* __value);

#endif  //__MY_ZK_H