#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "util.h"
#include "poller.h"
#include "packets.h"
#include "UDP_server.h"
#include "TCP_server.h"

#define SERVER_IP_ADDR "0.0.0.0"
#define CMD_MAXLEN 5


// ------------------------ PARSE USER INPUT ------------------------

enum command {SERVER_CMD_EXIT, SERVER_CMD_UNDEFINED};

/*
 * Check that the argument number is correct.
 */
int check_args(int argc, char *argv[])
{
    if (argc != 2) {
        printf("\n Usage: %s <PORT>\n", argv[0]);
        return 0;
    }

    return 1;
}

/*
 * Parse the arguments passed to the executable - server port.
 */
uint16_t get_args(char *argv[])
{
    uint16_t arg;
    int rc = sscanf(argv[1], "%hu", &arg);
    DIE(rc != 1, "invalid argument");

    return arg;
}

/*
 * Parse user command - SERVER_CMD_EXIT or SERVER_CMD_UNDEFINED.
 */
int get_user_command()
{
    char cmd[CMD_MAXLEN];
    scanf("%s", cmd);

    if (strncmp(cmd, "exit", CMD_MAXLEN) == 0)
        return SERVER_CMD_EXIT;
    return SERVER_CMD_UNDEFINED;
}


// ------------------------ INITIALIZE ------------------------

struct sockaddr_in server_initialize_sockaddr(uint16_t port)
{
    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);

    // Fill in.
    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    int rc = inet_pton(AF_INET, SERVER_IP_ADDR, &serv_addr.sin_addr.s_addr);
    DIE(rc <= 0, "inet_pton failed");

    return serv_addr;
}


// ------------------------ UDP ------------------------



// ------------------------ SERVER FUNCTIONS ------------------------

enum connection_status {CONNECTED, DISCONNECTED};

void server_exit(struct poller *poller,
                 struct UDP_server *UDP_server,
                 struct TCP_server *TCP_server)
{
    poller_destroy(poller);
    UDP_server_destroy(UDP_server);
    TCP_server_destroy(TCP_server);
    // TODO
    exit(0);
}

void server_print_connection_status(struct TCP_server *TCP_server,
                                    int fd,
                                    int status)
{
    client_node *client =
        clients_list_find_by_fd(TCP_server->connected_clients, fd);
    switch (status) {
        case CONNECTED:
            printf("New client %s connected from %s:%hu.\n",
                   client->id,
                   inet_ntoa(client->ip),
                   client->port);
            break;
        case DISCONNECTED:
            printf("Client %s disconnected.\n", client->id);
            break;
    }
}


// ------------------------ RUN ------------------------

void run_server(struct UDP_server *UDP_server, struct TCP_server *TCP_server)
{
    // Initialize poll with STDIN, UDP and TCP file descriptors.
    struct poller *poller = poller_create();
    poller_add_fd(poller, STDIN_FILENO);
    poller_add_fd(poller, UDP_server->fd);
    poller_add_fd(poller, TCP_server->fd);

    while (1) {
        int rc = poller_poll(poller);
        DIE(rc < 0, "poll failed");

        // Iterate through file descriptors where an event happened.
        while (1) {
            printf("\nEntered server loop...\n");

            int fd = poller_next_fd_with_POLLIN(poller);
            if (fd == -1) {
                break;
            } else if (fd == STDIN_FILENO) {  // Receive STDIN command.
                int cmd = get_user_command();
                switch (cmd) {
                    case SERVER_CMD_EXIT:
                        server_exit(poller, UDP_server, TCP_server);
                        break;
                    case SERVER_CMD_UNDEFINED:
                        fprintf(stderr, "Unknown command\n");
                        break;
                }
            } else if (fd == UDP_server->fd) {  // Receive UDP packet.
                rc = UDP_server_recv(UDP_server);
                DIE(rc < 0, "UDP_server_recv failed");

                // Send the packet to all the topic's subscribers.

            } else if (fd == TCP_server->fd) {  // Receive new TCP connection.
                int cli_fd = TCP_server_accept_connection(TCP_server);
                DIE(cli_fd < 0, "TCP_server_accept_connection failed");

                // Add the new socket to the poller.
                poller_add_fd(poller, cli_fd);
                // Don't consider this client at this iteration.
                poller_advance(poller);
            } else {  // Receive TCP message.
                rc = TCP_server_recv_all(TCP_server, fd);
                DIE(rc == -1, "TCP_server_recv_all failed");
                if (rc == 0) {  // Client disconnected.
                    server_print_connection_status(TCP_server,
                                                   fd,
                                                   DISCONNECTED);
                    TCP_server_close_connection(TCP_server, fd);
                    continue;
                }

                if (TCP_server_is_new_connection(TCP_server, fd)) {
                    printf("Treat new connection\n");

                    rc = TCP_server_update_client(TCP_server, fd);
                    if (rc == -1)  // Client is already connected.
                        poller_remove_fd(poller, fd);
                    else
                        server_print_connection_status(TCP_server,
                                                       fd,
                                                       CONNECTED);
                } else {  // Received subscribe/unsubscribe message.
                    switch (TCP_server->recv_msg.msg_type) {
                        case TCP_MSG_SUBSCRIBE:
                            printf("Treat subscribe message\n");

                            TCP_server_subscribe(TCP_server, fd);
                            break;
                        case TCP_MSG_UNSUBSCRIBE:
                            printf("Treat unsubscribe message\n");

                            TCP_server_unsubscribe(TCP_server, fd);
                            break;
                    }
                }
            }
        }
    }
}


// ------------------------ MAIN ------------------------

int main(int argc, char *argv[])
{
    // Initial configuration.
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    // Check and parse arguments.
    if (!check_args(argc, argv))
        return -1;
    uint16_t port = get_args(argv);

    // Initialize sockets.
    struct sockaddr_in serv_addr = server_initialize_sockaddr(port);
    // UDP.
    int UDP_fd = UDP_server_initialize_socket(serv_addr);
    DIE(UDP_fd < 0, "UDP_server_initialize_socket failed");
    struct UDP_server *UDP_server = UDP_server_create(UDP_fd);
    // TCP.
    int TCP_fd = TCP_server_initialize_socket(serv_addr);
    DIE(TCP_fd < 0, "TCP_server_initialize_socket failed");
    struct TCP_server *TCP_server = TCP_server_create(TCP_fd);

    // Run server.
    run_server(UDP_server, TCP_server);

    // Destroy sockets.
    UDP_server_destroy(UDP_server);
    TCP_server_destroy(TCP_server);

    return 0;
}