#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "TCP_server.h"
#include "packets.h"
#include "util.h"
#include "clients.h"

void TCP_server_print(struct TCP_server *TCP_server)
{
    printf("Print TCP server:\n");
    printf("\tfd: %d\n", TCP_server->fd);
    printf("\tIP address: %s\n", inet_ntoa(TCP_server->serv_addr.sin_addr));
    printf("\tport: %d\n", ntohs(TCP_server->serv_addr.sin_port));

    printf("\tmessage:\n");
    switch (TCP_server->recv_msg.msg_type) {
        case TCP_MSG_NEW:
            printf("\t\tmsg_type: TCP_MSG_NEW\n");
            break;
        case TCP_MSG_SUBSCRIBE:
            printf("\t\tmsg_type: TCP_MSG_SUBSCRIBE\n");
            break;
        case TCP_MSG_UNSUBSCRIBE:
            printf("\t\tmsg_type: TCP_MSG_UNSUBSCRIBE\n");
            break;
    }
    printf("\t\tpayload: %s\n", TCP_server->recv_msg.payload);

    printf("\tconnected_clients:\n");
    clients_list_print(TCP_server->connected_clients);
}

int TCP_server_initialize_socket(struct sockaddr_in serv_addr)
{
    // Open TCP socket.
    int TCP_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (TCP_fd < 0)
        return FAIL_SOCKET;

    // Make the socket address reusable.
    int enable = 1;
    if (setsockopt(TCP_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        return FAIL_SETSOCKOPT;

    // Bind TCP socket.
    int rc = bind(TCP_fd,
                  (const struct sockaddr *)&serv_addr,
                  sizeof(serv_addr));
    if (rc < 0)
        return FAIL_BIND;

    // Listen on TCP socket.
    rc = listen(TCP_fd, MAX_CONNECTIONS);
    if (rc < 0)
        return FAIL_LISTEN;

    return TCP_fd;
}

struct TCP_server *TCP_server_create(int fd)
{
    struct TCP_server *TCP_server = malloc(sizeof(struct TCP_server));

    TCP_server->fd = fd;
    TCP_server->serv_addr_len = sizeof(struct sockaddr_in);
    TCP_server->connected_clients = clients_list_create();
    TCP_server->bytes_read = 0;

    return TCP_server;
}

void TCP_server_destroy(struct TCP_server *TCP_server)
{
    close(TCP_server->fd);
    clients_list_destroy(TCP_server->connected_clients);
    free(TCP_server);
}

int TCP_server_accept_connection(struct TCP_server *TCP_server)
{
    struct sockaddr_in cli_addr;
    socklen_t cli_addr_len = sizeof(cli_addr);
    int cli_fd = accept(TCP_server->fd,
                        (struct sockaddr *)&cli_addr,
                        &cli_addr_len);
    if (cli_fd < 0)
        return FAIL_ACCEPT;
    
    // Add the new connection to the connected clients list.
    client_node *new_client = client_node_create(CLIENT_STATE_NEW,
                                                 cli_fd,
                                                 "",
                                                 cli_addr.sin_addr,
                                                 ntohs(cli_addr.sin_port));
    clients_list_add_client(TCP_server->connected_clients, new_client);

    printf("TCP_server after accepting connection:\n");
    TCP_server_print(TCP_server);
    printf("\n");

    return cli_fd;
}

int TCP_server_recv(struct TCP_server *TCP_server, int fd)
{
    // Receive packet.
    struct packet recv_packet;
    int rc = recv_all(fd,
                      &recv_packet,
                      sizeof(recv_packet));
    if (rc < 0)
        return -1;
    if (rc == 0)  // Connection ended.
        return 0;

    // Check if `recv_all` finished reading the message length.
    TCP_server->bytes_read += rc;
    if (TCP_server->bytes_read < 2)
        return -2;

    // Check if `recv_all` finished reading the entire message.
    struct TCP_header *TCP_hdr = (struct TCP_header *)recv_packet.msg;
    int msg_len_total = TCP_hdr->msg_len + sizeof(TCP_hdr->msg_len);
    if (TCP_server->bytes_read < msg_len_total)
        return -2;

    // Copy the packet inside the TCP server as a TCP header.
    memcpy(&TCP_server->recv_msg, TCP_hdr->msg, msg_len_total);

    printf("TCP_server after recv:\n");
    TCP_server_print(TCP_server);
    printf("\n");

    return 1;
}

int TCP_server_recv_all(struct TCP_server *TCP_server, int fd)
{
    int rc = -2;
    while (rc == -2) {
        rc = TCP_server_recv(TCP_server, fd);
        if (rc == 0 || rc == -1)
            break;
    }

    // Reset `bytes_read`.
    if (rc > 0)
        TCP_server->bytes_read = 0;

    return rc;
}

void TCP_server_close_connection(struct TCP_server *TCP_server, int fd)
{
    clients_list_remove_client_by_fd(TCP_server->connected_clients, fd);
}

int TCP_server_is_new_connection(struct TCP_server *TCP_server, int fd)
{
    client_node *client =
        clients_list_find_by_fd(TCP_server->connected_clients, fd);
    if (!client) {
        fprintf(stderr, "Client not found.\n");
        return 0;
    }

    return client->state == CLIENT_STATE_NEW;
}

int TCP_server_client_exists(struct TCP_server *TCP_server, char *id)
{
    client_node *client =
        clients_list_find_by_id(TCP_server->connected_clients, id);

    if (client)
        return 1;
    return 0;
}

int TCP_server_update_client(struct TCP_server *TCP_server, int fd)
{
    struct TCP_ctos_msg recv_msg = TCP_server->recv_msg;
    if (recv_msg.msg_type != TCP_MSG_NEW) {
        fprintf(stderr, "Wrong message type.\n");
        return -2;
    }

    // Parse connection message.
    char id[ID_MAXLEN];
    strcpy(id, recv_msg.payload);

    // Check if client already exists.
    if (TCP_server_client_exists(TCP_server, id)) {
        printf("Client %s already connected.\n", id);
        return -1;
    }

    // Update client.
    clients_list_update_client_by_fd(TCP_server->connected_clients,
                                     fd,
                                     CLIENT_STATE_CONNECTED,
                                     id);

    return 0;
}
