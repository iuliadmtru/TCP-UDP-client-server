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
                                    char *id,
                                    char *ip,
                                    uint16_t port)
{
    if (connected) {
        printf("New client %s connected from ", id);
        printf("%s:%hu.\n", ip, port);
    } else {
        printf("Client %s disconnected.\n", id);
    }
}

void TCP_client_print_subscription_status(int subscribed)
{
    char *msg;
    msg = subscribed ? "Subscribed to topic." : "Unsubscribed from topic.";
    printf("%s\n", msg);
}

void UDP_parse_message(struct packet packet,
                       char *ip,
                       uint16_t *port,
                       void *destination)
{
    printf("\nEntered `UDP_parse_message`...\n");


    struct UDP_message *parsed_msg = (struct UDP_message *)destination;
    char *addr = packet.message;

    // Copy ip address.
    memcpy(parsed_msg->ip, ip, 15);
    // Copy port.
    memcpy(&parsed_msg->port, (char *)port, sizeof(parsed_msg->port));
    // Parse and terminate topic.
    memcpy(parsed_msg->topic, addr, sizeof(parsed_msg->topic));
    addr += sizeof(parsed_msg->topic);
    // parsed_msg->topic[TOPIC_SIZE] = '\0';
    // Parse data type.
    memcpy(&parsed_msg->data_type, addr, sizeof(parsed_msg->data_type));
    addr += sizeof(parsed_msg->data_type);
    // Parse and terminate content.
    memcpy(parsed_msg->content, addr, sizeof(parsed_msg->content));
    parsed_msg->content[CONTENT_MAXSIZE] = '\0';

    printf("Parsed ip: %s\n", parsed_msg->ip);
    printf("Parsed port: %d\n", parsed_msg->port);
    printf("Parsed topic: %s\n", parsed_msg->topic);
    printf("Parsed data type: %d\n", parsed_msg->data_type);
    printf("Parsed content: %s\n", parsed_msg->content);


    printf("Exiting `UDP_parse_message`...\n\n");
}

void packet_from_UDP_msg(struct packet *packet, struct UDP_message UDP_msg)
{
    printf("\nEntered `packet_from_UDP_message`...\n");

    // sprintf(packet->message,
    //         "%s %hu %s %hhu %s",
    //         UDP_msg.ip,
    //         UDP_msg.port,
    //         UDP_msg.topic,
    //         UDP_msg.data_type,
    //         UDP_msg.content);

    uint8_t sign;
    switch (UDP_msg.data_type) {
        case 0:
            sign = *(uint8_t *)UDP_msg.content;
            uint32_t data_int = htonl(*(uint32_t *)(UDP_msg.content + 1));
            sprintf(packet->message,
                    "%s %hu %s %hhu %hhu %d",
                    UDP_msg.ip,
                    UDP_msg.port,
                    UDP_msg.topic,
                    UDP_msg.data_type,
                    sign,
                    data_int);
            break;
        case 1:
            uint16_t data_short_real = *(uint16_t *)UDP_msg.content;
            // printf("SHORT_REAL - %.2f\n", (float)data_short_real / 100);
            sprintf(packet->message,
                    "%s %hu %s %hhu %hu",
                    UDP_msg.ip,
                    UDP_msg.port,
                    UDP_msg.topic,
                    UDP_msg.data_type,
                    data_short_real);
            break;
        case 2:
            sign = *(uint8_t *)UDP_msg.content;
            uint32_t significand = *(uint32_t *)(UDP_msg.content + 1);
            uint8_t exponent = *(uint8_t *)(UDP_msg.content + 5);
            // float data_float = significand / pow(10, exponent);
            sprintf(packet->message,
                    "%s %hu %s %hhu %hhu %d %hhu",
                    UDP_msg.ip,
                    UDP_msg.port,
                    UDP_msg.topic,
                    UDP_msg.data_type,
                    sign,
                    significand,
                    exponent);
            break;
        case 3:
            sprintf(packet->message,
                    "%s %hu %s %hhu %s",
                    UDP_msg.ip,
                    UDP_msg.port,
                    UDP_msg.topic,
                    UDP_msg.data_type,
                    UDP_msg.content);
            break;
        default:
            sprintf(packet->message,
                    "%s %hu %s %hhu %s",
                    UDP_msg.ip,
                    UDP_msg.port,
                    UDP_msg.topic,
                    UDP_msg.data_type,
                    "INVALID");
            break;
    }

    // char *addr_sender_details = packet->sender_details;
    // // Parse ip.
    // memcpy(addr_sender_details, UDP_msg.ip, sizeof(UDP_msg.ip));
    // addr_sender_details += sizeof(UDP_msg.ip);
    // // Parse port.
    // sprintf(addr_sender_details, "%d", UDP_msg.port);

    // char *addr_msg = packet->message;
    // // Parse ip.
    // memcpy(addr_msg, UDP_msg.ip, sizeof(UDP_msg.ip));
    // addr_msg += sizeof(UDP_msg.ip);
    // // Parse port.
    // char port[6];
    // sprintf(port, "%hu", UDP_msg.port);
    // memcpy(addr_msg, port, sizeof(port));

    // char test[10000];
    // sprintf(test, "%s %d", packet->message, UDP_msg.port);
    // printf("Test buffer: %s\n", test);

    // printf("Port %hu was parsed as %s\n", UDP_msg.port, addr_msg);
    // printf("Packet so far: %s\n", packet->message);

    // addr_msg += sizeof(port);
    // // Parse topic.
    // memcpy(addr_msg, UDP_msg.topic, sizeof(UDP_msg.topic));
    // addr_msg += sizeof(UDP_msg.topic);
    // // Parse data type.
    // sprintf(addr_msg, " %hhu ", UDP_msg.data_type);
    // // Parse content.
    // memcpy(addr_msg, UDP_msg.content, sizeof(UDP_msg.content));

    printf("UDP_msg.content: %s\n", UDP_msg.content);
    printf("Parsed packet: %s\n", packet->message);

    printf("Exiting `packet_from_UDP_message`...\n\n");

    // sprintf(packet->sender_details,
    //         "%s %hu",
    //         UDP_msg.ip,
    //         UDP_msg.port);

    // sprintf(packet->message,
    //         "%s %hhu %s",
    //         udp_msg.topic,
    //         udp_msg.data_type,
    //         udp_msg.content);
}

