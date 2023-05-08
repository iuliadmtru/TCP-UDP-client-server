#ifndef _CLIENTS_H_
#define _CLIENTS_H_

#include <stdint.h>

#define ID_MAXLEN 10

typedef struct clients_list {
    client_node *head;
} clients_list;

typedef struct client_node {
    uint8_t state;  // CLIENT_STATE_NEW || CLIENT_STATE_CONNECTED
    uint32_t sockfd;
    char id[ID_MAXLEN];
    client_node *prev;
    client_node *next;
} client_node;

client_node *client_node_create(uint32_t sockfd,
                                char *id,
                                client_node *prev);

void client_node_destroy(client_node *client);

clients_list *clients_list_create();

void clients_list_destroy(clients_list *clients);

void clients_list_add_client(clients_list *clients, client_node *client);

void clients_list_remove_client(clients_list *clients, client_node *client);

client_node *clients_list_find_by_id(clients_list *clients, char *id);

client_node *clients_list_find_by_fd(clients_list *clients, uint32_t fd);

#endif  // _CLIENTS_H