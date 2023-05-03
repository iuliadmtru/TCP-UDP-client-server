#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timerfd.h>

#include "util/common.h"
#include "util/helpers.h"
#include "server_utils/util.h"

void run_server(int UDP_clientfd, int TCP_clientfd)
{
    //////////////////////////////////////////// REMOVE ////////////////////////////////////////////
    printf("Entered `run_server`...\n");
    //////////////////////////////////////////// REMOVE ////////////////////////////////////////////


    struct pollfd poll_fds[MAX_CONNECTIONS];
    int num_clients = 0, rc;

    // Add stdin to poll.
    poll_fds[0].fd = STDIN_FILENO;
    poll_fds[0].events = POLLIN;
    num_clients++;

    struct packet received_packet;
    struct UDP_message UDP_parsed_msg;
    struct TCP_message TCP_parsed_msg;

    // Add UDP file descriptor to poll.
    poll_fds[1].fd = UDP_clientfd;
    poll_fds[1].events = POLLIN;
    num_clients++;

    // Listen for TCP clients.
    rc = listen(TCP_clientfd, MAX_CONNECTIONS);
    DIE(rc < 0, "listen failed");

    // Add TCP file descriptor to poll.
    poll_fds[2].fd = TCP_clientfd;
    poll_fds[2].events = POLLIN;
    num_clients++;

    while (1) {
        //////////////////////////////////////////// REMOVE ////////////////////////////////////////////
        printf("\nEntered server loop...\n");
        //////////////////////////////////////////// REMOVE ////////////////////////////////////////////

        rc = poll(poll_fds, num_clients, -1);
        DIE(rc < 0, "poll failed");

        for (int i = 0; i < num_clients; i++) {
            if (poll_fds[i].revents & POLLIN) {
                if (poll_fds[i].fd == 0) {
                    // Receive stdin command.
                    printf("Received stdin command...\n");
                    char server_cmd[CMD_MAXSIZE];
                    scanf("%s", server_cmd);
                    if (strcmp(server_cmd, "exit") == 0)
                        server_exit(poll_fds, num_clients);
                    else
                        printf("Unknown command\n");
                } else if (poll_fds[i].fd == UDP_clientfd) {
                    // Receive UDP packet.
                    struct sockaddr_in UDP_cli_addr;
                    socklen_t UDP_cli_len = sizeof(UDP_cli_addr);
                    int rc = recvfrom(UDP_clientfd,
                                    &received_packet,
                                    sizeof(struct packet),
                                    0,
                                    (struct sockaddr *)&UDP_cli_addr,
                                    &UDP_cli_len);
                    DIE(rc < 0, "recvfrom failed");

                    //////////////////////////////////////////// REMOVE ////////////////////////////////////////////
                    printf("Received UDP packet...\n");
                    //////////////////////////////////////////// REMOVE ////////////////////////////////////////////

                    // Parse UDP message.
                    UDP_parse_message(received_packet, &UDP_parsed_msg);

                    //////////////////////////////////////////// REMOVE ////////////////////////////////////////////
                    UDP_print_subscription_message(inet_ntoa(UDP_cli_addr.sin_addr),
                                                ntohs(UDP_cli_addr.sin_port),
                                                UDP_parsed_msg.topic,
                                                UDP_parsed_msg.data_type,
                                                UDP_parsed_msg.content);
                    //////////////////////////////////////////// REMOVE ////////////////////////////////////////////

                    // TODO: send message to all subscribed TCP clients.
                } else if (poll_fds[i].fd == TCP_clientfd) {
                    // New TCP connection.
                    struct sockaddr_in cli_addr;
                    socklen_t cli_len = sizeof(cli_addr);
                    int newsockfd = accept(TCP_clientfd,
                                           (struct sockaddr *)&cli_addr,
                                           &cli_len);
                    DIE(newsockfd < 0, "accept failed");

                    //////////////////////////////////////////// REMOVE ////////////////////////////////////////////
                    printf("Received TCP connection...\n");
                    //////////////////////////////////////////// REMOVE ////////////////////////////////////////////

                    // Add new socket to read file descriptors in poll.
                    poll_fds[num_clients].fd = newsockfd;
                    poll_fds[num_clients].events = POLLIN;
                    num_clients++;

                    server_print_connection_status(1,
                                                   newsockfd,
                                                   inet_ntoa(cli_addr.sin_addr),
                                                   ntohs(cli_addr.sin_port));
                } else {
                    // Receive message on one of the client sockets.
                    //////////////////////////////////////////// REMOVE ////////////////////////////////////////////
                    printf("Received TCP message...\n");
                    //////////////////////////////////////////// REMOVE ////////////////////////////////////////////

                    // TODO: check why this is not working for 2 clients
                    // blocks here after the second client connects
                    // problem at `recv_all`

                    int rc = recv_all(poll_fds[i].fd,
                                      &received_packet,
                                      sizeof(received_packet));
                    DIE(rc < 0, "recv_all failed");

                    if (rc == 0) {
                        //////////////////////////////////////////// REMOVE ////////////////////////////////////////////
                        printf("TCP connection closed...\n");
                        //////////////////////////////////////////// REMOVE ////////////////////////////////////////////

                        // Client disconnected.
                        server_print_connection_status(0, poll_fds[i].fd, 0, 0);
                        close(poll_fds[i].fd);

                        // Remove closed socket from poll.
                        for (int j = i; j < num_clients - 1; j++)
                        {
                            poll_fds[j] = poll_fds[j + 1];
                        }
                        num_clients--;
                    } else {
                        //////////////////////////////////////////// REMOVE ////////////////////////////////////////////
                        printf("Treat TCP message...\n");
                        //////////////////////////////////////////// REMOVE ////////////////////////////////////////////

                        TCP_parse_message(received_packet, &TCP_parsed_msg);
                        // TODO: treat subscription messages from TCP clients.

                        // UDP_print_subscription_message();
                        printf("S-a primit de la clientul de pe socketul %d mesajul: %s\n",
                                     poll_fds[i].fd, received_packet.message);

                        // /* TODO 2.1: Trimite mesajul catre toti ceilalti clienti */
                        // for (int j = 0; j < num_clients; j++)
                        // {
                        //     if (j != i && poll_fds[j].fd != UDP_clientfd)
                        //     {
                        //         send_all(poll_fds[j].fd, &received_packet, sizeof(received_packet));
                        //     }
                        // }
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("\n Usage: %s <IP_ADDRESS> <PORT>\n", argv[0]);
        return 1;
    }

    // Parse port number.
    uint16_t port;
    int rc = sscanf(argv[2], "%hu", &port);
    DIE(rc != 1, "Invalid port");

    // Set up the sockaddr_in struct.
    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);
    // Fill in the structure.
    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    rc = inet_pton(AF_INET, argv[1], &serv_addr.sin_addr.s_addr);
    DIE(rc <= 0, "inet_pton failed");

    // ---------------------- UDP ---------------------- 
    // Open UDP socket.
    int UDP_clientfd = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(UDP_clientfd < 0, "socket failed");

    // Make the socket address reusable.
    int enable = 1;
    if (setsockopt(UDP_clientfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    // Bind UDP socket.
    rc = bind(UDP_clientfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    DIE(rc < 0, "bind failed");

    // ---------------------- TCP ---------------------- 
    // Open TCP socket.
    int TCP_clientfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(TCP_clientfd < 0, "socket failed");

    // Make the socket address reusable.
    enable = 1;
    if (setsockopt(TCP_clientfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    // Bind TCP socket.
    rc = bind(TCP_clientfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    DIE(rc < 0, "bind failed");


    run_server(UDP_clientfd, TCP_clientfd);

    // Close all connections.
    close(UDP_clientfd);
    close(TCP_clientfd);

    return 0;
}
