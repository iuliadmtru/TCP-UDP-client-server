#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "UDP_server.h"
#include "packets.h"
#include "util.h"

void UDP_server_print(struct UDP_server *UDP_server)
{
    printf("Print UDP server:\n");
    printf("\tfd: %d\n", UDP_server->fd);
    printf("\tIP address: %s\n", inet_ntoa(UDP_server->serv_addr.sin_addr));
    printf("\tport: %d\n", ntohs(UDP_server->serv_addr.sin_port));

    printf("\tmessage:\n");
    printf("\t\ttopic: %s\n", UDP_server->msg.topic);
    printf("\t\tdata_type: %hhu\n", UDP_server->msg.data_type);
    printf("\t\tcontent: %s\n", UDP_server->msg.content);
}

int UDP_server_get_fd(struct sockaddr_in serv_addr)
{
    // Open socket.
    int UDP_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (UDP_fd < 0)
        return FAIL_SOCKET;

    // Make the socket address reusable.
    int enable = 1;
    if (setsockopt(UDP_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        return FAIL_SETSOCKOPT;

    // Bind socket.
    int rc = bind(UDP_fd,
                  (const struct sockaddr *)&serv_addr,
                  sizeof(serv_addr));
    if (rc < 0)
        return FAIL_BIND;

    return UDP_fd;
}

struct UDP_server *UDP_server_create(int fd)
{
    struct UDP_server *UDP_server = malloc(sizeof(struct UDP_server));

    UDP_server->fd = fd;
    UDP_server->serv_addr_len = sizeof(struct sockaddr_in);

    return UDP_server;
}

void UDP_server_destroy(struct UDP_server *UDP_server)
{
    close(UDP_server->fd);
    free(UDP_server);
}

int UDP_server_recv(struct UDP_server *UDP_server)
{
    // Receive packet.
    struct packet recv_packet;
    int rc = recvfrom(UDP_server->fd,
                      &recv_packet,
                      sizeof(struct packet),
                      0,
                      (struct sockaddr *)&UDP_server->serv_addr,
                      &UDP_server->serv_addr_len);
    if (rc < 0)
        return -1;

    // Copy the packet inside the UDP server as a UDP message.
    struct UDP_msg *msg = (struct UDP_msg *)recv_packet.msg;
    memcpy(&UDP_server->msg, msg, sizeof(struct UDP_msg));

    printf("\nUDP_server after recvfrom:\n");
    UDP_server_print(UDP_server);

    return 0;
}
