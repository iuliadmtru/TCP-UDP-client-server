#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>

#include "poller.h"

void poller_print(struct poller *poller)
{
    printf("Print poller with %d file descriptors and iterator index at %d:\n", poller->num_pollfds, poller->idx_iterator);
    for (int i = 0; i < poller->num_pollfds; i++) {
        printf("\tpollfd %d: {fd: %d, events: %hu, revents: %hu}\n", i,
                                                                     poller->pollfds[i].fd,
                                                                     poller->pollfds[i].events,
                                                                     poller->pollfds[i].revents);
    }
}

struct poller *poller_create()
{
    struct poller *poller = malloc(sizeof(struct poller));
    poller->num_pollfds = 0;
    poller->idx_iterator = 0;
    return poller;
}

void poller_destroy(struct poller *poller)
{
    for (int i = 0; i < poller->num_pollfds; i++) {
        close(poller->pollfds[i].fd);
    }
    free(poller);
}

int poller_poll(struct poller *poller)
{
    poller->idx_iterator = 0;
    int rc = poll(poller->pollfds, poller->num_pollfds, -1);
    return rc < 0 ? -1 : 0;
}

void poller_add_fd(struct poller *poller, int fd)
{
    int idx = poller->num_pollfds;
    poller->pollfds[idx].fd = fd;
    poller->pollfds[idx].events = POLLIN;
    poller->num_pollfds++;

    printf("Poller after add:\n");
    poller_print(poller);
    printf("\n");
}

int poller_find_fd_idx(struct poller *poller, int fd)
{
    for (int i = 0; i < poller->num_pollfds; i++) {
        if (poller->pollfds[i].fd == fd)
            return i;
    }

    return -1;
}

void poller_remove_fd(struct poller *poller, int fd)
{
    close(fd);  // TODO: should this be here? or outside?

    int idx = poller_find_fd_idx(poller, fd);
    if (idx < 0) {
        fprintf(stderr, "File descriptor not found in poll\n");
        return;
    }

    for (int i = idx; i < poller->num_pollfds - 1; i++) {
        poller->pollfds[i] = poller->pollfds[i + 1];
    }
    poller->num_pollfds--;
}

int poller_next_fd_with_POLLIN(struct poller *poller)
{
    // printf("Get next fd...\n");
    
    // Search for the next event.
    while (poller->idx_iterator < poller->num_pollfds &&
           poller->pollfds[poller->idx_iterator].revents != POLLIN)
        poller->idx_iterator++;

    if (poller->idx_iterator == poller->num_pollfds) {  // No event.
        // printf("No event\n");

        return -1;
    }

    printf("\nPoller after finding event at file descriptor %d:\n", poller->pollfds[poller->idx_iterator].fd);
    poller_print(poller);
    
    // Move index to the next entry and return the file descriptor where the
    // event happened.
    int fd = poller->pollfds[poller->idx_iterator].fd;
    poller->idx_iterator++;

    return fd;
}
