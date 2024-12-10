#ifndef __SOFT_DPLANE_H__
#define __SOFT_DPLANE_H__

#include <zcmdsh/shell.h>

#define L3FWD_ARGV_MAX 16
extern char *l3fwd_argv[L3FWD_ARGV_MAX];
extern int l3fwd_argc;

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

int lthread_main (__rte_unused void *dummy);

void sdplane_cmd_init (struct command_set *cmdset);
void sdplane_init ();

#endif /*__SOFT_DPLANE_H__*/
