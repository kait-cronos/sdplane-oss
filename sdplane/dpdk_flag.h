#ifndef __DPDK_FLAG_H__
#define __DPDK_FLAG_H__

struct flag_name
{
  char *name;
  uint64_t val;
};

extern struct flag_name link_speeds[];
extern struct flag_name rx_offload_capa[];
extern struct flag_name tx_offload_capa[];

int
snprintf_flags (char *buf, int size, uint64_t flags,
                struct flag_name *flag_names,
                char *delim, int flag_names_size);

#endif /*__DPDK_FLAG_H__*/
