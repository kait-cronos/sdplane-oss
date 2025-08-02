#ifndef __THREAD_INFO_H__
#define __THREAD_INFO_H__

#include <lthread.h>

struct thread_info
{
  int lcore_id;
  lthread_t *lthread;
  lthread_func func;
  char *name;
  void *arg;
};

struct thread_counter
{
  uint64_t *loop_counter_ptr;
  uint64_t current;
  uint64_t prev;
  uint64_t persec;
};

#define THREAD_INFO_LIMIT 16
extern struct thread_info threads[];
extern rte_rwlock_t thread_info_lock;
extern uint8_t thread_info_size;

extern struct thread_counter thread_counters[];

int thread_register (int lcore_id, lthread_t *ltread, lthread_func func,
                     char *name, void *arg);
void thread_unregister (int thread_id);
int thread_update (int thread_id, int lcore_id, lthread_t *ltread,
                   lthread_func func, char *name, void *arg);

int thread_register_loop_counter (int thread_id, uint64_t *ptr);
int thread_unregister_loop_counter (int thread_id);

int thread_lookup (void *func);
int thread_lookup_by_lcore (void *func, int lcore);

EXTERN_COMMAND (show_thread_cmd);

int thread_info_cmd_init (struct command_set *cmdset);
int thread_info_init ();

void lthread_cancel_all ();
void lthread_join_all ();

#endif /*__THREAD_INFO_H__*/
