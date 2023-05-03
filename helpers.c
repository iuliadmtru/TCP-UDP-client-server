#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void print_ip_address(uint32_t ip_addr) {
    uint8_t *ip_bytes = (uint8_t *) &ip_addr;
    printf("%d.%d.%d.%d", ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3]);
}

void server_print_connection_status(int connected,
                                    int id,
                                    uint32_t ip,
                                    int port)
{
    if (connected) {
        printf("New client %d connected from ", id);
        print_ip_address(ip);
        printf(":%d.\n", port);
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

void TCP_client_print_subscription_message(uint32_t ip,
                                           int port,
                                           char *topic,
                                           char *data_type,
                                           char *message)
{
    print_ip_address(ip);
    printf(":%d - %s - %s - %s\n", port, topic, data_type, message);
}