void UDP_msg_from_packet(struct UDP_message *UDP_msg, struct packet packet)
{
    char content[CONTENT_MAXSIZE];
    sscanf(packet.message,
           "%s %hu %s %hhu %s",
           UDP_msg->ip,
           &UDP_msg->port,
           UDP_msg->topic,
           &UDP_msg->data_type,
           content);

    uint8_t sign;
    switch (UDP_msg->data_type) {
        case 0:
            sign = *(uint8_t *)content;
            uint32_t data_int = *(uint32_t *)(content + 1);
            sprintf(UDP_msg->content, "%hhu%d", sign, data_int);
            break;
        case 1:
            uint16_t data_short_real = *(uint16_t *)UDP_msg->content;
            // printf("SHORT_REAL - %->2f\n", (float)data_short_real / 100);
            sprintf(packet.message,
                    "%s %hu %s %hhu %hu",
                    UDP_msg->ip,
                    UDP_msg->port,
                    UDP_msg->topic,
                    UDP_msg->data_type,
                    data_short_real);
            break;
        case 2:
            sign = *(uint8_t *)UDP_msg->content;
            uint32_t significand = *(uint32_t *)(UDP_msg->content + 1);
            uint8_t exponent = *(uint8_t *)(UDP_msg->content + 5);
            // float data_float = significand / pow(10, exponent);
            sprintf(packet.message,
                    "%s %hu %s %hhu %hhu %d %hhu",
                    UDP_msg->ip,
                    UDP_msg->port,
                    UDP_msg->topic,
                    UDP_msg->data_type,
                    sign,
                    significand,
                    exponent);
            break;
        case 3:
            sprintf(UDP_msg->content, "%s", content);
            break;
        default:
            sprintf(packet.message,
                    "%s %hu %s %hhu %s",
                    UDP_msg->ip,
                    UDP_msg->port,
                    UDP_msg->topic,
                    UDP_msg->data_type,
                    "INVALID");
            break;
    }
}

void UDP_print_subscription_message(struct UDP_message UDP_msg)
{
    printf("---------------- UDP message ----------------\n");
    printf("%s:%hu - %s - ", UDP_msg.ip, UDP_msg.port, UDP_msg.topic);

    uint8_t sign;
    switch (UDP_msg.data_type) {
        case 0:
            sign = *(uint8_t *)UDP_msg.content;
            uint32_t data_int = ntohl(*(uint32_t *)(UDP_msg.content + 1));
            sign ? printf("INT - -%d\n", data_int) :
                   printf("INT - %d\n", data_int);
            break;
        case 1:
            uint16_t data_short_real = ntohs(*(uint16_t *)UDP_msg.content);
            printf("SHORT_REAL - %.2f\n", (float)data_short_real / 100);
            break;
        case 2:
            sign = *(uint8_t *)UDP_msg.content;
            uint32_t significand = ntohl(*(uint32_t *)(UDP_msg.content + 1));
            uint8_t exponent = *(uint8_t *)(UDP_msg.content + 5);
            float data_float = significand / pow(10, exponent);
            sign ? printf("FLOAT - -%.*f\n", exponent, data_float) :
                   printf("FLOAT - %.*f\n", exponent, data_float);
            break;
        case 3:
            printf("STRING - %s\n", UDP_msg.content);
            break;
        default:
            printf("UNKNOWN - %s\n", UDP_msg.content);
            break;
    }

    printf("---------------------------------------------\n");
}

void TCP_parse_message(struct packet packet, void *destination)
{
    printf("\nEntered `TCP_parse_message`...\n");


    struct TCP_message *parsed_msg = (struct TCP_message *)destination;
    sscanf(packet.message,
           "%s %hhu %s %hhu",
           parsed_msg->id,
           &parsed_msg->subscribe_status,
           parsed_msg->topic,
           &parsed_msg->sf);

    printf("Parsed id: %s\n", parsed_msg->id);
    printf("Parsed subscribe status: %d\n", parsed_msg->subscribe_status);
    printf("Parsed topic: %s\n", parsed_msg->topic);
    printf("Parsed sf: %d\n", parsed_msg->sf);


    printf("Exiting `TCP_parse_message`...\n\n");
}
