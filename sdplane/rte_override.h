#ifndef __RTE_OVERRIDE_H__
#define __RTE_OVERRIDE_H__

#include <rte_version.h>

/* override rte_exit() so that the whole process is not broken. */
#define rte_exit(x, ...)                                                      \
  do                                                                          \
    {                                                                         \
      printf (__VA_ARGS__);                                                   \
    }                                                                         \
  while (0)
#define rte_warn(x, ...)                                                      \
  do                                                                          \
    {                                                                         \
      printf (__VA_ARGS__);                                                   \
    }                                                                         \
  while (0)

#if RTE_VERSION < RTE_VERSION_NUM(24,0,0,0)
  #define RTE_IPV6_ADDR_SIZE 16
  #ifndef RTE_VLAN_TCI_ID
  #define RTE_VLAN_TCI_ID(vlan_tci) ((vlan_tci) & 0x0fff)
  #define RTE_VLAN_TCI_PRI(vlan_tci) \
    (((vlan_tci) & 0xe000) >> 13)
  #define RTE_VLAN_TCI_DEI(vlan_tci) \
    (((vlan_tci) & 0x1000) >> 12)
  #endif
#endif

#if RTE_VERSION >= RTE_VERSION_NUM(25,0,0,0)
  #define __rte_packed_begin
  #define __rte_packed_end __rte_packed
#endif

#if RTE_VERSION >= RTE_VERSION_NUM(24,11,0,0)
  #define IPV6_ADDR_BYTES(x) ((x).a)
#else
  #define IPV6_ADDR_BYTES(x) (x)
#endif

#endif /*__RTE_OVERRIDE_H__*/
