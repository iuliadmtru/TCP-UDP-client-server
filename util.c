#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

#include "util.h"

int recv_all(int sockfd, void *buffer, size_t len)
{
    // printf("Entered `recv_all`...\n");

    size_t bytes_received = 0;
    size_t bytes_remaining = len;
    char *buff = buffer;

    while (bytes_remaining) {
        // printf("Receive bytes...\n");

        ssize_t rc = recv(sockfd, buff + bytes_received, bytes_remaining, 0);
        DIE(rc < 0, "recv failed");

        if (rc == 0)
            return 0;

        bytes_received += rc;
        bytes_remaining -= rc;
    }

    // printf("Exiting `recv_all` after receiving message %s...\n", (char *)buffer);

    return recv(sockfd, buffer, len, 0);
}

int send_all(int sockfd, void *buffer, size_t len)
{
    size_t bytes_sent = 0;
    size_t bytes_remaining = len;
    char *buff = buffer;

    while(bytes_remaining) {
        ssize_t rc = send(sockfd, buff + bytes_sent, bytes_remaining, 0);
        DIE(rc < 0, "send failed");

        bytes_sent += rc;
        bytes_remaining -= rc;
    }

    return send(sockfd, buffer, len, 0);
}

void TCP_header_print(struct TCP_header TCP_hdr, int msg_type)
{
    printf("Print TCP header:\n");
    printf("\tmsg_len: %hu\n", TCP_hdr.msg_len);
    msg_type == TCP_CTOS_MSG ? TCP_ctos_msg_print(*(struct TCP_ctos_msg *)TCP_hdr.msg)
                             : TCP_stoc_msg_print(*(struct TCP_stoc_msg *)TCP_hdr.msg);
    printf("\n");
}

void TCP_ctos_msg_print(struct TCP_ctos_msg TCP_msg)
{
    printf("Print TCP CTOS message:\n");
    switch (TCP_msg.msg_type) {
        case TCP_MSG_NEW:
            printf("\tmsg_type: TCP_MSG_NEW\n");
            break;
        case TCP_MSG_SUBSCRIBE:
            printf("\tmsg_type: TCP_MSG_SUBSCRIBE\n");
            break;
        case TCP_MSG_UNSUBSCRIBE:
            printf("\tmsg_type: TCP_MSG_UNSUBSCRIBE\n");
            break;
    }
    printf("\tpayload: %s\n", TCP_msg.payload);
    printf("\n");
}

#include <arpa/inet.h>

void TCP_stoc_msg_print(struct TCP_stoc_msg TCP_msg)
{
    printf("Print TCP STOC message:\n");
    printf("\tsrc_ip: %s\n", inet_ntoa(TCP_msg.src_ip));
    printf("\tsrc_port: %hu\n", TCP_msg.src_port);
    printf("Print content:\n");
    UDP_msg_print(TCP_msg.content);
    printf("\n");
}

void UDP_msg_print(struct UDP_msg UDP_msg)
{
    printf("Print UDP message:\n");
    printf("\ttopic: %s\n", UDP_msg.topic);
    printf("\tdata_type: %hhu\n", UDP_msg.data_type);
    printf("\tcontent: %s\n", UDP_msg.content);
    printf("\n");
}
