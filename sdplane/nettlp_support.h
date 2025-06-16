#ifndef __NETTLP_SUPPORT_H__
#define __NETTLP_SUPPORT_H__

#include "tlp.h"

/*
 * NetTLP specific header
 */
struct nettlp_hdr
{
  uint16_t seq;
  uint32_t tstamp;
} __attribute__ ((packed));

#endif /*__NETTLP_SUPPORT_H__*/
