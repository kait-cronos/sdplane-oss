/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef __DEBUG_SDPLANE_H__
#define __DEBUG_SDPLANE_H__

#include <stdint.h>

#define DEBUG_SDPLANE_LTHREAD            (1ULL << 0)
#define DEBUG_SDPLANE_CONSOLE            (1ULL << 1)
#define DEBUG_SDPLANE_TAPHANDLER         (1ULL << 2)
#define DEBUG_SDPLANE_L2FWD              (1ULL << 3)
#define DEBUG_SDPLANE_L3FWD              (1ULL << 4)
#define DEBUG_SDPLANE_VTY_SERVER         (1ULL << 5)
#define DEBUG_SDPLANE_VTY_SHELL          (1ULL << 6)
#define DEBUG_SDPLANE_TELNET_OPT         (1ULL << 7)
#define DEBUG_SDPLANE_STAT_COLLECTOR     (1ULL << 8)
#define DEBUG_SDPLANE_SCHED              (1ULL << 9)
#define DEBUG_SDPLANE_VTY                (1ULL << 10)
#define DEBUG_SDPLANE_PACKET             (1ULL << 11)
#define DEBUG_SDPLANE_FDB                (1ULL << 12)
#define DEBUG_SDPLANE_FDB_CHANGE         (1ULL << 13)
#define DEBUG_SDPLANE_RCU_READ           (1ULL << 14)
#define DEBUG_SDPLANE_RCU_WRITE          (1ULL << 15)
#define DEBUG_SDPLANE_L2_REPEATER        (1ULL << 16)
#define DEBUG_SDPLANE_THREAD             (1ULL << 17)
#define DEBUG_SDPLANE_RIB                (1ULL << 18)
#define DEBUG_SDPLANE_VSWITCH            (1ULL << 19)
#define DEBUG_SDPLANE_RIB_MESG           (1ULL << 21)
#define DEBUG_SDPLANE_RIB_CHECK          (1ULL << 22)
#define DEBUG_SDPLANE_IMESSAGE           (1ULL << 23)
#define DEBUG_SDPLANE_NETTLP             (1ULL << 24)
#define DEBUG_SDPLANE_NETDEVICE          (1ULL << 25)
#define DEBUG_SDPLANE_NETLINK            (1ULL << 26)
#define DEBUG_SDPLANE_VLAN_SWITCH        (1ULL << 27)
#define DEBUG_SDPLANE_PKTGEN             (1ULL << 28)
#define DEBUG_SDPLANE_LINKFLAP_GENERATOR (1ULL << 29)
#define DEBUG_SDPLANE_STARTUP_CONFIG     (1ULL << 30)
#define DEBUG_SDPLANE_ENHANCED_REPEATER  (1ULL << 31)
#define DEBUG_SDPLANE_NEIGH              (1ULL << 32)

#define DEBUG_SDPLANE_LOG(type, format, ...)                                  \
  DEBUG_LOG (SDPLANE, type, format, ##__VA_ARGS__)

#define DEBUG_LOG_FLAG(cate, flag, format, ...)                               \
  do                                                                          \
    {                                                                         \
      if (FLAG_CHECK (DEBUG_CONFIG (cate), (flag)))                           \
        DEBUG_LOG_MSG (format, ##__VA_ARGS__);                                \
    }                                                                         \
  while (0)

#define DEBUG_SDPLANE_FLAG(flag, format, ...)                                 \
  DEBUG_LOG_FLAG (SDPLANE, flag, format, ##__VA_ARGS__)

EXTERN_COMMAND (debug_sdplane);
EXTERN_COMMAND (show_debug_sdplane);
void debug_sdplane_cmd_init ();

#endif /*__DEBUG_SDPLANE_H__*/
