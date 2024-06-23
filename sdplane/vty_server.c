#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include <lthread.h>

struct vty_client
{
  struct sockaddr_in peer_addr;
  int fd;
};
typedef struct vty_client vty_client_t;

void
vty_shell (void *arg)
{
  vty_client_t *client = (vty_client_t *) arg;

  char client_addr_str[128];
  inet_ntop (AF_INET, &client->peer_addr.sin_addr,
             client_addr_str, sizeof (client_addr_str));

  printf ("%s for %s.\n", __func__, client_addr_str);

  char send_buf[1024];
  char buf[1024];
  int ret;
  ret = lthread_recv (client->fd, buf, sizeof (buf), 0, 5000);
  if (ret == -2)
    {
      snprintf (send_buf, sizeof (send_buf), "timeout.\n");
      lthread_send (client->fd, send_buf, strlen (send_buf), 0);
      lthread_close (client->fd);
      return;
    }

  snprintf (send_buf, sizeof (send_buf), "your message: %s\n", buf);
  lthread_send (client->fd, send_buf, strlen (send_buf), 0);
  lthread_close (client->fd);

  printf ("%s finished for %s.\n", __func__, client_addr_str);

  return;
}

void
vty_server (void *arg)
{
  int sockfd;
  int ret;
  int client_fd;

  sockfd = lthread_socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd == -1)
    {
      fprintf (stderr, "lthread_socket() failed.\n");
      return;
    }

  int optval = 1;
  ret = setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR,
                    &optval, sizeof (optval));
  if (ret < 0)
    {
      fprintf (stderr, "setsockopt(REUSEADDR) failed: %s.\n",
               strerror (errno));
      return;
    }

  struct sockaddr_in peer_addr;
  struct sockaddr_in sin;
  socklen_t addrlen = sizeof (peer_addr);

  sin.sin_family = PF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons (9882);

  ret = bind (sockfd, (struct sockaddr *) &sin, sizeof (sin));
  if (ret < 0)
    {
      fprintf (stderr, "bind() failed: %s.\n", strerror (errno));
      return;
    }

  printf ("Starting vty_server on port 9882\n");

  int client_size = 0;
  vty_client_t client_info[5];
  lthread_t *client_lt = NULL;

  listen (sockfd, 128);

  while (1)
    {
      printf ("%s: schedule.\n");
      client_fd = lthread_accept (sockfd, (struct sockaddr *) &peer_addr,
                                  &addrlen);
      if (client_fd < 0)
        {
          fprintf (stderr, "lthread_accept() failed.\n");
          continue;
        }

      printf ("%s: lthread_accept: client[%d]\n", client_size);
      client_info[client_size].peer_addr = peer_addr;
      client_info[client_size].fd = client_fd;
      ret = lthread_create (&client_lt, vty_shell, &client_info[client_size]);

      client_size++;
    }
}

