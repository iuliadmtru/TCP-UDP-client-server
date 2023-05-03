#include "common.h"
#include "helpers.h"

#include <sys/socket.h>
#include <sys/types.h>

/*
    TODO 1.1: Rescrieți funcția de mai jos astfel încât ea să facă primirea
    a exact len octeți din buffer.
*/
int recv_all(int sockfd, void *buffer, size_t len)
{

    size_t bytes_received = 0;
    size_t bytes_remaining = len;
    char *buff = buffer;

    while (bytes_remaining) {
        ssize_t rc = recv(sockfd, buff + bytes_received, bytes_remaining, 0);
        DIE(rc == -1, "recv failed\n");

        bytes_received += rc;
        bytes_remaining -= rc;
    }

    return recv(sockfd, buffer, len, 0);
}

/*
    TODO 1.2: Rescrieți funcția de mai jos astfel încât ea să facă trimiterea
    a exact len octeți din buffer.
*/

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