#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <math.h>

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
        printf("%s:%hu.\n", ip, port);
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
    // printf("\nEntered `UDP_parse_message`...\n");


    struct UDP_message *parsed_msg = (struct UDP_message *)destination;
    // Parse and terminate topic.
    memcpy(parsed_msg->topic, packet.message, TOPIC_SIZE);
    parsed_msg->topic[TOPIC_SIZE] = '\0';
    // Parse data type.
    memcpy(&parsed_msg->data_type, packet.message + TOPIC_SIZE, sizeof(parsed_msg->data_type));
    // Parse and terminate content.
    memcpy(parsed_msg->content, packet.message + TOPIC_SIZE + 1, CONTENT_MAXSIZE);
    parsed_msg->content[CONTENT_MAXSIZE] = '\0';

    // printf("Parsed topic: %s\n", parsed_msg->topic);
    // printf("Parsed data type: %d\n", parsed_msg->data_type);
    // printf("Parsed content: %s\n", parsed_msg->content);


    // printf("Exiting `UDP_parse_message`...\n\n");
}

void UDP_print_subscription_message(char *ip,
                                    uint16_t port,
                                    char *topic,
                                    uint8_t data_type,
                                    char *content)
{
    printf("---------------- UDP message ----------------\n");
    printf("%s:%hu - %s - ", ip, port, topic);

    uint8_t sign;
    switch (data_type) {
        case 0:
            sign = *(uint8_t *)content;
            uint32_t data_int = ntohl(*(uint32_t *)(content + 1));
            sign ? printf("INT - -%d\n", data_int) :
                   printf("INT - %d\n", data_int);
            break;
        case 1:
            uint16_t data_short_real = ntohs(*(uint16_t *)content);
            printf("SHORT_REAL - %.2f\n", (float)data_short_real / 100);
            break;
        case 2:
            sign = *(uint8_t *)content;
            uint32_t significand = ntohl(*(uint32_t *)(content + 1));
            uint8_t exponent = *(uint8_t *)(content + 5);
            float data_float = significand / pow(10, exponent);
            sign ? printf("FLOAT - -%.*f\n", exponent, data_float) :
                   printf("FLOAT - %.*f\n", exponent, data_float);
            break;
        case 3:
            printf("STRING - %s\n", content);
            break;
        default:
            printf("UNKNOWN - %s\n", content);
            break;
    }

    printf("---------------------------------------------\n");
}
