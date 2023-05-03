/*
 * Protocoale de comunicatii
 * Laborator 7 - TCP
 * Echo Server
 * server.c
 */

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

#include "common.h"
#include "helpers.h"

#define MAX_CONNECTIONS 32

// // Primeste date de pe connfd1 si trimite mesajul receptionat pe connfd2
// int receive_and_send(int connfd1, int connfd2, size_t len)
// {
//     int bytes_received;
//     char buffer[len];

//     // Primim exact len octeti de la connfd1
//     bytes_received = recv_all(connfd1, buffer, len);
//     // S-a inchis conexiunea
//     if (bytes_received == 0)
//     {
//         return 0;
//     }
//     DIE(bytes_received < 0, "recv");

//     // Trimitem mesajul catre connfd2
//     int rc = send_all(connfd2, buffer, len);
//     if (rc <= 0)
//     {
//         perror("send_all");
//         return -1;
//     }

//     return bytes_received;
// }

// void run_chat_server(int listenfd)
// {
//     struct sockaddr_in client_addr1;
//     struct sockaddr_in client_addr2;
//     socklen_t clen1 = sizeof(client_addr1);
//     socklen_t clen2 = sizeof(client_addr2);

//     int connfd1 = -1;
//     int connfd2 = -1;
//     int rc;

//     // Setam socket-ul listenfd pentru ascultare
//     rc = listen(listenfd, 2);
//     DIE(rc < 0, "listen");

//     // Acceptam doua conexiuni
//     printf("Astept conectarea primului client...\n");
//     connfd1 = accept(listenfd, (struct sockaddr *)&client_addr1, &clen1);
//     DIE(connfd1 < 0, "accept");

//     printf("Astept connectarea clientului 2...\n");

//     connfd2 = accept(listenfd, (struct sockaddr *)&client_addr2, &clen2);
//     DIE(connfd2 < 0, "accept");

//     while (1)
//     {
//         // Primim de la primul client, trimitem catre al 2lea
//         printf("Primesc de la 1 si trimit catre 2...\n");
//         int rc = receive_and_send(connfd1, connfd2, sizeof(struct chat_packet));
//         if (rc <= 0)
//         {
//             break;
//         }

//         rc = receive_and_send(connfd2, connfd1, sizeof(struct chat_packet));
//         if (rc <= 0)
//         {
//             break;
//         }
//     }

//     // Inchidem conexiunile si socketii creati
//     close(connfd1);
//     close(connfd2);
// }

