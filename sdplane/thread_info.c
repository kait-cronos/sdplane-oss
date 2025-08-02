#include "include.h"

#include <lthread.h>

#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

#include <rte_rwlock.h>

#include "thread_info.h"

rte_rwlock_t thread_info_lock;

uint8_t thread_info_size = 0;
struct thread_info threads[THREAD_INFO_LIMIT];

struct thread_counter thread_counters[THREAD_INFO_LIMIT];

int
thread_register (int lcore_id, lthread_t *lthread, lthread_func func,
                 char *name, void *arg)
{
  int thread_id;
  struct thread_info *tinfo;
  int i;

  rte_rwlock_write_lock (&thread_info_lock);

  for (i = 0; i < thread_info_size; i++)
    {
      tinfo = &threads[i];
      if (tinfo->func == NULL)
        break;
    }

  if (i < thread_info_size)
    thread_id = i;
  else if (thread_info_size < THREAD_INFO_LIMIT)
    thread_id = thread_info_size++;
  else
    thread_id = -1;

  if (thread_id >= 0)
    {
      tinfo = &threads[thread_id];
      tinfo->lcore_id = lcore_id;
      tinfo->lthread = lthread;
      tinfo->func = func;
      tinfo->name = strdup (name);
      tinfo->arg = arg;
    }

  rte_rwlock_write_unlock (&thread_info_lock);

  return thread_id;
}

void
thread_unregister (int thread_id)
{
  struct thread_info *tinfo;

  rte_rwlock_write_lock (&thread_info_lock);

  tinfo = &threads[thread_id];
  tinfo->lcore_id = -1;
  tinfo->lthread = NULL;
  tinfo->func = NULL;
  if (tinfo->name)
    free (tinfo->name);
  tinfo->name = NULL;
  tinfo->arg = NULL;

  rte_rwlock_write_unlock (&thread_info_lock);
}

int
thread_update (int thread_id, int lcore_id, lthread_t *lthread,
               lthread_func func, char *name, void *arg)
{
  struct thread_info *tinfo;
  int i;

  if (thread_id < 0 || thread_info_size <= thread_id)
    return -1;

  rte_rwlock_write_lock (&thread_info_lock);

  if (thread_id >= 0)
    {
      tinfo = &threads[thread_id];
      tinfo->lcore_id = lcore_id;
      tinfo->lthread = lthread;
      tinfo->func = func;
      tinfo->name = strdup (name);
      tinfo->arg = arg;
    }

  rte_rwlock_write_unlock (&thread_info_lock);

  return thread_id;
}

int
thread_register_loop_counter (int thread_id, uint64_t *ptr)
{
  struct thread_info *tinfo;
  struct thread_counter *tcounter;
  if (thread_id < 0 || THREAD_INFO_LIMIT <= thread_id)
    return -1;
  tinfo = &threads[thread_id];
  tcounter = &thread_counters[thread_id];
  tcounter->loop_counter_ptr = ptr;
}

int
thread_unregister_loop_counter (int thread_id)
{
  struct thread_info *tinfo;
  struct thread_counter *tcounter;
  if (thread_id < 0 || THREAD_INFO_LIMIT <= thread_id)
    return -1;
  tinfo = &threads[thread_id];
  tcounter = &thread_counters[thread_id];
  tcounter->loop_counter_ptr = NULL;
}

int
thread_lookup (void *func)
{
  struct thread_info *tinfo;
  int i;
  int ret = -1;

  rte_rwlock_read_lock (&thread_info_lock);
  for (i = 0; i < thread_info_size; i++)
    {
      tinfo = &threads[i];
      if (tinfo->func == func)
        ret = i;
    }
  rte_rwlock_read_unlock (&thread_info_lock);

  return ret;
}

int
thread_lookup_by_lcore (void *func, int lcore)
{
  struct thread_info *tinfo;
  int i;
  int ret = -1;

  rte_rwlock_read_lock (&thread_info_lock);
  for (i = 0; i < thread_info_size; i++)
    {
      tinfo = &threads[i];
      if (tinfo->func == func && tinfo->lcore_id == lcore)
        ret = i;
    }
  rte_rwlock_read_unlock (&thread_info_lock);

  return ret;
}

CLI_COMMAND2 (show_thread_cmd, "show thread", SHOW_HELP,
              "thread information\n")
{
  struct shell *shell = (struct shell *) context;
  struct thread_info *tinfo;
  struct thread_counter *tc;
  int i;
  char buf[256];

  rte_rwlock_read_lock (&thread_info_lock);
  fprintf (shell->terminal, "thread size: %d%s", thread_info_size, shell->NL);
  fprintf (shell->terminal, "%-4s %-4s %-7s %-18s %12s %18s%s",
           "id", "core", "type", "func", "loop/sec", "#loops", shell->NL);
  for (i = 0; i < thread_info_size; i++)
    {
      tinfo = &threads[i];
      tc = &thread_counters[i];
      snprintf (buf, sizeof (buf), "%-4d %-4d %-7s %-18s %'12lu %'18lu", i,
                tinfo->lcore_id, (tinfo->lthread ? "lthread" : "dpdk"),
                tinfo->name, tc->persec, tc->current);
      fprintf (shell->terminal, "%s%s", buf, shell->NL);
    }
  rte_rwlock_read_unlock (&thread_info_lock);
  return 0;
}

CLI_COMMAND2 (show_thread_counter, "show thread counter", SHOW_HELP,
              "thread information.\n", "counter information.\n")
{
  struct shell *shell = (struct shell *) context;
  struct thread_info *tinfo;
  struct thread_counter *tc;
  int i;
  char buf[256];

  rte_rwlock_read_lock (&thread_info_lock);
  fprintf (shell->terminal, "thread size: %d%s", thread_info_size, shell->NL);
  fprintf (shell->terminal, "%-4s %-4s %-7s %-16s %-10s %-18s%s", "id", "core",
           "type", "func", "loop/sec", "#loops", shell->NL);
  for (i = 0; i < thread_info_size; i++)
    {
      tinfo = &threads[i];
      tc = &thread_counters[i];
      snprintf (buf, sizeof (buf), "%-4d %-4d %-7s %-16s %'10lu %'18lu", i,
                tinfo->lcore_id, (tinfo->lthread ? "lthread" : "dpdk"),
                tinfo->name, tc->persec, tc->current);
      fprintf (shell->terminal, "%s%s", buf, shell->NL);
    }
  rte_rwlock_read_unlock (&thread_info_lock);
  return 0;
}

int
thread_info_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, show_thread_cmd);
  INSTALL_COMMAND2 (cmdset, show_thread_counter);
}

int
thread_info_init ()
{
  memset (threads, 0, sizeof (threads));
  rte_rwlock_init (&thread_info_lock);
}

void
lthread_cancel_all ()
{
  int i;
  struct thread_info *tinfo;
  rte_rwlock_read_lock (&thread_info_lock);
  for (i = 0; i < thread_info_size; i++)
    {
      tinfo = &threads[i];
      if (tinfo->lthread)
        lthread_cancel (tinfo->lthread);
    }
  rte_rwlock_read_unlock (&thread_info_lock);
}
