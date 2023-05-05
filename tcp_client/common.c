#include <string.h>

#include "common.h"
#include "../util/common.h"
#include "../util/helpers.h"

struct TCP_client TCP_client_create(char *id, uint32_t fd)
{
    struct TCP_client client;
    strcpy(client.id, id);
    client.fd = fd;

    return client;
}

void TCP_client_print(struct TCP_client client)
{
    printf("TCP client with {ID = %s, file descriptor = %d}.\n", client.id, client.fd);
}

struct TCP_clients *TCP_clients_array_create()
{
    struct TCP_clients *clients =
        malloc(sizeof(struct TCP_clients));
    DIE(!clients, "malloc failed");

    clients->size = 0;
    clients->clients = malloc(MAX_CONNECTIONS * sizeof(struct TCP_client));

    return clients;
}

void TCP_clients_array_free(struct TCP_clients *clients)
{
    free(clients->clients);
    free(clients);
}

void TCP_clients_array_print(struct TCP_clients *clients)
{
    printf("TCP clients array with %d clients:\n", clients->size);
    for (int i = 0; i < clients->size; i++) {
        printf("\t");
        TCP_client_print(clients->clients[i]);
    }
}

void TCP_clients_array_add_client(struct TCP_clients *clients,
                                  char *id,
                                  uint32_t fd)
{
    clients->clients[clients->size] = TCP_client_create(id, fd);
    clients->size++;
}

void TCP_clients_array_remove_client(struct TCP_clients *clients,
                                     char *id)
{
    if (clients->size == 0) {
        fprintf(stderr, "Cannot remove client from empty clients array.\n");
        return;
    }

    int i = 0, n = clients->size;
    while (i < n && strcmp(clients->clients[i].id, id) != 0)
        i++;

    if (i == n) {
        fprintf(stderr, "Client not found.\n");
        return;
    }

    while (i < n - 1) {
        clients->clients[i] = clients->clients[i + 1];
        i++;
    }

    clients->size--;
}

struct TCP_client *TCP_clients_array_find_client(struct TCP_clients *clients,
                                                char *id)
{
    struct TCP_client *client = NULL;
    for (int i = 0; i < clients->size; i++) {
        if (strcmp(clients->clients[i].id, id) == 0) {
            client = &clients->clients[i];
            break;
        }
    }

    return client;
}
