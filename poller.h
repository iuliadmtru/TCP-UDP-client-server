#ifndef _POLLER_H_
#define _POLLER_H_

#include <poll.h>

#include "util.h"

struct poller {
    struct pollfd pollfds[MAX_CONNECTIONS];
    int num_pollfds;
    int idx_iterator;
};

struct poller *poller_create();

void poller_destroy(struct poller *poller);

int poller_poll(struct poller *poller);

/*
 * Add a new entry with the file descriptor `fd` at the end of the `pollfd`
 * array.
 */
void poller_add_fd(struct poller *poller, int fd);

void poller_remove_fd(struct poller *poller, int fd);

/*
 * Get each file descriptor where a POLLIN happened, one by one. Return -1 if
 * there are no more events.
 */
int poller_next_fd_with_POLLIN(struct poller *poller);

#endif  // _POLLER_H_