void run_server(int UDP_listenfd, int TCP_listenfd)
{
    //////////////////////////////////////////// REMOVE ////////////////////////////////////////////
    printf("Entered `run_server`...\n");
    //////////////////////////////////////////// REMOVE ////////////////////////////////////////////


    struct pollfd poll_fds[MAX_CONNECTIONS];
    int num_clients = 1;
    int rc;

    struct UDP_packet received_packet;
    struct UDP_message parsed_msg;

    // // Set listenfd pentru ascultare
    // rc = listen(UDP_listenfd, MAX_CONNECTIONS);
    // DIE(rc < 0, "listen failed");

    // Add UDP file descriptor to poll.
    poll_fds[0].fd = UDP_listenfd;
    poll_fds[0].events = POLLIN;

    while (1) {
        //////////////////////////////////////////// REMOVE ////////////////////////////////////////////
        printf("\nEntered server loop...\n");
        //////////////////////////////////////////// REMOVE ////////////////////////////////////////////

        rc = poll(poll_fds, num_clients, -1);
        DIE(rc < 0, "poll failed");

        // Receive UDP packet.
        struct sockaddr_in UDP_cli_addr;
        socklen_t UDP_cli_len = sizeof(UDP_cli_addr);
        int rc = recvfrom(UDP_listenfd,
                          &received_packet,
                          sizeof(struct UDP_packet),
                          0,
                          (struct sockaddr *)&UDP_cli_addr,
                          &UDP_cli_len);
        DIE(rc < 0, "recvfrom failed");

        //////////////////////////////////////////// REMOVE ////////////////////////////////////////////
        printf("Received UDP packet...\n");
        //////////////////////////////////////////// REMOVE ////////////////////////////////////////////

        // Parse UDP message.
        UDP_parse_message(received_packet, &parsed_msg);

        //////////////////////////////////////////// REMOVE ////////////////////////////////////////////
        UDP_print_subscription_message(inet_ntoa(UDP_cli_addr.sin_addr),
                                       ntohs(UDP_cli_addr.sin_port),
                                       parsed_msg.topic,
                                       parsed_msg.data_type,
                                       parsed_msg.content);
        //////////////////////////////////////////// REMOVE ////////////////////////////////////////////

        // for (int i = 0; i < num_clients; i++) {
        for (int i = 0; i < 0; i++) {
            if (poll_fds[i].revents & POLLIN) {
                if (poll_fds[i].fd == UDP_listenfd) {
                    // New UDP connection.
                    struct sockaddr_in cli_addr;
                    socklen_t cli_len = sizeof(cli_addr);
                    int rc = recvfrom(UDP_listenfd, &received_packet, sizeof(struct UDP_packet), 0,
                                        (struct sockaddr *)&cli_addr, &cli_len);
                    DIE(rc < 0, "recvfrom failed");
                    // int newsockfd =
                    //         accept(UDP_listenfd, (struct sockaddr *)&cli_addr, &cli_len);
                    // DIE(newsockfd < 0, "accept failed");

                    // // Add new socket to read file descriptors in poll.
                    // poll_fds[num_clients].fd = newsockfd;
                    // poll_fds[num_clients].events = POLLIN;
                    // num_clients++;

                    // server_print_connection_status(1,
                    //                                newsockfd,
                    //                                inet_ntoa(cli_addr.sin_addr),
                    //                                ntohs(cli_addr.sin_port));
                    // // printf("Noua conexiune de la %s, port %d, socket client %d\n",
                    // //              inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port),
                    // //              newsockfd);
                } else if (poll_fds[i].fd == TCP_listenfd) {
                    // for (int j = 0; j < num_clients; j++)
                    // {
                    //     if (j != i && poll_fds[j].fd != listenfd)
                    //     {
                    //         send_all(poll_fds[j].fd, &timed_packet, sizeof(timed_packet));
                    //     }
                    // }
                } else {
                    // Receive message on one of the client sockets.
                    int rc = recv_all(poll_fds[i].fd,
                                      &received_packet,
                                      sizeof(received_packet));
                    DIE(rc < 0, "recv_all failed");
                    UDP_parse_message(received_packet, &parsed_msg);

                    if (rc == 0) {
                        // Client disconnected.
                        server_print_connection_status(0, poll_fds[i].fd, 0, 0);
                        // printf("Socket-ul client %d a inchis conexiunea\n", i);
                        close(poll_fds[i].fd);

                        // Remove closed socket from poll.
                        for (int j = i; j < num_clients - 1; j++)
                        {
                            poll_fds[j] = poll_fds[j + 1];
                        }

                        num_clients--;
                    } else {
                        // UDP_print_subscription_message();
                        printf("S-a primit de la clientul de pe socketul %d mesajul: %s\n",
                                     poll_fds[i].fd, received_packet.message);

                        // /* TODO 2.1: Trimite mesajul catre toti ceilalti clienti */
                        // for (int j = 0; j < num_clients; j++)
                        // {
                        //     if (j != i && poll_fds[j].fd != UDP_listenfd)
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
    if (argc != 3)
    {
        printf("\n Usage: %s <ip_address> <port>\n", argv[0]);
        return 1;
    }

    // Parse port number.
    uint16_t port;
    int rc = sscanf(argv[2], "%hu", &port);
    DIE(rc != 1, "Invalid port");

    // Open UDP socket.
    int UDP_listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(UDP_listenfd < 0, "socket failed");

    // Set up the sockaddr_in struct.
    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);

    // Make the socket address reusable.
    int enable = 1;
    if (setsockopt(UDP_listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    // Fill in the structure.
    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    rc = inet_pton(AF_INET, argv[1], &serv_addr.sin_addr.s_addr);
    DIE(rc <= 0, "inet_pton failed");

    // Bind socket.
    rc = bind(UDP_listenfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    DIE(rc < 0, "bind failed");

    run_server(UDP_listenfd, 0);

    // Close all connections.
    close(UDP_listenfd);

    return 0;
}
