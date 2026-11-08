#include "include.h"
#include <assert.h>
#include "sdplane.h"
#include "rib_manager.h"
#include "internal_message.h"
#include "fib.h"
#include "fdb.h"
#include "mdb.h"

char *pid_path;
struct sdplane_process_status process_status;
extern void rib_manager_process_message (void *);
extern void *__real_malloc (size_t);
extern void *__real_aligned_alloc (size_t, size_t);
extern struct fib_tree *__real_fib_new (struct fib_tree *);

enum failure_target
{
  FAIL_RIB,
  FAIL_RIB_INFO,
  FAIL_FIRST_FIB,
  FAIL_LAST_FIB,
  FAIL_TARGET_COUNT
};

static size_t fail_size;
static int fail_fib;

static bool
fail_allocation (size_t size)
{
  if (fail_size && size == fail_size)
    {
      fail_size = 0;
      return true;
    }
  return false;
}

void *
__wrap_malloc (size_t size)
{
  return fail_allocation (size) ? NULL : __real_malloc (size);
}

void *
__wrap_aligned_alloc (size_t alignment, size_t size)
{
  return fail_allocation (size) ? NULL
                                : __real_aligned_alloc (alignment, size);
}

struct fib_tree *
__wrap_fib_new (struct fib_tree *old)
{
  if (fail_fib && --fail_fib == 0)
    return NULL;
  return __real_fib_new (old);
}

static struct internal_msg_header *
create_update_message (void)
{
  /* An unknown type triggers RIB publication without port configuration. */
  struct internal_msg_header *message =
      internal_msg_create (UINT16_MAX, NULL, 0);
  assert (message);
  return message;
}

static void
check_failure (enum failure_target target)
{
  struct rib *old = rcu_global_ptr_rib;
  unsigned char *snapshot = NULL;
  if (old)
    {
      snapshot = malloc (sizeof (*old->rib_info));
      assert (snapshot);
      memcpy (snapshot, old->rib_info, sizeof (*old->rib_info));
    }
  struct internal_msg_header *message = create_update_message ();
  if (target == FAIL_RIB)
    fail_size = sizeof (struct rib);
  else if (target == FAIL_RIB_INFO)
    fail_size = sizeof (struct rib_info);
  else
    fail_fib = target == FAIL_FIRST_FIB ? 1 : ROUTE_TREE_SIZE;
  rib_manager_process_message (message);
  assert (! fail_size && ! fail_fib);
  assert (rcu_global_ptr_rib == old);
  if (old)
    assert (! memcmp (snapshot, old->rib_info, sizeof (*old->rib_info)));
  free (snapshot);
}

int
main (void)
{
  for (int target = 0; target < FAIL_TARGET_COUNT; target++)
    check_failure (target);
  rib_manager_process_message (create_update_message ());
  assert (rcu_global_ptr_rib);
  for (int target = 0; target < FAIL_TARGET_COUNT; target++)
    {
      check_failure (target);
      struct rib *old = rcu_global_ptr_rib;
      rib_manager_process_message (create_update_message ());
      assert (rcu_global_ptr_rib && rcu_global_ptr_rib != old);
    }

  struct rib *current = rcu_global_ptr_rib;
  for (int i = 0; i < ROUTE_TREE_SIZE; i++)
    fib_free (current->rib_info->fib_tree[i]);
  fdb_table_destroy (&current->rib_info->fdb_table);
  mdb_table_destroy (&current->rib_info->mdb_table);
  free (current->rib_info);
  free (current);
  rcu_global_ptr_rib = NULL;
  puts ("PASS: RIB allocation failures preserve published state; "
        "subsequent updates succeed");
  return 0;
}
