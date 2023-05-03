#ifndef __COMMON_H__
#define __COMMON_H__

#include <stddef.h>
#include <stdint.h>

int send_all(int sockfd, void *buff, size_t len);
int recv_all(int sockfd, void *buff, size_t len);

/*
 * Maximum sizes.
 */
#define MSG_MAXSIZE 1551
#define TOPIC_SIZE 50
#define CONTENT_MAXSIZE 1500

struct UDP_packet {
    uint16_t len;
    char message[MSG_MAXSIZE + 1];
};

struct UDP_message {
    char topic[TOPIC_SIZE];
    uint8_t data_type;
    char content[CONTENT_MAXSIZE];
}

#endif
