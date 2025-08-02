#ifndef __VTY_SERVER_H__
#define __VTY_SERVER_H__

struct vty_client
{
  int id;
  struct sockaddr_in peer_addr;
  int fd;
};
typedef struct vty_client vty_client_t;

#define VTY_CLIENT_MAX  5

void vty_server (void *arg);

#endif /*__VTY_SERVER_H__*/
