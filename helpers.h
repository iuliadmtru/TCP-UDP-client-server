#ifndef _HELPERS_H
#define _HELPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * Error-checking macro.
 */
#define DIE(assertion, call_description)                                       \
    do {                                                                       \
        if (assertion) {                                                       \
            fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                 \
            perror(call_description);                                          \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    } while (0)

/*
 * Messages.
 */
void server_print_connection_status(int connected,
                                    int id,
                                    uint32_t ip,
                                    int port);

void TCP_client_print_subscription_status(int subscribed);

void TCP_client_print_subscription_message(uint32_t ip,
                                           int port,
                                           char *topic,
                                           char *data_type,
                                           char *message);

#endif
