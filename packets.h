#ifndef _PACKETS_H_
#define _PACKETS_H_

#include <stdint.h>
#include <netinet/in.h>

#define TOPIC_LEN 50
#define CONTENT_MAXLEN 1500
#define MSG_MAXLEN 1551
#define PACKET_MAXLEN 1583  // 1551 + 32 -> sizeof TCP_header

struct packet {
    char msg[PACKET_MAXLEN];
} __attribute__((packed));

struct UDP_msg {
    char topic[TOPIC_LEN];
    uint8_t data_type;
    char content[CONTENT_MAXLEN];
} __attribute__((packed));

struct TCP_header {
    uint32_t msg_len;
    char msg[MSG_MAXLEN];
} __attribute__((packed));

/*
 * Client to server message.
 */
struct TCP_ctos_msg {
    struct TCP_header hdr;
    uint8_t msg_type;  // new || subscribe || unsubscribe
    char payload[TOPIC_LEN];  // id || topic
} __attribute__((packed));

/*
 * Server to client message.
 */
struct TCP_stoc_msg {
    struct TCP_header hdr;
    struct in_addr src_ip;
    uint16_t src_port;
    struct UDP_msg content;
} __attribute__((packed));

#endif  // _PACKETS_H_
