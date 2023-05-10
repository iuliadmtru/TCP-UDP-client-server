#ifndef _PACKETS_H_
#define _PACKETS_H_

#include <stdint.h>
#include <netinet/in.h>

#define TOPIC_LEN 50
#define CONTENT_MAXLEN 1500
#define UDP_MSG_MAXLEN 1551
#define TCP_MSG_MAXLEN 1599  // 1551 + 16 + 32
#define PACKET_MAXLEN 1615  // 1599 + 16
#define CTOS_MAXLEN 52  // TOPIC_LEN + <space> + <sf>

enum TCP_msg_type {TCP_MSG_NEW, TCP_MSG_SUBSCRIBE, TCP_MSG_UNSUBSCRIBE};
enum UDP_msg_data_type {DATA_TYPE_INT,
                        DATA_TYPE_SHORT_REAL,
                        DATA_TYPE_FLOAT,
                        DATA_TYPE_STRING,
                        DATA_TYPE_INVALID};

struct packet {
    char msg[PACKET_MAXLEN];
} __attribute__((packed));
// };

struct UDP_msg {
    char topic[TOPIC_LEN];
    uint8_t data_type;
    char content[CONTENT_MAXLEN];
} __attribute__((packed));

struct TCP_header {
    uint16_t msg_len;
    char msg[TCP_MSG_MAXLEN];  // TCP_ctos_msg || TCP_stoc_msg
} __attribute__((packed));
// };

/*
 * Client to server message.
 */
struct TCP_ctos_msg {
    uint8_t msg_type;  // new || subscribe || unsubscribe
    char payload[CTOS_MAXLEN];  // id || topic + sf
} __attribute__((packed));
// };

/*
 * Server to client message.
 */
struct TCP_stoc_msg {
    struct in_addr src_ip;
    uint16_t src_port;
    struct UDP_msg content;
} __attribute__((packed));

#endif  // _PACKETS_H_
