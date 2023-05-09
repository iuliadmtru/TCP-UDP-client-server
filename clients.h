#ifndef _CLIENTS_H_
#define _CLIENTS_H_

#include <stdint.h>
#include <netinet/in.h>

#define ID_MAXLEN 10

enum client_state {CLIENT_STATE_NEW, CLIENT_STATE_CONNECTED};

typedef struct client_node {
    uint8_t state;  // CLIENT_STATE_NEW || CLIENT_STATE_CONNECTED
    int fd;
    char id[ID_MAXLEN];
    struct in_addr ip;
    uint16_t port;
    struct client_node *prev;
    struct client_node *next;
} client_node;

typedef struct clients_list {
    client_node *head;
} clients_list;

void client_node_print(client_node *client, int idx);

client_node *client_node_create(uint8_t state,
                                int fd,
                                char *id,
                                struct in_addr ip,
                                uint16_t port);

void client_node_destroy(client_node *client);

void clients_list_print(clients_list *clients);

clients_list *clients_list_create();

void clients_list_destroy(clients_list *clients);

void clients_list_add_client(clients_list *clients, client_node *client);

void clients_list_remove_client(clients_list *clients, client_node *client);

void clients_list_remove_client_by_fd(clients_list *clients, int fd);

client_node *clients_list_find_by_id(clients_list *clients, char *id);

client_node *clients_list_find_by_fd(clients_list *clients, int fd);

void clients_list_update_client_by_fd(clients_list *clients,
                                      int fd,
                                      uint8_t state,
                                      char *id);

#endif  // _CLIENTS_H