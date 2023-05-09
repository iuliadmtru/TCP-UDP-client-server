#ifndef _TCP_SERV_H_
#define _TCP_SERV_H_

#include <netinet/in.h>

#include "packets.h"
#include "clients.h"

struct TCP_server {
    int fd;
    struct sockaddr_in serv_addr;
    socklen_t serv_addr_len;
    struct TCP_header msg;
    clients_list *connected_clients;
};

void TCP_server_print(struct TCP_server *TCP_server);

int TCP_server_get_fd(struct sockaddr_in serv_addr);

struct TCP_server *TCP_server_create(int fd);

void TCP_server_destroy(struct TCP_server *TCP_server);

int TCP_server_accept_connection(struct TCP_server *TCP_server);

// int TCP_server_recv(struct TCP_server *TCP_server);

#endif  // _TCP_SERV_H_