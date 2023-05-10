#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdlib.h>

#include "packets.h"

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
#define MAX_CONNECTIONS 1000

enum error_ret_value {FAIL_SOCKET = -1,
                      FAIL_SETSOCKOPT = -2,
                      FAIL_BIND = -3,
                      FAIL_LISTEN = -4,
                      FAIL_ACCEPT = -5,
                      FAIL_CONNECT = -6};

int recv_all(int sockfd, void *buffer, size_t len);

int send_all(int sockfd, void *buffer, size_t len);

enum msg_type {TCP_CTOS_MSG, TCP_STOC_MSG};

void TCP_header_print(struct TCP_header TCP_hdr, int msg_type);

void TCP_ctos_msg_print(struct TCP_ctos_msg TCP_msg);

void TCP_stoc_msg_print(struct TCP_stoc_msg TCP_msg);

void UDP_msg_print(struct UDP_msg UDP_msg);

#endif  // _UTIL_H_