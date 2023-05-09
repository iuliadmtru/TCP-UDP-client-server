#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdlib.h>

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

#endif  // _UTIL_H_