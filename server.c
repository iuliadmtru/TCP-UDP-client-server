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
#include "tcp_client/common.h"

#define DEFAULT_IP_ADDR "127.0.0.1"

void run_server(int UDP_clientfd, int TCP_clientfd)
{
    printf("Entered `run_server`...\n");

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

    // Create topics array.
    struct topics *topics = topics_array_create();

    // Create TCP clients array.
    struct TCP_clients *clients = TCP_clients_array_create();

    while (1) {
        printf("\nEntered server loop...\n");

        rc = poll(poll_fds, num_clients, -1);
        DIE(rc < 0, "poll failed");

        for (int i = 0; i < num_clients; i++) {

            // printf("\nStart for with i = %d\n\n", i);

            if (poll_fds[i].revents == POLLIN) {

                printf("!!! Event for fd %d !!!\n", poll_fds[i].fd);

                if (poll_fds[i].fd == 0) {
                    // Receive stdin command.

                    printf("Received stdin command...\n");

                    char server_cmd[CMD_MAXSIZE];
                    scanf("%s", server_cmd);
                    if (strcmp(server_cmd, "exit") == 0)
                        server_exit(poll_fds, num_clients, topics, clients);
                    else
                        fprintf(stderr, "Unknown command\n");
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

                    printf("Received UDP packet...\n");

                    // Parse UDP message.
                    uint16_t port = ntohs(UDP_cli_addr.sin_port);
                    UDP_parse_message(received_packet,
                                      inet_ntoa(UDP_cli_addr.sin_addr),
                                      &port,
                                      &UDP_parsed_msg);
                    
                    UDP_print_subscription_message(UDP_parsed_msg);

                    // Search for the topic in the topic list.
                    struct topic *topic =
                        find_topic(topics, UDP_parsed_msg.topic);
                    if (!topic)  // There are no subscribers for this topic.
                        continue;
                    
                    // Send the message to all subscribers.
                    struct packet sent_packet;
                    memcpy(sent_packet.message, (char *)&UDP_parsed_msg, sizeof(UDP_parsed_msg));
                    for (int i = 0; i < topic->num_subscribers; i++) {
                        send_all(topic->subscribers[i].fd,
                                 &sent_packet,
                                 sizeof(sent_packet));
                    }
                } else if (poll_fds[i].fd == TCP_clientfd) {
                    // New TCP connection.
                    struct sockaddr_in cli_addr;
                    socklen_t cli_len = sizeof(cli_addr);
                    int newsockfd = accept(TCP_clientfd,
                                           (struct sockaddr *)&cli_addr,
                                           &cli_len);
                    DIE(newsockfd < 0, "accept failed");

                    printf("Received TCP connection...\n");

                    // // Receive a message with the client's details.
                    // int rc = recv_all(newsockfd,
                    //                   &received_packet,
                    //                   sizeof(received_packet));
                    // DIE(rc < 0, "recv_all failed");
                    // // Parse the message and add the client's ID to the list.
                    // struct TCP_message TCP_msg =
                    //     *(struct TCP_message *)received_packet.message;
                    TCP_clients_array_add_client(clients,
                                                 "",
                                                 newsockfd,
                                                 inet_ntoa(cli_addr.sin_addr),
                                                 ntohs(cli_addr.sin_port));
                    
                    TCP_clients_array_print(clients);

                    // Add new socket to read file descriptors in poll.
                    poll_fds[num_clients].fd = newsockfd;
                    poll_fds[num_clients].events = POLLIN;
                    num_clients++;

                    // // TODO!!!!!!!!!!!!!!: change newsockfd to TCP_id
                    // server_print_connection_status(1,
                    //                                newsockfd,
                    //                                inet_ntoa(cli_addr.sin_addr),
                    //                                ntohs(cli_addr.sin_port));

                    i++;  // Don't consider this client at this iteration.
                } else {
                    // Receive message on one of the client sockets.
                    printf("Received TCP message...\n");

                    int rc = recv_all(poll_fds[i].fd,
                                      &received_packet,
                                      sizeof(received_packet));
                    DIE(rc < 0, "recv_all failed");

                    if (rc == 0) {
                        printf("TCP connection closed...\n");

                        // Client disconnected.
                        // TODO!!!!!!!!!!!!!!: change fd to TCP_id
                        struct TCP_client *TCP_client =
                            TCP_clients_array_find_client_from_fd(clients, poll_fds[i].fd);

                        server_print_connection_status(0, TCP_client->id, 0, 0);
                        close(poll_fds[i].fd);

                        // Remove closed socket from poll.
                        for (int j = i; j < num_clients - 1; j++) {
                            poll_fds[j] = poll_fds[j + 1];
                        }
                        num_clients--;
                    } else {
                        printf("Treat TCP message...\n");

                        // TCP_parse_message(received_packet, &TCP_parsed_msg);
                        struct TCP_message TCP_msg =
                            *(struct TCP_message *)received_packet.message;

                        // printf("S-a primit de la clientul de pe socketul %d mesajul: %s\n",
                        //              poll_fds[i].fd, received_packet.message);
                        
                        // Check if it's a new connection.
                        if (TCP_msg.sf == 2) {
                            printf("Get id from new TCP connection...\n");

                            // TCP_clients_array_add_client(clients,
                            //                             TCP_msg.id,
                            //                             TCP_clientfd);
                            
                            // TCP_clients_array_print(clients);

                            struct TCP_client *TCP_client =
                                TCP_clients_array_find_client_from_fd(clients,
                                                                      poll_fds[i].fd);
                            if (!TCP_client) {
                                fprintf(stderr, "No client found with file descriptor %d.\n", poll_fds[i].fd);
                                continue;
                            }

                            // Don't accept connection of an existing client.
                            struct TCP_client *existing_client =
                                TCP_clients_array_find_client_from_id(clients,
                                                                      TCP_msg.id);
                            if (existing_client) {
                                printf("Client exists.\n");

                                printf("Client %s already connected.\n", TCP_client->id);
                                close(poll_fds[i].fd);
                                // Remove closed socket from poll.
                                for (int j = i; j < num_clients - 1; j++) {
                                    poll_fds[j] = poll_fds[j + 1];
                                }
                                num_clients--;

                                continue;
                            }

                            strcpy(TCP_client->id, TCP_msg.id);

                            TCP_clients_array_print(clients);

                            server_print_connection_status(1,
                                                           TCP_client->id,
                                                           TCP_client->ip,
                                                           TCP_client->port);
                            continue;
                        }
                        
                        // Find the topic to subscribe to/unsubscribe from.
                        struct topic *topic =
                            find_topic(topics, TCP_parsed_msg.topic);

                        if (TCP_parsed_msg.subscribe_status == 1) {
                            printf("Before subscribing...\n");
                            topics_array_print(topics);

                            // Create subscriber.
                            struct subscriber sub =
                                subscriber_create(poll_fds[i].fd,
                                                  TCP_parsed_msg.id,
                                                  TCP_parsed_msg.sf);
                            
                            // If the topic is not in the list, add it.
                            if (!topic) {
                                topics_array_add_topic(topics,
                                                       TCP_parsed_msg.topic);
                            }

                            // Subscribe.
                            subscribe_to_topic(topics->topics[topics->size - 1],
                                               sub);

                            printf("After subscribing...\n");
                            topics_array_print(topics);
                        } else {
                            printf("Before unsubscribing...\n");
                            topics_array_print(topics);

                            // Unsubscribe.
                            if (!topic) {
                                printf("Cannot unsubscribe - topic not found.\n");
                                continue;
                            } 
                            unsubscribe_from_topic(topic, TCP_parsed_msg.id);

                            // If topic has no subscribers, remove it.
                            if (topic->num_subscribers == 0) {
                                topics_array_remove_topic(topics, topic->name);
                            }

                            printf("After unsubscribing...\n");
                            topics_array_print(topics);
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("\n Usage: %s <PORT>\n", argv[0]);
        return 1;
    }

    // Parse port number.
    uint16_t port;
    int rc = sscanf(argv[1], "%hu", &port);
    DIE(rc != 1, "Invalid port");

    // Set up the sockaddr_in struct.
    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);
    // Fill in the structure.
    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    rc = inet_pton(AF_INET, DEFAULT_IP_ADDR, &serv_addr.sin_addr.s_addr);
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
