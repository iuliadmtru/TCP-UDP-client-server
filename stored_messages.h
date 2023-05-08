#ifndef _STORED_MSGS_H_
#define _STORED_MSGS_H_

#include "clients.h"
#include "packets.h"

typedef struct stored_messages_list {
    stored_messages_node *head;
} stored_messages_list;

typedef struct stored_messages_node {
    char client_id[ID_MAXLEN];
    struct packet *packet;
    stored_messages_node *prev;
    stored_messages_node *next;
} stored_messages_node;

stored_messages_node *stored_messages_node_create(char *id,
                                                  struct packet *packet,
                                                  stored_messages_node *prev);

void stored_messages_node_destroy(stored_messages_node *stored_msg);

stored_messages_list *stored_messages_list_create();

void stored_messages_list_destroy(stored_messages_node *stored_msgs);

void stored_messages_list_add_msg(stored_messages_node *stored_msgs,
                                  stored_messages_node *stored_msg);

void stored_messages_list_remove_msg(stored_messages_node *stored_msgs,
                                     stored_messages_node *stored_msg);

#endif  // _STORED_MSGS_H_