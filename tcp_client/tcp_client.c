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

void run_client(int sockfd)
{
    char buf[MSG_MAXSIZE + 1];
    memset(buf, 0, MSG_MAXSIZE + 1);

    struct packet sent_packet;
    struct packet recv_packet;

    struct pollfd poll_fds[MAX_CONNECTIONS];
    int nfds = 0, rc;

    // Add stdin to poll for command reading.
    poll_fds[nfds].fd = STDIN_FILENO;
    poll_fds[nfds].events = POLLIN;
    nfds++;

    // Add client socket.
    poll_fds[nfds].fd = sockfd;
    poll_fds[nfds].events = POLLIN;
    nfds++;

    while (1) {
        rc = poll(poll_fds, nfds, -1);
        DIE(rc < 0, "poll failed");

        if ((poll_fds[0].revents & POLLIN) != 0) {
            // Receive command from stdin.
            printf("Received stdin command...\n");
            char tcp_cmd[CMD_MAXSIZE];
            scanf("%s", tcp_cmd);
            if (strcmp(tcp_cmd, "exit") == 0) {
                // server_exit(poll_fds, num_clients);
            } else if (strcmp(tcp_cmd, "subscribe") == 0) {
                // Send subscribe message to server.
                char buf[TOPIC_SIZE + 4];

                char topic[TOPIC_SIZE];
                int sf;
                scanf("%s", topic);
                scanf("%d", &sf);

                snprintf(sent_packet.message, sizeof(buf), "%d %s %d", 1, topic, sf);
                printf("Sending subscribe message: %s...\n", sent_packet.message);

                // memcpy(sent_packet.message, buf, sizeof(buf));

                send_all(sockfd, &sent_packet, sizeof(sent_packet));
            } else if (strcmp(tcp_cmd, "unsubscribe") == 0) {
                // Send unsubscribe message to server.
                char buf[TOPIC_SIZE + 2];
                strcpy(buf, "0 ");

                char topic[TOPIC_SIZE];
                scanf("%s", topic);
                strcat(buf, topic);
                memcpy(sent_packet.message, buf, sizeof(buf));

                printf("Sending unsubscribe message: %s...\n", sent_packet.message);

                send_all(sockfd, &sent_packet, sizeof(sent_packet));
            } else {
                printf("Unknown command\n");
            }
        } else if ((poll_fds[1].revents & POLLIN) != 0) {
            // se trateaza noua conexiune...
            int rc = recv_all(sockfd, &recv_packet, sizeof(recv_packet));
            printf("Received new connection: %s...\n", recv_packet.message);
        } else {
            // se trateaza mesajele de la un client...
        }
    }

    // while (fgets(buf, sizeof(buf), stdin) && !isspace(buf[0]))
    // {
    //     sent_packet.len = strlen(buf) + 1;
    //     strcpy(sent_packet.message, buf);

    //     // Use send_all function to send the pachet to the server.
    //     send_all(sockfd, &sent_packet, sizeof(sent_packet));

    //     // Receive a message and show it's content
    //     int rc = recv_all(sockfd, &recv_packet, sizeof(recv_packet));
    //     if (rc <= 0)
    //     {
    //         break;
    //     }

    //     printf("%s\n", recv_packet.message);
    // }
}

int main(int argc, char *argv[])
{
    int sockfd = -1;

    if (argc != 4)
    {
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
