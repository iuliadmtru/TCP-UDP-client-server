#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "subscriptions.h"

// ------------------------ SUBSCRIPTION_NODE ------------------------

void subscription_node_print(subscription_node *subscription, int idx)
{
    printf("Print subscription %d:\n", idx);
    printf("\ttopic: %s\n", subscription->topic);
    printf("\tsubscription_id: %s\n", subscription->subscriber_id);
    printf("\tstore_and_forward: %hhu\n", subscription->store_and_forward);
    subscription->prev ? printf("\tprev {topic, id}: {%s, %s}\n",
                                subscription->prev->topic,
                                subscription->prev->subscriber_id) :
                         printf("\tno prev\n");
    subscription->next ? printf("\tnext {topic, id}: {%s, %s}\n",
                                subscription->next->topic,
                                subscription->next->subscriber_id) :
                         printf("\tno next\n");
}

subscription_node *subscription_node_create(char *topic,
                                            char *id,
                                            uint8_t sf)
{
    subscription_node *subscription = malloc(sizeof(subscription_node));

    strcpy(subscription->topic, topic);
    strcpy(subscription->subscriber_id, id);
    subscription->store_and_forward = sf;
    subscription->prev = NULL;
    subscription->next = NULL;

    return subscription;
}

void subscription_node_destroy(subscription_node *subscription)
{
    free(subscription);
}


// ------------------------ SUBSCRIPTIONS_LIST ------------------------

void subscriptions_print(subscriptions_list *subscriptions)
{
    if (!subscriptions->head) {
        printf("Subscriptions list empty.\n");
        return;
    }

    printf("Print subscriptions list:\n");

    int idx = 0;
    subscription_node *current = subscriptions->head;
    while (current) {
        subscription_node_print(current, idx);
        idx++;
        current = current->next;
    }
}

subscriptions_list *subscriptions_create()
{
    subscriptions_list *subscriptions = malloc(sizeof(subscriptions_list));
    subscriptions->head = NULL;
    subscriptions->current = subscriptions->head;

    return subscriptions;
}

int subscriptions_list_is_empty(subscriptions_list *subscriptions)
{
    return subscriptions->head == NULL;
}

void subscriptions_destroy(subscriptions_list *subscriptions)
{
    while (!subscriptions_list_is_empty(subscriptions)) {
        subscriptions_remove_subscription(subscriptions, subscriptions->head);
    }
    free(subscriptions);
}

void subscriptions_add_subscription(subscriptions_list *subscriptions,
                                    subscription_node *subscription)
{
    if (subscriptions_list_is_empty(subscriptions)) {
        subscriptions->head = subscription;

        // printf("Print subscriptions list after add:\n");
        // subscriptions_print(subscriptions);
        // printf("\n");

        return;
    }

    subscription->next = subscriptions->head;
    subscriptions->head->prev = subscription;
    subscriptions->head = subscription;

    // printf("Print subscriptions list after add:\n");
    // subscriptions_print(subscriptions);
    // printf("\n");
}

int subscriptions_list_is_head(subscriptions_list *subscriptions,
                               subscription_node *subscription)
{
    subscription_node *head = subscriptions->head;
    if (strcmp(head->topic, subscription->topic) == 0 &&
        strcmp(head->subscriber_id, subscription->subscriber_id) == 0 &&
        head->store_and_forward == subscription->store_and_forward &&
        head->prev == subscription->prev &&
        head->next == subscription->next) {
        return 1;
    }
    return 0;
}

void subscriptions_remove_subscription(subscriptions_list *subscriptions,
                                       subscription_node *subscription)
{
    if (subscriptions_list_is_empty(subscriptions)) {
        fprintf(stderr, "Cannot remove from empty subscriptions list\n");
        return;
    }
    if (!subscription) {
        fprintf(stderr, "Cannot remove NULL subscription\n");
        return;
    }

    if (subscriptions_list_is_head(subscriptions, subscription)) {
        subscriptions->head = subscriptions->head->next;
    } else {
        subscription->next->prev = subscription->prev;
        subscription->prev->next = subscription->next;
    }
    subscription_node_destroy(subscription);
}

subscription_node *subscriptions_find(subscriptions_list *subscriptions,
                                      char *topic,
                                      char *subscriber_id)
{
    if (subscriptions_list_is_empty(subscriptions)) {
        fprintf(stderr,
                "Cannot find subscription in empty subscriptions list\n");
        return NULL;
    }

    subscription_node *current = subscriptions->head;
    while (current) {
        if (strcmp(current->topic, topic) == 0 &&
            strcmp(current->subscriber_id, subscriber_id) == 0)
            return current;

        current = current->next;
    }

    return NULL;
}

subscription_node *subscriptions_get_next(subscriptions_list *subscriptions,
                                          char *topic)
{
    if (subscriptions_list_is_empty(subscriptions)) {
        fprintf(stderr,
                "Cannot get subscription from empty subscriptions list\n");
        return NULL;
    }

    subscription_node *current = subscriptions->current;
    while (current) {
        if (strcmp(current->topic, topic) == 0) {
            subscriptions->current = current->next;
            return current;
        }
        current = current->next;
    }

    return NULL;
}
