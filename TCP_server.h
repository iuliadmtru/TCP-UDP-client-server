#ifndef _TCP_SERV_H_
#define _TCP_SERV_H_

#include <netinet/in.h>

#include "packets.h"
#include "clients.h"
#include "subscriptions.h"
#include "poller.h"

struct TCP_server {
    int fd;
    struct sockaddr_in serv_addr;
    socklen_t serv_addr_len;
    struct TCP_ctos_msg recv_msg;
    int bytes_read;
    clients_list *connected_clients;
    subscriptions_list *subscriptions;
};

void TCP_server_print(struct TCP_server *TCP_server);

int TCP_server_initialize_socket(struct sockaddr_in serv_addr);

struct TCP_server *TCP_server_create(int fd);

void TCP_server_destroy(struct TCP_server *TCP_server);

int TCP_server_accept_connection(struct TCP_server *TCP_server);

int TCP_server_recv_all(struct TCP_server *TCP_server, int fd);

int TCP_server_send(struct TCP_server *TCP_server,
                    int fd,
                    struct TCP_header TCP_msg);

void TCP_server_close_connection(struct TCP_server *TCP_server,
                                 struct poller *poller,
                                 int fd);

int TCP_server_is_new_connection(struct TCP_server *TCP_server, int fd);

int TCP_server_update_client(struct TCP_server *TCP_server, int fd);

void TCP_server_subscribe(struct TCP_server *TCP_server, int fd);

void TCP_server_unsubscribe(struct TCP_server *TCP_server, int fd);

int TCP_server_send_to_subscribers(struct TCP_server *TCP_server,
                                   struct TCP_header TCP_msg);

#endif  // _TCP_SERV_H_