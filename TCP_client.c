#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "poller.h"
#include "packets.h"
#include "clients.h"
#include "util.h"

// ------------------------ PARSE USER INPUT ------------------------

#define CMD_MAXLEN 12

enum command {CLIENT_CMD_EXIT,
              CLIENT_CMD_SUBSCRIBE,
              CLIENT_CMD_UNSUBSCRIBE,
              CLIENT_CMD_UNDEFINED};

struct args {
    char id[ID_MAXLEN];
    struct in_addr server_ip;
    uint16_t server_port;
};

/*
 * Check that the argument number is correct.
 */
int check_args(int argc, char *argv[])
{
    if (argc != 4) {
        printf("\n Usage: %s <ID_CLIENT> <IP_SERVER> <PORT_SERVER>\n", argv[0]);
        return 0;
    }

    return 1;
}

/*
 * Parse the arguments passed to the executable:
 * ID_CLIENT, IP_SERVER, PORT_SERVER.
 */
struct args get_args(char *argv[])
{
    struct args args;

    int rc;
    rc = sscanf(argv[1], "%s", args.id);
    DIE(rc != 1, "invalid id");
    rc = sscanf(argv[2], "%d", &args.server_ip.s_addr);
    DIE(rc != 1, "invalid server ip");
    rc = sscanf(argv[3], "%hu", &args.server_port);
    DIE(rc != 1, "invalid server port");

    return args;
}

/*
 * Parse user command - SERVER_CMD_EXIT or SERVER_CMD_UNDEFINED.
 */
int get_user_command()
{
    char cmd[CMD_MAXLEN];
    scanf("%s", cmd);

    if (strcmp(cmd, "exit") == 0)
        return CLIENT_CMD_EXIT;
    return CLIENT_CMD_UNDEFINED;
}


// ------------------------ INITIALIZE ------------------------

struct sockaddr_in client_initialize_sockaddr(struct args args)
{
    struct sockaddr_in cli_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);

    // Fill in.
    memset(&cli_addr, 0, socket_len);
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = htons(args.server_port);
    int rc = inet_pton(AF_INET,
                       inet_ntoa(args.server_ip),
                       &cli_addr.sin_addr.s_addr);
    DIE(rc <= 0, "inet_pton failed");

    return cli_addr;
}

int client_initialize_socket(struct sockaddr_in cli_addr)
{
    // Open TCP socket.
    int cli_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_fd < 0)
        return FAIL_SOCKET;

    // Make the socket address reusable.
    int enable = 1;
    if (setsockopt(cli_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        return FAIL_SETSOCKOPT;

    // Connect to server.
    int rc = connect(cli_fd, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
    if (rc < 0)
        return FAIL_CONNECT;

    return cli_fd;
}


// ------------------------ TCP_client ------------------------

struct TCP_client {
    int fd;
    struct sockaddr_in cli_addr;
    socklen_t cli_addr_len;
    struct packet recv_packet;
    struct TCP_ctos_msg sent_msg;
};

struct TCP_client *TCP_client_create(int fd)
{
    struct TCP_client *TCP_client = malloc(sizeof(struct TCP_client));

    TCP_client->fd = fd;
    TCP_client->cli_addr_len = sizeof(struct sockaddr_in);

    return TCP_client;
}

void TCP_client_destroy(struct TCP_client *TCP_client)
{
    close(TCP_client->fd);
    free(TCP_client);
}


// ------------------------ CLIENT FUNCTIONS ------------------------

void client_exit(struct poller *poller, struct TCP_client *TCP_client)
{
    poller_destroy(poller);
    TCP_client_destroy(TCP_client);
    // TODO?
    exit(0);
}


// ------------------------ RUN ------------------------

void run_client(struct TCP_client *TCP_client)
{
    // Initialize poll with STDIN and TCP client file descriptors.
    struct poller *poller = poller_create();
    poller_add_fd(poller, STDIN_FILENO);
    poller_add_fd(poller, TCP_client->fd);

    while (1) {
        int rc = poller_poll(poller);
        DIE(rc < 0, "poll failed");

        // Iterate through file descriptors where an event happened.
        while (1) {
            int fd = poller_next_fd_with_POLLIN(poller);
            if (fd == -1) {
                break;
            } else if (fd == STDIN_FILENO) {  // Receive STDIN command.
                int cmd = get_user_command();
                switch (cmd) {
                    case CLIENT_CMD_EXIT:
                        client_exit(poller, TCP_client);
                        break;
                    case CLIENT_CMD_UNDEFINED:
                        fprintf(stderr, "Unknown command\n");
                        break;
                }
            } else {  // Receive packet.

            }
        }
    }
}


int main(int argc, char *argv[])
{
    // Check and parse arguments.
    if (!check_args(argc, argv))
        return -1;
    struct args args = get_args(argv);

    // Initialize socket.
    struct sockaddr_in cli_addr = client_initialize_sockaddr(args);
    int cli_fd = client_initialize_socket(cli_addr);
    DIE(cli_fd < 0, "client_connect_to_server_initialize_fd failed");
    struct TCP_client *TCP_client = TCP_client_create(cli_fd);

    // Send connection message to provide the client id to the server.
    // TODO

    // Run server.
    run_client(TCP_client);

    // Destroy sockets.
    TCP_client_destroy(TCP_client);

    return 0;
}