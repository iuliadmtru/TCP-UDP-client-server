#ifndef __COMMON_H__
#define __COMMON_H__

#include <stddef.h>
#include <stdint.h>

int send_all(int sockfd, void *buff, size_t len);
int recv_all(int sockfd, void *buff, size_t len);

/*
 * Maximum sizes.
 */
#define ID_MAXSIZE 10
#define CMD_MAXSIZE 20
#define MSG_MAXSIZE 1551
#define TOPIC_SIZE 50
#define CONTENT_MAXSIZE 1500

#define MAX_CONNECTIONS 32
#define MAX_TOPICS 1000

struct packet {
    char message[MSG_MAXSIZE + 1];
};

struct UDP_message {
    char topic[TOPIC_SIZE];
    uint8_t data_type;
    char content[CONTENT_MAXSIZE];
};

struct TCP_message {
    char id[ID_MAXSIZE];
    uint8_t subscribe_status;
    char topic[TOPIC_SIZE];
    uint8_t sf;
};

struct subscriber {
    char id[ID_MAXSIZE];
    uint8_t sf;
};

struct topic {
    char name[TOPIC_SIZE];
    struct subscriber *subscribers;
    int num_subscribers;
};

struct topics {
    struct topic **topics;
    int size;
};

struct subscriber subscriber_create(char *id, uint8_t sf);

void subscriber_print(struct subscriber sub);

struct topic *topic_create(char *name);

void topic_free(struct topic *topic);

void topic_print(struct topic *topic);

void subscribe_to_topic(struct topic *topic, struct subscriber sub);

void unsubscribe_from_topic(struct topic *topic, char *id);

struct topics *topics_array_create();

void topics_array_free(struct topics *topics);

void topics_array_print(struct topics *topics);

void topics_array_add_topic(struct topics* topics, char *name);

struct topic *find_topic(struct topics* topics, char *name);

#endif
