// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2023-2026 Yasuhiro Ohara

#ifndef __NETLINK_SEG6_H__
#define __NETLINK_SEG6_H__

#include <netinet/in.h>

#include <linux/lwtunnel.h>
#include <linux/seg6.h>
#include <linux/seg6_local.h>
#include <linux/seg6_iptunnel.h>

struct seg6_param
{
  struct seg6_iptunnel_encap *seg6_tuninfo;
};

struct seg6local_param
{
  uint32_t action;
  uint32_t table_id;
  uint32_t vrf_id;
  struct in_addr nh4;
  struct in6_addr nh6;
  int iif;
  int oif;
  struct ipv6_sr_hdr *srh;
};

#endif /*__NETLINK_SEG6_H__*/