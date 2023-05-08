#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "util.h"
#include "poller.h"

#define SERVER_IP_ADDR "0.0.0.0"
#define CMD_MAXLEN 5

enum command {SERVER_CMD_EXIT, SERVER_CMD_UNDEFINED};


// ------------------------ PARSE USER INPUT ------------------------

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

/*
 * Initialize UDP client with type = SOCK_DGRAM and TCP client with
 * type = SOCK_STREAM;
 */
int server_initialize_socket(struct sockaddr_in serv_addr, int type)
{
    // Open socket.
    int clientfd = socket(AF_INET, type, 0);
    DIE(clientfd < 0, "socket failed");

    // Make the socket address reusable.
    int enable = 1;
    if (setsockopt(clientfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    // Bind socket.
    int rc = bind(clientfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    DIE(rc < 0, "bind failed");

    return clientfd;
}


// ------------------------ SERVER FUNCTIONS ------------------------
void server_exit(struct poller *poller)
{
    poller_destroy(poller);
    // TODO
    exit(0);
}


// ------------------------ RUN ------------------------

void run_server(int UDP_fd, int TCP_fd)
{
    // Listen for TCP clients.
    int rc = listen(TCP_fd, MAX_CONNECTIONS);
    DIE(rc < 0, "listen failed");

    // Initialize poll with STDIN, UDP and TCP file descriptors.
    struct poller *poller = poller_create();
    poller_add_fd(poller, STDIN_FILENO);
    poller_add_fd(poller, UDP_fd);
    poller_add_fd(poller, TCP_fd);

    while (1) {
        rc = poller_poll(poller);
        DIE(rc < 0, "poll failed");

        // Iterate through file descriptors where an event happened.
        while (1) {
            int fd = poller_next_fd_with_POLLIN(poller);
            if (fd == -1) {
                break;
            } else if (fd == STDIN_FILENO) {  // Receive STDIN command.
                int cmd = get_user_command();
                switch (cmd) {
                    case SERVER_CMD_EXIT:
                        server_exit(poller);
                        break;
                    case SERVER_CMD_UNDEFINED:
                        fprintf(stderr, "Unknown command\n");
                        break;
                }
            } else if (fd == UDP_fd) {  // Receive UDP packet.

            } else if (fd == TCP_fd) {  // Receive new TCP connection.

            } else {  // Receive TCP message.

            }
        }
    }
}


// ------------------------ MAIN ------------------------

int main(int argc, char *argv[])
{
    // Check and parse arguments.
    if (!check_args(argc, argv))
        return -1;
    uint16_t port = get_args(argv);

    // Initialize sockets.
    struct sockaddr_in serv_addr = server_initialize_sockaddr(port);
    int UDP_fd = server_initialize_socket(serv_addr, SOCK_DGRAM);
    int TCP_fd = server_initialize_socket(serv_addr, SOCK_STREAM);

    // Run server.
    run_server(UDP_fd, TCP_fd);

    // Close sockets.
    close(UDP_fd);
    close(TCP_fd);

    return 0;
}