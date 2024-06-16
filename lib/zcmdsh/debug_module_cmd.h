/*
 * Copyright (C) 2024 Yasuhiro Ohara. All rights reserved.
 */

#ifndef __DEBUG_MODULE_CMD_H__
#define __DEBUG_MODULE_CMD_H__

struct debug_module_type
{
  char *module_name;
  int debug_type_size;
  struct debug_type *debug_types;
};

extern struct debug_module_type debug_module_slots[DEBUG_MODULE_MAX];

EXTERN_COMMAND3 (debug_module);
EXTERN_COMMAND (show_debug_module);

int debug_module_get_size ();
void
debug_module_register (int module, char *name, int debug_type_size,
                       struct debug_type *debug_types);
void debug_module_cmd_init ();


#endif /*__DEBUG_MODULE_CMD_H__*/
