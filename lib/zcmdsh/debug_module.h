/*
 * Copyright (C) 2024 Yasuhiro Ohara. All rights reserved.
 */

#ifndef __DEBUG_MODULE_H__
#define __DEBUG_MODULE_H__

#include <stdint.h>

#include <zcmdsh/flag.h>
#if 1
#include <zcmdsh/debug.h>
#else
struct debug_type
{
  uint64_t flag;
  const char *name;
};
#endif

#define DEBUG_MODULE_MAX 32

extern uint64_t debug_module_config[DEBUG_MODULE_MAX];

//if (FLAG_CHECK (debug_module_config[debug_module], DEBUG_MODULE_SUBCATEGORY))
//if (FLAG_CHECK (debug_module_config[debug_sdplane], DEBUG_SDPLANE_LTHREAD))
//if (FLAG_CHECK (debug_module_config[debug_lthread], DEBUG_LTHREAD_CTXSWITCH))

#endif /*__DEBUG_MODULE_H__*/
