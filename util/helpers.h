#ifndef _HELPERS_H
#define _HELPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "common.h"

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

;

/*
 * Messages.
 */
void server_print_connection_status(int connected,
                                    int id,
                                    char *ip,
                                    uint16_t port);

void TCP_client_print_subscription_status(int subscribed);

void UDP_parse_message(struct packet packet,
                       char *ip,
                       uint16_t *port,
                       void *destination);

void packet_from_UDP_msg(struct packet *packet, struct UDP_message UDP_msg);

void UDP_msg_from_packet(struct UDP_message *UDP_msg, struct packet packet);

void UDP_print_subscription_message(struct UDP_message UDP_msg);

void TCP_parse_message(struct packet packet, void *destination);

#endif
