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
 
#ifndef __MY_PLUGIN_H
#define __MY_PLUGIN_H

#include "my_global_exports.h"

/* The following strings to define licenses for plugins. */
#define PLUGIN_LICENSE_PROPRIETARY 0
#define PLUGIN_LICENSE_GPL 1
#define PLUGIN_LICENSE_BSD 2

#define PLUGIN_LICENSE_PROPRIETARY_STRING "PROPRIETARY"
#define PLUGIN_LICENSE_GPL_STRING "GPL"
#define PLUGIN_LICENSE_BSD_STRING "BSD"

struct my_plugin_t
{
  int type;             /* the plugin type (a MYSQL_XXX_PLUGIN value)     */
  void *events;         /* pointer to type-specific plugin descriptor     */
  const char *name;     /* plugin name                                    */
  const char *author;   /* plugin author                                  */
  const char *descr;    /* general descriptive text                       */
  int license;          /* the plugin license                             */
  int (*init)(void*);   /* the function to invoke when plugin is loaded   */
  int (*uninit)(void*); /* the function to invoke when plugin is unloaded */
  unsigned int version; /* plugin version                                 */
  void * reserved;      /* reserved for dependency checking               */
  unsigned long flags;  /* flags for plugin                               */
};

typedef struct my_plugin_t my_plugin;

MY_GLOBAL_API my_plugin* plugin_init();

MY_GLOBAL_API void plugin_uninit(my_plugin* __plugin);

MY_GLOBAL_API int plugin_install(const char* __name);

MY_GLOBAL_API int plugin_uninstall(const char* __name);

MY_GLOBAL_API int plugin_lock(const char* __name);

MY_GLOBAL_API int plugin_unlock(const char* __name);



#endif  //__MY_PLUGIN_H