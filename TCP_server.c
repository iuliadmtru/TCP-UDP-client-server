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
#include "subscriptions.h"
#include "poller.h"

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
    TCP_server->subscriptions = subscriptions_create();
    TCP_server->bytes_read = 0;

    return TCP_server;
}

void TCP_server_destroy(struct TCP_server *TCP_server)
{
    close(TCP_server->fd);
    clients_list_destroy(TCP_server->connected_clients);
    subscriptions_destroy(TCP_server->subscriptions);
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

    // printf("TCP_server after accepting connection:\n");
    // TCP_server_print(TCP_server);
    // printf("\n");

    return cli_fd;
}

int TCP_server_recv_all(struct TCP_server *TCP_server, int fd)
{
    struct packet recv_packet;
    memset(&recv_packet, 0, sizeof(struct packet));

    // printf("Receive packet on file descriptor %d...\n", fd);

    // Receive packet.
    int rc = recv(fd, &recv_packet, sizeof(recv_packet), MSG_WAITALL);

    // printf("Received %d bytes out of %ld, message: %hu\n", rc, sizeof(TCP_hdr.msg_len), *(uint16_t *)recv_packet.msg);

    if (rc < 0)
        return -1;
    if (rc == 0)  // Connection ended.
        return 0;

    struct TCP_header TCP_hdr = *(struct TCP_header *)recv_packet.msg;
    memcpy(&TCP_server->recv_msg, (struct TCP_ctos_msg *)&TCP_hdr.msg, TCP_hdr.msg_len);

    return 1;
}

int TCP_server_recv_all1(struct TCP_server *TCP_server, int fd)
{
    struct TCP_header TCP_hdr;
    struct packet recv_packet;
    memset(&recv_packet, 0, sizeof(struct packet));

    printf("Receive packet on file descriptor %d...\n", fd);

    // Receive packet length.
    int rc = recv(fd, &recv_packet, sizeof(TCP_hdr.msg_len), MSG_WAITALL);

    printf("Received %d bytes out of %ld, message: %hu\n", rc, sizeof(TCP_hdr.msg_len), *(uint16_t *)recv_packet.msg);

    if (rc < 0)
        return -1;
    if (rc == 0)  // Connection ended.
        return 0;

    // Parse the message length.
    uint16_t msg_len;
    sscanf(recv_packet.msg, "%hu", &msg_len);

    TCP_hdr = *(struct TCP_header *)recv_packet.msg;

    // Receive entire packet.
    rc = recv(fd, &recv_packet + sizeof(TCP_hdr.msg_len), TCP_hdr.msg_len, MSG_WAITALL);
    if (rc < 0)
        return -1;

    printf("Received %d more bytes out of %hu, message:\n", rc, TCP_hdr.msg_len);
    TCP_hdr = *(struct TCP_header *)recv_packet.msg;
    struct TCP_ctos_msg TCP_msg = *(struct TCP_ctos_msg *)TCP_hdr.msg;
    TCP_ctos_msg_print(TCP_msg);
    
    // Copy the packet inside the TCP server as a TCP CTOS message.
    // int msg_len_total = TCP_hdr.msg_len + sizeof(TCP_hdr.msg_len);
    memcpy(&TCP_server->recv_msg,
           &TCP_msg,
           TCP_hdr.msg_len);

    // printf("TCP_server after recv:\n");
    // TCP_server_print(TCP_server);
    // printf("\n");

    return 1;
}

int TCP_server_send(struct TCP_server *TCP_server,
                    int fd,
                    struct TCP_header TCP_msg)
{
    struct packet sent_packet;
    memset(&sent_packet, 0, sizeof(sent_packet));

    memcpy(sent_packet.msg, (char *)&TCP_msg, TCP_msg.msg_len);
    int rc = send_all(fd, &sent_packet, sizeof(sent_packet));

    printf("Sent message:\n");
    TCP_header_print(*(struct TCP_header *)sent_packet.msg, TCP_STOC_MSG);

    return rc;
}

void TCP_server_close_connection(struct TCP_server *TCP_server,
                                 struct poller *poller,
                                 int fd)
{
    // printf("TCP server closing connection with file descriptor %d\n", fd);

    clients_list_remove_client_by_fd(TCP_server->connected_clients, fd);
    poller_remove_fd(poller, fd);

    // printf("TCP server after closing connection:\n");
    // TCP_server_print(TCP_server);
    // printf("\n");
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

struct subscribe_args {
    char topic[TOPIC_LEN];
    uint8_t store_and_forward;
};

struct subscribe_args TCP_server_get_sub_args(struct TCP_server *TCP_server)
{
    struct subscribe_args args;
    sscanf(TCP_server->recv_msg.payload,
           "%s %hhu",
           args.topic,
           &args.store_and_forward);

    return args;
}

void TCP_server_subscribe(struct TCP_server *TCP_server, int fd)
{
    // Find subscriber.
    client_node *subscriber =
        clients_list_find_by_fd(TCP_server->connected_clients, fd);
    if (!subscriber) {
        fprintf(stderr, "Cannot subscribe - client not found\n");
        return;
    }

    // Create new subscription.
    struct subscribe_args args = TCP_server_get_sub_args(TCP_server);
    subscription_node *sub = subscription_node_create(args.topic,
                                                      subscriber->id,
                                                      args.store_and_forward);
    subscriptions_add_subscription(TCP_server->subscriptions, sub);
}

void TCP_server_unsubscribe(struct TCP_server *TCP_server, int fd)
{
    // Find subscriber.
    client_node *unsubscriber =
        clients_list_find_by_fd(TCP_server->connected_clients, fd);
    if (!unsubscriber) {
        fprintf(stderr, "Cannot unsubscribe - client not connected\n");
        return;
    }

    subscription_node *subscription =
        subscriptions_find(TCP_server->subscriptions,
                           TCP_server->recv_msg.payload,
                           unsubscriber->id);
    if (!subscription) {
        fprintf(stderr, "Cannot unsubscribe - client not subscribed\n");
        return;
    }

    subscriptions_remove_subscription(TCP_server->subscriptions, subscription);
}

int TCP_server_send_to_subscribers(struct TCP_server *TCP_server,
                                   struct TCP_header TCP_msg)
{
    struct TCP_stoc_msg TCP_stoc_msg = *(struct TCP_stoc_msg *)TCP_msg.msg;
    struct UDP_msg UDP_msg = TCP_stoc_msg.content;

    int rc;
    while (1) {
        subscription_node *sub =
            subscriptions_get_next(TCP_server->subscriptions, UDP_msg.topic);
        if (!sub)
            break;

        // Send the message to the subscriber. (Or store it!)
        client_node *subscriber =
            clients_list_find_by_id(TCP_server->connected_clients,
                                    sub->subscriber_id);
        rc = TCP_server_send(TCP_server, subscriber->fd, TCP_msg);

        if (rc < 0)
            return -1;
    }

    return rc;
}
