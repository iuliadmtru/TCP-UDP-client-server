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
#include "../util/common.h"
#include "../tcp_client/common.h"

void server_exit(struct pollfd *poll_fds,
                 int num_clients,
                 struct topics *topics,
                 struct TCP_clients *clients)
{
    for (int i = 0; i < num_clients; i++) {
        close(poll_fds[i].fd);
    }
    topics_array_free(topics);
    TCP_clients_array_free(clients);

    exit(0);
}
