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

    struct pollfd poll_fds[MAX_CONNECTIONS];
    int num_clients = 1;
    int rc;

    struct UDP_packet received_packet;
    struct UDP_message parsed_msg;
    UDP_parse_message(received_packet, &parsed_msg);
    // struct UDP_packet timed_packet;
    // char str[] = "Dragi clienti, pentru doar 12 lei o sa puteti trimite de 10 ori mai multe mesaje in jumatate din timp\n";
    // strcpy(timed_packet.message, str);
    // timed_packet.len = strlen(str);

    // Set listenfd pentru ascultare
    rc = listen(UDP_listenfd, MAX_CONNECTIONS);
    DIE(rc < 0, "listen failed");

    // se adauga noul file descriptor (socketul pe care se asculta conexiuni) in
    // multimea read_fds
    poll_fds[0].fd = UDP_listenfd;
    poll_fds[0].events = POLLIN;

    // // adaugă timerfd
    // poll_fds[1].fd = timerfd;
    // poll_fds[0].events = POLLIN;

    while (1) {
        rc = poll(poll_fds, num_clients, -1);
        DIE(rc < 0, "poll failed");

        for (int i = 0; i < num_clients; i++) {
            if (poll_fds[i].revents & POLLIN) {
                if (poll_fds[i].fd == UDP_listenfd) {
                    // New UDP connection.
                    struct sockaddr_in cli_addr;
                    socklen_t cli_len = sizeof(cli_addr);
                    int newsockfd =
                            accept(UDP_listenfd, (struct sockaddr *)&cli_addr, &cli_len);
                    DIE(newsockfd < 0, "accept failed");

                    // Add new socket to read file descriptors in poll.
                    poll_fds[num_clients].fd = newsockfd;
                    poll_fds[num_clients].events = POLLIN;
                    num_clients++;

                    server_print_connection_status(1,
                                                   newsockfd,
                                                   inet_ntoa(cli_addr.sin_addr),
                                                   ntohs(cli_addr.sin_port));
                    // printf("Noua conexiune de la %s, port %d, socket client %d\n",
                    //              inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port),
                    //              newsockfd);
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
    if (argc != 2)
    {
        printf("\n Usage: %s <port>\n", argv[0]);
        return 1;
    }

    // Parse port number.
    uint16_t port;
    int rc = sscanf(argv[1], "%hu", &port);
    DIE(rc != 1, "Invalid port");

    // Open UDP socket.
    int UDP_listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(UDP_listenfd < 0, "Opening UDP socket failed");

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

    // // timerfd
    // int timerfd;
    // timerfd = timerfd_create(CLOCK_REALTIME, 0);

    // struct itimerspec spec;
    // spec.it_value.tv_sec = 4;
    // spec.it_value.tv_nsec = 0;
    // spec.it_interval.tv_sec = 4;
    // spec.it_interval.tv_nsec = 0;
    // timerfd_settime(timerfd, 0, &spec, NULL);

    // // run_chat_server(listenfd);
    // run_chat_multi_server(listenfd, timerfd);

    // Close all connections.
    close(UDP_listenfd);

    return 0;
}
