#ifndef _UTIL_H_
#define _UTIL_H_

// #include <stdlib.h>

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

enum error_ret_value {FAIL_SOCKET = -1,
                      FAIL_SETSOCKOPT = -2,
                      FAIL_BIND = -3,
                      FAIL_LISTEN = -4};

#endif  // _UTIL_H_