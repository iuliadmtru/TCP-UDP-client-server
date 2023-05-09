#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "TCP_server.h"
#include "packets.h"
#include "util.h"

void TCP_server_print(struct TCP_server *TCP_server)
{
    printf("Print TCP server:\n");
    printf("\tfd: %d\n", TCP_server->fd);
    printf("\tIP address: %s\n", inet_ntoa(TCP_server->serv_addr.sin_addr));
    printf("\tport: %d\n", ntohs(TCP_server->serv_addr.sin_port));

    printf("\tmessage:\n");
    printf("\t\tmsg_len: %hhu\n", TCP_server->msg.msg_len);
    printf("\t\tmsg: %s\n", TCP_server->msg.msg);
}

int TCP_server_get_fd(struct sockaddr_in serv_addr)
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
    client_node *new_client =
        client_node_create(CLIENT_STATE_NEW, cli_fd, "");
    clients_list_add_client(TCP_server->connected_clients, new_client);

    return cli_fd;
}

// int TCP_server_recv(struct TCP_server *TCP_server)
// {
//     // Receive packet.
//     struct packet recv_packet;
//     int rc = recvfrom(TCP_server->fd,
//                       &recv_packet,
//                       sizeof(struct packet),
//                       0,
//                       (struct sockaddr *)&TCP_server->serv_addr,
//                       &TCP_server->serv_addr_len);
//     if (rc < 0)
//         return -1;

//     // Copy the packet inside the TCP server as a TCP message.
//     struct TCP_msg *msg = (struct TCP_msg *)recv_packet.msg;
//     memcpy(&TCP_server->msg, msg, sizeof(struct TCP_msg));

//     printf("\nTCP_server after recvfrom:\n");
//     TCP_server_print(TCP_server);

//     return 0;
// }
