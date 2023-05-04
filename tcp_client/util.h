#ifndef _TCP_UTIL_H
#define _TCP_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>

void tcp_client_exit(int sockfd, struct pollfd *poll_fds, int num_fds);

#endif