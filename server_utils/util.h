#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>

void server_exit(struct pollfd *poll_fds, int num_clients);

#endif