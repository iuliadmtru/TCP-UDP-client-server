#include "common.h"
#include "helpers.h"

#include <sys/socket.h>
#include <sys/types.h>

int recv_all(int sockfd, void *buffer, size_t len)
{
    printf("Entered `recv_all`...\n");

    size_t bytes_received = 0;
    size_t bytes_remaining = len;
    char *buff = buffer;

    while (bytes_remaining) {
        printf("Receive bytes...\n");

        ssize_t rc = recv(sockfd, buff + bytes_received, bytes_remaining, 0);
        DIE(rc == -1, "recv failed\n");
        if (rc == 0)
            return 0;

        bytes_received += rc;
        bytes_remaining -= rc;
    }

    printf("Exiting `recv_all` after receiving message %s...\n", (char *)buffer);

    return recv(sockfd, buffer, len, 0);
}

int send_all(int sockfd, void *buffer, size_t len)
{
    size_t bytes_sent = 0;
    size_t bytes_remaining = len;
    char *buff = buffer;

    while(bytes_remaining) {
        ssize_t rc = send(sockfd, buff + bytes_sent, bytes_remaining, 0);
        DIE(rc == -1, "send failed\n");

        bytes_sent += rc;
        bytes_remaining -= rc;
    }

    return send(sockfd, buffer, len, 0);
}