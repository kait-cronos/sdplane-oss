#ifndef __RIB_MANAGER_H__
#define __RIB_MANAGER_H__

#include "queue_config.h"

struct rib {
  struct sdplane_queue_conf qconf[RTE_MAX_LCORE];
};

#endif /*__RIB_MANAGER_H__*/
