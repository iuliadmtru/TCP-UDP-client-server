#ifndef _SERVER_UTIL_H
#define _SERVER_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>

#include "../util/common.h"
#include "../tcp_client/common.h"

void server_exit(struct pollfd *poll_fds,
                 int num_clients,
                 struct topics *topics,
                 struct TCP_clients *clients);

#endif