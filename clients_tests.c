#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "clients.h"

int main()
{
    clients_list *clients = clients_list_create();
    clients_list_print(clients);
    printf("\n");

    // Add two clients.
    client_node *client1 = client_node_create(CLIENT_STATE_NEW, 11, "client1");
    client_node *client2 = client_node_create(CLIENT_STATE_NEW, 12, "client2");
    clients_list_add_client(clients, client1);
    clients_list_add_client(clients, client2);
    clients_list_print(clients);
    printf("\n");

    // Search for clients.
    // client1 by id
    client_node *search1 = clients_list_find_by_id(clients, client1->id);
    assert(search1->state == client1->state);
    assert(search1->fd == client1->fd);
    assert(strcmp(search1->id, client1->id) == 0);
    assert(search1->prev == client1->prev);
    assert(search1->next == client1->next);
    // client1 by fd
    client_node *search2 = clients_list_find_by_fd(clients, client2->fd);
    assert(search2->state == client2->state);
    assert(search2->fd == client2->fd);
    assert(strcmp(search2->id, client2->id) == 0);
    assert(search2->prev == client2->prev);
    assert(search2->next == client2->next);

    clients_list_print(clients);
    printf("\n");

    // Update clients.
    clients_list_update_client_by_fd(clients,
                                     client1->fd,
                                     CLIENT_STATE_CONNECTED,
                                     "client1_connected");
    clients_list_print(clients);
    printf("\n");

    // Destroy clients list.
    clients_list_destroy(clients);

    return 0;
}
