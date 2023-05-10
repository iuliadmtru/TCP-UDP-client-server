#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "clients.h"

// ------------------------ CLIENT_NODE ------------------------

void client_node_print(client_node *client, int idx)
{
    printf("Print client %d:\n", idx);
    client->state == CLIENT_STATE_NEW ? printf("\tstate: CLIENT_STATE_NEW\n") :
                                        printf("\tstate: CLIENT_STATE_CONNECTED\n");
    printf("\tfd: %d\n", client->fd);
    printf("\tid: %s\n", client->id);
    printf("\tip: %s\n", inet_ntoa(client->ip));
    printf("\tport: %hu\n", client->port);
    client->prev ? printf("\tprev id: %s\n", client->prev->id) :
                   printf("\tno prev\n");
    client->next ? printf("\tnext id: %s\n", client->next->id) :
                   printf("\tno next\n");
}

client_node *client_node_create(uint8_t state,
                                int fd,
                                char *id,
                                struct in_addr ip,
                                uint16_t port)
{
    client_node *client = malloc(sizeof(client_node));

    client->state = state;
    client->fd = fd;
    strcpy(client->id, id);
    client->ip = ip;
    client->port = port;
    client->prev = NULL;
    client->next = NULL;

    return client;
}

void client_node_destroy(client_node *client)
{
    close(client->fd);
    free(client);
}


// ------------------------ CLIENTS_LIST ------------------------

void clients_list_print(clients_list *clients)
{
    if (!clients->head) {
        printf("Clients list empty.\n");
        return;
    }

    printf("Print clients list:\n");

    int idx = 0;
    client_node *current = clients->head;
    while (current) {
        client_node_print(current, idx);
        idx++;
        current = current->next;
    }
}

clients_list *clients_list_create()
{
    clients_list *clients = malloc(sizeof(clients_list));
    clients->head = NULL;

    return clients;
}

int clients_list_is_empty(clients_list *clients)
{
    return clients->head == NULL;
}

void clients_list_destroy(clients_list *clients)
{
    while (!clients_list_is_empty(clients)) {
        clients_list_remove_client(clients, clients->head);
    }
    free(clients);
}

void clients_list_add_client(clients_list *clients, client_node *client)
{
    if (clients_list_is_empty(clients)) {
        clients->head = client;
        return;
    }

    client->next = clients->head;
    clients->head->prev = client;
    clients->head = client;
}

int clients_list_is_head(clients_list *clients, client_node *client)
{
    if (clients->head->fd == client->fd &&
        strcmp(clients->head->id, client->id) == 0 &&
        clients->head->state == client->state &&
        clients->head->prev == client->prev &&
        clients->head->next == client->next) {
        return 1;
    }
    return 0;
}

void clients_list_remove_client(clients_list *clients, client_node *client)
{
    if (clients_list_is_empty(clients)) {
        fprintf(stderr, "Cannot remove from empty clients list\n");
        return;
    }
    if (!client) {
        fprintf(stderr, "Cannot remove NULL client\n");
        return;
    }

    if (clients_list_is_head(clients, client)) {
        clients->head = clients->head->next;
    } else {
        client->next->prev = client->prev;
        client->prev->next = client->next;
    }
    client_node_destroy(client);
}

void clients_list_remove_client_by_fd(clients_list *clients, int fd)
{
    if (clients_list_is_empty(clients)) {
        fprintf(stderr, "Cannot remove from empty clients list\n");
        return;
    }

    client_node *removed = clients_list_find_by_fd(clients, fd);
    if (!removed) {
        fprintf(stderr, "Cannot remove non-existent client\n");
        return;
    }

    clients_list_remove_client(clients, removed);
}

client_node *clients_list_find_by_id(clients_list *clients, char *id)
{
    if (clients_list_is_empty(clients)) {
        fprintf(stderr, "Cannot find client in empty clients list\n");
        return NULL;
    }

    client_node *current = clients->head;
    while (current) {
        if (strcmp(current->id, id) == 0)
            return current;

        current = current->next;
    }

    return NULL;
}

client_node *clients_list_find_by_fd(clients_list *clients, int fd)
{
    if (clients_list_is_empty(clients)) {
        fprintf(stderr, "Cannot find client in empty clients list\n");
        return NULL;
    }

    client_node *current = clients->head;
    while (current) {
        if (current->fd == fd)
            return current;

        current = current->next;
    }

    return NULL;
}

void clients_list_update_client_by_fd(clients_list *clients,
                                      int fd,
                                      uint8_t state,
                                      char *id)
{
    client_node *client = clients_list_find_by_fd(clients, fd);
    if (!client) {
        fprintf(stderr, "Cannot update client - client not found\n");
        return;
    }

    client->state = state;
    strcpy(client->id, id);
}
