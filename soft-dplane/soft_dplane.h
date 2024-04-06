#ifndef __SOFT_DPLANE_H__
#define __SOFT_DPLANE_H__

struct lcore_worker
{
  lcore_function_t *func;
  void *arg;
  char *func_name;
};
extern struct lcore_worker lcore_workers[RTE_MAX_LCORE];

//#define SHOW_HELP "show information\n"
#define CLEAR_HELP "clear information\n"
#define SET_HELP "set information\n"
#define RESET_HELP "reset information\n"
#define START_HELP "start information\n"
#define STOP_HELP "stop information\n"
#define RESTART_HELP "restart information\n"
#define WORKER_HELP "worker information\n"
#define LCORE_HELP "lcore information\n"
#define LCORE_NUMBER_HELP "specify lcore number\n"
#define LCORE_ALL_HELP "do for all lcores\n"

#define PORT_HELP "port information\n"
#define PORT_NUMBER_HELP "specify port number\n"
#define PORT_ALL_HELP "do for all ports\n"

#define ALL_HELP "all variables\n"
#define VARS_HELP "all variables\n"

void soft_dplane_cmd_init (struct command_set *cmdset);

#endif /*__SOFT_DPLANE_H__*/
