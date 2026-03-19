#ifndef __DEBUG_SDPLANE_H__
#define __DEBUG_SDPLANE_H__

#include <stdint.h>
#include <sdplane/command.h>
#include <sdplane/debug_log.h>

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
#define DEBUG_SDPLANE_DHCP_SERVER        (1ULL << 33)
#define DEBUG_SDPLANE_L2_SWITCH          (1ULL << 34)
#define DEBUG_SDPLANE_ROUTE_ENTRY        (1ULL << 35)
#define DEBUG_SDPLANE_ROUTER             (1ULL << 36)
#define DEBUG_SDPLANE_ENHANCED_INFO      (1ULL << 37)
#define DEBUG_SDPLANE_WARNING            (1ULL << 38)
#define DEBUG_SDPLANE_ERROR              (1ULL << 39)
#define DEBUG_SDPLANE_NETLINK_HOOK       (1ULL << 40)
#define DEBUG_SDPLANE_SIGNAL             (1ULL << 41)
#define DEBUG_SDPLANE_NEXTHOP            (1ULL << 42)

#define DEBUG_DOMAIN_OF_LTHREAD            SDPLANE
#define DEBUG_DOMAIN_OF_CONSOLE            SDPLANE
#define DEBUG_DOMAIN_OF_TAPHANDLER         SDPLANE
#define DEBUG_DOMAIN_OF_L2FWD              SDPLANE
#define DEBUG_DOMAIN_OF_L3FWD              SDPLANE
#define DEBUG_DOMAIN_OF_VTY_SERVER         SDPLANE
#define DEBUG_DOMAIN_OF_VTY_SHELL          SDPLANE
#define DEBUG_DOMAIN_OF_TELNET_OPT         SDPLANE
#define DEBUG_DOMAIN_OF_STAT_COLLECTOR     SDPLANE
#define DEBUG_DOMAIN_OF_SCHED              SDPLANE
#define DEBUG_DOMAIN_OF_VTY                SDPLANE
#define DEBUG_DOMAIN_OF_PACKET             SDPLANE
#define DEBUG_DOMAIN_OF_FDB                SDPLANE
#define DEBUG_DOMAIN_OF_FDB_CHANGE         SDPLANE
#define DEBUG_DOMAIN_OF_RCU_READ           SDPLANE
#define DEBUG_DOMAIN_OF_RCU_WRITE          SDPLANE
#define DEBUG_DOMAIN_OF_L2_REPEATER        SDPLANE
#define DEBUG_DOMAIN_OF_THREAD             SDPLANE
#define DEBUG_DOMAIN_OF_RIB                SDPLANE
#define DEBUG_DOMAIN_OF_VSWITCH            SDPLANE
#define DEBUG_DOMAIN_OF_RIB_MESG           SDPLANE
#define DEBUG_DOMAIN_OF_RIB_CHECK          SDPLANE
#define DEBUG_DOMAIN_OF_IMESSAGE           SDPLANE
#define DEBUG_DOMAIN_OF_NETTLP             SDPLANE
#define DEBUG_DOMAIN_OF_NETDEVICE          SDPLANE
#define DEBUG_DOMAIN_OF_NETLINK            SDPLANE
#define DEBUG_DOMAIN_OF_VLAN_SWITCH        SDPLANE
#define DEBUG_DOMAIN_OF_PKTGEN             SDPLANE
#define DEBUG_DOMAIN_OF_LINKFLAP_GENERATOR SDPLANE
#define DEBUG_DOMAIN_OF_STARTUP_CONFIG     SDPLANE
#define DEBUG_DOMAIN_OF_ENHANCED_REPEATER  SDPLANE
#define DEBUG_DOMAIN_OF_NEIGH              SDPLANE
#define DEBUG_DOMAIN_OF_DHCP_SERVER        SDPLANE
#define DEBUG_DOMAIN_OF_L2_SWITCH          SDPLANE
#define DEBUG_DOMAIN_OF_ROUTE_ENTRY        SDPLANE
#define DEBUG_DOMAIN_OF_ROUTER             SDPLANE
#define DEBUG_DOMAIN_OF_ENHANCED_INFO      SDPLANE
#define DEBUG_DOMAIN_OF_WARNING            SDPLANE
#define DEBUG_DOMAIN_OF_ERROR              SDPLANE
#define DEBUG_DOMAIN_OF_NETLINK_HOOK       SDPLANE
#define DEBUG_DOMAIN_OF_SIGNAL             SDPLANE
#define DEBUG_DOMAIN_OF_NEXTHOP            SDPLANE

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

#define DEBUG_CHECK(cate, type) \
  (FLAG_CHECK (DEBUG_CONFIG (cate), DEBUG_TYPE(cate, type)))

#ifndef IS_DEBUG
#define IS_DEBUG(type) DEBUG_CHECK(SDPLANE, type)
#endif

#ifndef DEBUG_NEW
#define DEBUG_NEW(type, format, ...) \
    DEBUG_LOG(SDPLANE, type, format, ##__VA_ARGS__)
#endif

#ifndef ERROR_MSG
#define ERROR_MSG(format, ...) \
    DEBUG_LOG(SDPLANE, ERROR, format, ##__VA_ARGS__)
#endif

#ifndef WARNING
#define WARNING(format, ...) \
    DEBUG_LOG(SDPLANE, WARNING, format, ##__VA_ARGS__)
#endif

EXTERN_COMMAND (debug_sdplane);
EXTERN_COMMAND (show_debug_sdplane);
void debug_sdplane_cmd_init ();

#endif /*__DEBUG_SDPLANE_H__*/
