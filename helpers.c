#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "helpers.h"
#include "common.h"

void print_ip_address(uint32_t ip_addr) {
    uint8_t *ip_bytes = (uint8_t *) &ip_addr;
    printf("%d.%d.%d.%d", ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3]);
}

void server_print_connection_status(int connected,
                                    int id,
                                    char *ip,
                                    uint16_t port)
{
    if (connected) {
        printf("New client %d connected from ", id);
        // print_ip_address(ip);
        printf("%s", ip);
        printf(":%hu.\n", port);
    } else {
        printf("Client %d disconnected.\n", id);
    }
}

void TCP_client_print_subscription_status(int subscribed)
{
    char *msg;
    msg = subscribed ? "Subscribed to topic." : "Unsubscribed from topic.";
    printf("%s\n", msg);
}

void UDP_parse_message(struct UDP_packet packet, void *destination)
{
    struct UDP_message *parsed_msg = (struct UDP_message *)destination;
    // Parse and terminate topic.
    memcpy(&parsed_msg->topic, &packet.message, TOPIC_SIZE);
    parsed_msg->topic[TOPIC_SIZE] = '\0';
    // Parse data type.
    memcpy(&parsed_msg->data_type, &packet.message + TOPIC_SIZE, 1);
    // Parse and terminate content.
    memcpy(&parsed_msg->content, &packet.message + TOPIC_SIZE + 1, CONTENT_MAXSIZE);
    parsed_msg->content[CONTENT_MAXSIZE] = '\0';
}

void UDP_print_subscription_message(char *ip,
                                    uint16_t port,
                                    char *topic,
                                    uint8_t data_type,
                                    char *content)
{
    char *data_type_str;
    switch (data_type) {
        case 0:
            data_type_str = "INT";
            break;
        case 1:
            data_type_str = "SHORT_REAL";
            break;
        case 2:
            data_type_str = "FLOAT";
            break;
        case 3:
            data_type_str = "STRING";
            break;
        default:
            data_type_str = "UNKNOWN";
            break;
    }

    // print_ip_address(ip);
    printf("%s", ip);
    printf(":%hu - %s - %s - %s\n", port, topic, data_type_str, content);
}
