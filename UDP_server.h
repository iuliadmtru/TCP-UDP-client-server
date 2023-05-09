#ifndef _UDP_SERV_H_
#define _UDP_SERV_H_

#include <netinet/in.h>

#include "packets.h"

struct UDP_server {
    int fd;
    struct sockaddr_in serv_addr;
    socklen_t serv_addr_len;
    struct UDP_msg msg;
};

void UDP_server_print(struct UDP_server *UDP_server);

int UDP_server_get_fd(struct sockaddr_in serv_addr);

struct UDP_server *UDP_server_create(int fd);

void UDP_server_destroy(struct UDP_server *UDP_server);

int UDP_server_recv(struct UDP_server *UDP_server);

#endif  // _UDP_SERV_H_