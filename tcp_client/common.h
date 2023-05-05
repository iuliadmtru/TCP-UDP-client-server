#ifndef _TCP_COMMON_H
#define _TCP_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "util.h"

struct TCP_client {
    char id[10];
    uint32_t fd;
};

struct TCP_clients {
    struct TCP_client *clients;
    int size;
};

struct TCP_client TCP_client_create(char *id, uint32_t fd);

void TCP_client_print(struct TCP_client client);

struct TCP_clients *TCP_clients_array_create();

void TCP_clients_array_free(struct TCP_clients *clients);

void TCP_clients_array_print(struct TCP_clients *clients);

void TCP_clients_array_add_client(struct TCP_clients *clients,
                                  char *id,
                                  uint32_t fd);

void TCP_clients_array_remove_client(struct TCP_clients *clients,
                                     char *id);

struct TCP_client *TCP_clients_array_find_client(struct TCP_clients *clients,
                                                char *id);

#endif