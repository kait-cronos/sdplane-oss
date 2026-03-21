// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2023-2026 Yasuhiro Ohara

#include "include.h"

#include <sdplane/debug.h>
#include <sdplane/vector.h>
#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>
#include <sdplane/log_cmd.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "internal_message.h"

void *
internal_msg_body (struct internal_msg_header *msgp)
{
  if (! msgp)
    return NULL;
  return (void *) (msgp + 1);
}

struct internal_msg_header *
internal_msg_create (uint16_t type, void *contentp, uint32_t content_length)
{
  void *msgp;
  struct internal_msg_header *msg_header;
  uint32_t length;
  void *msg_content;

  length = sizeof (struct internal_msg_header) + content_length;
  msgp = (void *) malloc (length);
  if (! msgp)
    return NULL;
  memset (msgp, 0, length);
  msg_header = (struct internal_msg_header *) msgp;
  msg_header->type = type;
  msg_header->length = content_length;
  msg_content = internal_msg_body (msgp);
  if (contentp)
    memcpy (msg_content, contentp, content_length);
  return msg_header;
}

void
internal_msg_delete (struct internal_msg_header *msgp)
{
  free (msgp);
}

#define MSG1 "can't send message %p to ring-queue: NULL."
#define MSG2 "sending internal message faild. "          \
             "please start \"rib_manager\" beforehand."
int
internal_msg_send_to (struct rte_ring *ring,
                      struct internal_msg_header *msgp, struct shell *shell)
{
  if (ring)
    {
      int ret;
      //DEBUG_NEW (IMESSAGE, "sending message %p.", msgp);
      ret = rte_ring_enqueue (ring, msgp);
      DEBUG_NEW (IMESSAGE, "imsg: ring: %p: %d/%d",
                 ring, rte_ring_count (ring),
                 rte_ring_get_size (ring));
      if (ret == -ENOBUFS)
        {
          WARNING ("rte_ring_enqueue failed: ring %s is full. "
                   "message %p (type: %d) is lost.",
                   ring->name, msgp, msgp->type);
          internal_msg_delete (msgp);
          return -1;
        }
    }
  else
    {
      if (shell)
        {
          fprintf (shell->terminal, MSG1 "%s", msgp, shell->NL);
          fprintf (shell->terminal, MSG2 "%s", shell->NL);
        }
      WARNING (MSG1, msgp);
      WARNING (MSG2);
      internal_msg_delete (msgp);
      return -1;
    }
  return 0;
}

struct internal_msg_header *
internal_msg_recv (struct rte_ring *ring)
{
  int ret;
  void *msgp;

  if (! ring)
    return NULL;

#if 0
  DEBUG_NEW (RIB, "receiving message on ring: %p", ring);
  DEBUG_NEW (RIB, "imsg: ring: %p: %d/%d",
             ring, rte_ring_count (ring),
             rte_ring_get_size (ring));
#endif

  ret = rte_ring_dequeue (ring, &msgp);
  if (ret == -ENOENT)
    return NULL;

#if 0
  DEBUG_SDPLANE_LOG (IMESSAGE, "receiving message %p.", msgp);
#endif
  return msgp;
}

int
internal_msg_recv_burst (struct rte_ring *ring,
                         struct internal_msg_header **msg_table,
                         int size)
{
  int ret;
  void *msgp;

  if (! ring)
    return 0;

#if 0
  DEBUG_NEW (RIB, "receiving message on ring: %p", ring);
  DEBUG_NEW (RIB, "imsg: ring: %p: %d/%d",
             ring, rte_ring_count (ring),
             rte_ring_get_size (ring));
#endif

  ret = rte_ring_dequeue_burst (ring, (void **) msg_table, size, NULL);
#if 0
  DEBUG_SDPLANE_LOG (IMESSAGE, "receiving message on ring %p: ret: %d.",
                     ring, ret);
#endif
  return ret;
}
