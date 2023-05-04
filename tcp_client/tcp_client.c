#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../util/common.h"
#include "../util/helpers.h"
#include "util.h"

void run_client(int sockfd)
{
    char buf[MSG_MAXSIZE + 1];
    memset(buf, 0, MSG_MAXSIZE + 1);

    struct packet sent_packet;
    struct packet recv_packet;

    struct pollfd poll_fds[MAX_CONNECTIONS];
    int num_fds = 0, rc;

    // Add stdin to poll for command reading.
    poll_fds[num_fds].fd = STDIN_FILENO;
    poll_fds[num_fds].events = POLLIN;
    num_fds++;

    // Add client socket.
    poll_fds[num_fds].fd = sockfd;
    poll_fds[num_fds].events = POLLIN;
    num_fds++;

    while (1) {
        rc = poll(poll_fds, num_fds, -1);
        DIE(rc < 0, "poll failed");

        if ((poll_fds[0].revents & POLLIN) != 0) {
            // Receive command from stdin.
            printf("\nReceived stdin command...\n");

            char tcp_cmd[CMD_MAXSIZE];
            scanf("%s", tcp_cmd);
            if (strcmp(tcp_cmd, "exit") == 0) {
                printf("Received exit command...\n");

                tcp_client_exit(sockfd, poll_fds, num_fds);
            } else if (strcmp(tcp_cmd, "subscribe") == 0) {
                // Send subscribe message to server.
                char topic[TOPIC_SIZE];
                uint8_t sf;
                scanf("%s", topic);
                scanf("%hhu", &sf);

                snprintf(sent_packet.message, TOPIC_SIZE + 4, "%hhu %s %hhu", 1, topic, sf);
                send_all(sockfd, &sent_packet, sizeof(sent_packet));
                TCP_client_print_subscription_status(1);

                printf("Sent subscribe message: %s...\n", sent_packet.message);
            } else if (strcmp(tcp_cmd, "unsubscribe") == 0) {
                // Send unsubscribe message to server.
                char topic[TOPIC_SIZE];
                scanf("%s", topic);

                snprintf(sent_packet.message, TOPIC_SIZE + 4, "%hhu %s %hhu", 0, topic, 0);
                send_all(sockfd, &sent_packet, sizeof(sent_packet));
                TCP_client_print_subscription_status(0);

                printf("Sent unsubscribe message: %s...\n", sent_packet.message);
            } else {
                printf("Unknown command\n");
            }
        } else if ((poll_fds[1].revents & POLLIN) != 0) {
            // New connection.
            int rc = recv_all(sockfd, &recv_packet, sizeof(recv_packet));
            DIE(rc < 0, "recv_all failed");

            printf("\nReceived new connection: %s...\n", recv_packet.message);

            if (rc == 0)  // Connection ended.
                tcp_client_exit(sockfd, poll_fds, num_fds);
        } else {
            // se trateaza mesajele de la un client...
            // TODO: Receive messages from UDP subscriptions.
        }
    }
}

int main(int argc, char *argv[])
{
    int sockfd = -1;

    if (argc != 4) {
        printf("\n Usage: %s <ID_CLIENT> <IP_SERVER> <PORT_SERVER>\n", argv[0]);
        return 1;
    }

    // Parse port number.
    uint16_t port;
    int rc = sscanf(argv[3], "%hu", &port);
    DIE(rc != 1, "Invalid port");

    // Open TCP socket.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket failed");

    // Set up the sockaddr_in struct.
    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);
    // Fill in the structure.
    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    rc = inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);
    DIE(rc <= 0, "inet_pton failed");

    // Connect to server.
    rc = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    DIE(rc < 0, "connect failed");

    run_client(sockfd);

    // Disconnect and close socket.
    close(sockfd);

    return 0;
}
