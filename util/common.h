#ifndef __COMMON_H__
#define __COMMON_H__

#include <stddef.h>
#include <stdint.h>

int send_all(int sockfd, void *buff, size_t len);
int recv_all(int sockfd, void *buff, size_t len);

/*
 * Maximum sizes.
 */
#define CMD_MAXSIZE 20
#define MSG_MAXSIZE 1551
#define TOPIC_SIZE 50
#define CONTENT_MAXSIZE 1500

#define MAX_CONNECTIONS 32

struct packet {
    char message[MSG_MAXSIZE + 1];
};

struct UDP_message {
    char topic[TOPIC_SIZE];
    uint8_t data_type;
    char content[CONTENT_MAXSIZE];
};

struct TCP_message {
    uint8_t subscribe_status;
    char topic[TOPIC_SIZE];
    uint8_t sf;
};

#endif
