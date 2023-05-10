#ifndef _SUBSCRIPTIONS_H_
#define _SUBSCRIPTIONS_H_

#include "packets.h"
#include "clients.h"

typedef struct subscription_node {
    char topic[TOPIC_LEN];
    char subscriber_id[ID_MAXLEN];
    uint8_t store_and_forward;
    struct subscription_node *prev;
    struct subscription_node *next;
} subscription_node;

typedef struct subscriptions_list {
    subscription_node *head;
    struct subscription_node *current;
} subscriptions_list;

void subscription_node_print(subscription_node *subscription, int idx);

subscription_node *subscription_node_create(char *topic,
                                            char *id,
                                            uint8_t sf);

void subscription_node_destroy(subscription_node *subscription);

void subscriptions_print(subscriptions_list *subscriptions);

subscriptions_list *subscriptions_create();

void subscriptions_destroy(subscriptions_list *subscriptions);

void subscriptions_add_subscription(subscriptions_list *subscriptions,
                                    subscription_node *subscription);

void subscriptions_remove_subscription(subscriptions_list *subscriptions,
                                       subscription_node *subscription);

subscription_node *subscriptions_find(subscriptions_list *subscriptions,
                                      char *topic,
                                      char *subscriber_id);

subscription_node *subscriptions_get_next(subscriptions_list *subscriptions,
                                          char *topic);

#endif  // _SUBSCRIPTIONS_H_