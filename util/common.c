#include "common.h"
#include "helpers.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>

int recv_all(int sockfd, void *buffer, size_t len)
{
    printf("Entered `recv_all`...\n");

    size_t bytes_received = 0;
    size_t bytes_remaining = len;
    char *buff = buffer;

    while (bytes_remaining) {
        printf("Receive bytes...\n");

        ssize_t rc = recv(sockfd, buff + bytes_received, bytes_remaining, 0);
        DIE(rc == -1, "recv failed\n");
        if (rc == 0)
            return 0;

        bytes_received += rc;
        bytes_remaining -= rc;
    }

    printf("Exiting `recv_all` after receiving message %s...\n", (char *)buffer);

    return recv(sockfd, buffer, len, 0);
}

int send_all(int sockfd, void *buffer, size_t len)
{
    size_t bytes_sent = 0;
    size_t bytes_remaining = len;
    char *buff = buffer;

    while(bytes_remaining) {
        ssize_t rc = send(sockfd, buff + bytes_sent, bytes_remaining, 0);
        DIE(rc == -1, "send failed\n");

        bytes_sent += rc;
        bytes_remaining -= rc;
    }

    return send(sockfd, buffer, len, 0);
}

struct subscriber subscriber_create(int fd, char *id, uint8_t sf)
{
    struct subscriber sub;
    sub.fd = fd;
    sub.sf = sf;
    strcpy(sub.id, id);

    return sub;
}

void subscriber_print(struct subscriber sub)
{
    printf("Subscriber with {file descriptor = %d, ID = %s, sf = %hhu}\n", sub.fd, sub.id, sub.sf);
}

struct topic *topic_create(char *name)
{
    // printf("Entered `topic_create`...\n");

    struct topic *topic = malloc(sizeof(struct topic));
    DIE(!topic, "malloc failed");

    strcpy(topic->name, name);

    topic->subscribers = malloc(MAX_CONNECTIONS * sizeof(struct subscriber));
    DIE(!topic->subscribers, "malloc failed");
    topic->num_subscribers = 0;

    // printf("Created topic %s...\n", name);

    return topic;
}

void topic_free(struct topic *topic)
{
    free(topic->subscribers);
    free(topic);
}

void topic_print(struct topic *topic)
{
    printf("Topic %s with %d subscribers:\n", topic->name , topic->num_subscribers);
    for (int i = 0; i < topic->num_subscribers; i++) {
        printf("\t");
        subscriber_print(topic->subscribers[i]);
    }
}

void subscribe_to_topic(struct topic *topic, struct subscriber sub)
{
    topic->subscribers[topic->num_subscribers] = sub;
    topic->num_subscribers++;
}

void unsubscribe_from_topic(struct topic *topic, char *id)
{
    int i = 0, n = topic->num_subscribers;
    if (n == 0) {
        // TODO: print la stderr
        printf("Cannot unsubscribe from topic with no subscribers.\n");
        return;
    }

    while (strcmp(topic->subscribers[i].id, id) != 0 && i < n)
        i++;
    
    // printf("Remove subscriber %d\n", i);

    if (i == n) {
        // TODO: print la stderr
        printf("Client is not subscribed to this topic.\n");
        return;
    }

    // Found subscriber -> remove from list and decrease subscribers number.
    while (i < n - 1) {
        // printf("Move subscriber %d to %d\n", i + 1, i);
        topic->subscribers[i] = topic->subscribers[i + 1];
        i++;
    }

    topic->num_subscribers--;
}

struct topics *topics_array_create()
{
    struct topics *topics = malloc(sizeof(struct topics));
    DIE(!topics, "malloc failed");

    topics->size = 0;
    topics->topics = malloc(MAX_TOPICS * sizeof(struct topic));

    return topics;
}

void topics_array_free(struct topics *topics)
{
    for (int i = 0; i < topics->size; i++) {
        topic_free(topics->topics[i]);
    }
    free(topics->topics);
    free(topics);
}

void topics_array_print(struct topics *topics)
{
    printf("Topics array with %d topics:\n", topics->size);
    for (int i = 0; i < topics->size; i++) {
        printf("\t");
        topic_print(topics->topics[i]);
    }
}

void topics_array_add_topic(struct topics* topics, char *name)
{
    struct topic *topic = topic_create(name);
    topics->topics[topics->size] = topic;
    topics->size++;
}

void topics_array_remove_topic(struct topics *topics, char *name)
{
    int n = topics->size;
    if (n == 0) {
        // TODO: print to stderr.
        printf("Cannot remove topic from empty topic list.\n");
        return;
    }

    int i = 0;
    while (i < n && strcmp(topics->topics[i]->name, name) != 0)
        i++;
    
    if (i == n) {
        // TODO: print to stderr.
        printf("Cannot remove topic - topic not found.\n");
        return;
    }

    topic_free(topics->topics[i]);

    while (i < n - 1) {
        topics->topics[i] = topics->topics[i + 1];
        i++;
    }

    topics->size--;
}

struct topic *find_topic(struct topics* topics, char *name)
{
    struct topic *topic = NULL;
    for (int i = 0; i < topics->size; i++) {
        if (strcmp(topics->topics[i]->name, name) == 0) {
            topic = topics->topics[i];
            break;
        }
    }

    return topic;
}
