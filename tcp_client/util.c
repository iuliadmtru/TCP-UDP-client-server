#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <poll.h>

#include "util.h"
#include "common.h"

void tcp_client_exit(int sockfd, struct pollfd *poll_fds, int num_fds)
{
    printf("Entered `tcp_client_exit`...\n");

    for (int i = 0; i < num_fds; i++) {
        close(poll_fds[i].fd);
    }
    close(sockfd);
    exit(0);
}
