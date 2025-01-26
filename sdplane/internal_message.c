#include "include.h"

#include <zcmdsh/debug.h>
#include <zcmdsh/vector.h>
#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>
#include <zcmdsh/log_cmd.h>
#include <zcmdsh/debug_log.h>
#include <zcmdsh/debug_category.h>
#include <zcmdsh/debug_cmd.h>
#include <zcmdsh/debug_zcmdsh.h>
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
internal_msg_create (uint16_t type, void *contentp, uint16_t content_length)
{
  void *msgp;
  struct internal_msg_header *msg_header;
  uint16_t length;
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

int
internal_msg_send_to (struct rte_ring *ring,
                      struct internal_msg_header *msgp,
                      struct shell *shell)
{
  if (ring)
    {
      DEBUG_SDPLANE_LOG (IMESSAGE, "sending message %p.", msgp);
      rte_ring_enqueue (ring, msgp);
    }
  else
    {
      if (shell)
        fprintf (shell->terminal, "can't send message %p to "
                 "ring-queue: NULL.%s", msgp, shell->NL);
      DEBUG_SDPLANE_LOG (IMESSAGE, "can't send message %p. "
                 "ring-queue: NULL", msgp);
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

  ret = rte_ring_dequeue (ring, &msgp);
  if (ret == -ENOENT)
    return NULL;

  DEBUG_SDPLANE_LOG (IMESSAGE, "receiving message %p.", msgp);
  return msgp;
}

