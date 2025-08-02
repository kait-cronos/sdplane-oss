#ifndef __SOFT_DPLANE_H__
#define __SOFT_DPLANE_H__

#include <rte_ether.h>
#include <rte_ethdev.h>
#include <sdplane/shell.h>

extern char *pid_path;

#define L3FWD_ARGV_MAX 16
extern char *l3fwd_argv[L3FWD_ARGV_MAX];
extern int l3fwd_argc;

#define ARGV_LIST_MAX      8
#define ARGV_LIST_ARGV_MAX 32
extern char *argv_list[ARGV_LIST_MAX][ARGV_LIST_ARGV_MAX];
extern int argv_list_argc[ARGV_LIST_MAX];

extern volatile bool force_quit;
extern volatile bool force_stop[RTE_MAX_LCORE];

struct lcore_worker
{
  lcore_function_t *func;
  void *arg;
  char *func_name;
};
extern struct lcore_worker lcore_workers[RTE_MAX_LCORE];

#define VSWITCH_PORT_TYPE_NONE       0
#define VSWITCH_PORT_TYPE_DPDK_LCORE 1
#define VSWITCH_PORT_TYPE_LINUX_TAP  2

#define TAPDIR_UP   0
#define TAPDIR_DOWN 1
#define TAPDIR_SIZE 2

struct vswitch_port
{
  uint8_t id;
  uint8_t type;
  char *name;
  int sockfd;
  int lcore_id;
  int dpdk_port_id;
  int dpdk_queue_id;
  struct rte_ring *ring[TAPDIR_SIZE];
};

#define VSWITCH_PORT_SIZE 16
struct vswitch
{
  int limit;
  int size;
  struct vswitch_port port[VSWITCH_PORT_SIZE];
};
extern struct vswitch vswitch0;

#define FDB_SIZE 16
struct fdb_entry
{
  struct rte_ether_addr l2addr;
  int port;
};
extern struct fdb_entry fdb[FDB_SIZE];

// #define SHOW_HELP "show information\n"
#define CLEAR_HELP        "clear information\n"
#define SET_HELP          "set information\n"
#define DELETE_HELP       "delete information\n"
#define RESET_HELP        "reset information\n"
#define START_HELP        "start information\n"
#define STOP_HELP         "stop information\n"
#define RESTART_HELP      "restart information\n"
#define WORKER_HELP       "worker information\n"
#define LCORE_HELP        "lcore information\n"
#define LCORE_NUMBER_HELP "specify lcore number\n"
#define LCORE_ALL_HELP    "do for all lcores\n"

#define PORT_HELP        "port information\n"
#define PORT_NUMBER_HELP "specify port number\n"
#define PORT_ALL_HELP    "do for all ports\n"

#define QUEUE_HELP        "queue information\n"
#define QUEUE_NUMBER_HELP "specify queue number\n"

#define ALL_HELP  "all variables\n"
#define VARS_HELP "all variables\n"

#define ENABLE_HELP  "enable feature\n"
#define DISABLE_HELP "disable feature\n"

void start_lcore (struct shell *shell, int lcore_id);
void stop_lcore (struct shell *shell, int lcore_id);

EXTERN_COMMAND (set_worker);
EXTERN_COMMAND (start_stop_worker);
EXTERN_COMMAND (show_worker);

EXTERN_COMMAND (set_locale);

EXTERN_COMMAND (start_stop_port);
EXTERN_COMMAND (show_port);
EXTERN_COMMAND (show_port_statistics);
EXTERN_COMMAND (set_port_promiscuous);
EXTERN_COMMAND (show_port_promiscuous);
EXTERN_COMMAND (show_port_flowcontrol);
EXTERN_COMMAND (set_port_flowcontrol);

void lthread_main (__rte_unused void *dummy);

void sdplane_cmd_init (struct command_set *cmdset);
void sdplane_init ();

extern struct rte_ring *msg_queue_rib;
extern struct rte_ring *msg_queue_neigh;

#include <rte_ethdev.h>
#include "queue_config.h"

struct stream_msg_header
{
  uint16_t type;
  uint16_t length; // not including the header size.
};

#define STREAM_MSG_TYPE_NONE     0
#define STREAM_MSG_TYPE_QCONF    1
#define STREAM_MSG_TYPE_ETH_LINK 2

struct stream_msg_eth_link
{
  struct rte_eth_link link[RTE_MAX_ETHPORTS];
};

struct stream_msg_qconf
{
  struct sdplane_queue_conf qconf[RTE_MAX_LCORE];
};

#endif /*__SOFT_DPLANE_H__*/
