#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

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

struct command_args {
    int cmd;
    char topic[TOPIC_LEN];
    uint8_t store_and_forward;
};

void command_args_print(struct command_args args)
{
    printf("Print command arguments:\n");
    switch (args.cmd) {
        case CLIENT_CMD_EXIT:
            printf("cmd: CLIENT_CMD_EXIT\n");
            printf("topic: %s\n", args.topic);
            printf("store_and_forward: %hhu\n", args.store_and_forward);
            break;
        case CLIENT_CMD_SUBSCRIBE:
            printf("cmd: CLIENT_CMD_SUBSCRIBE\n");
            printf("topic: %s\n", args.topic);
            printf("store_and_forward: %hhu\n", args.store_and_forward);
            break;
        case CLIENT_CMD_UNSUBSCRIBE:
            printf("cmd: CLIENT_CMD_UNSUBSCRIBE\n");
            printf("topic: %s\n", args.topic);
            printf("store_and_forward: %hhu\n", args.store_and_forward);
            break;
        case CLIENT_CMD_UNDEFINED:
            printf("cmd: CLIENT_CMD_UNDEFINED\n");
            break;
    }
}

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

int vread(char *fmt, ...)
{
   int rc;
   va_list arg_ptr;
   va_start(arg_ptr, fmt);
   rc = vscanf(fmt, arg_ptr);
   va_end(arg_ptr);
   return(rc);
}

/*
 * Parse user command: CLIENT_CMD_EXIT, CLIENT_CMD_SUBSCRIBE,
 * CLIENT_CMD_UNSUBSCRIBE, CLIENT_CMD_UNDEFINED.
 */
struct command_args get_user_command()
{
    struct command_args args;
    memset(&args, 0, sizeof(args));

    char cmd[CMD_MAXLEN];
    int rc = scanf("%s", cmd);

    if (strcmp(cmd, "exit") == 0) {
        args.cmd = CLIENT_CMD_EXIT;
    } else if (strcmp(cmd, "subscribe") == 0) {
        rc = vread("%s %hhu", args.topic, &args.store_and_forward);
        if (rc != 2) {
            fprintf(stderr, "Not all arguments were initialized\n");
            args.cmd = CLIENT_CMD_UNDEFINED;
            return args;
        }
        args.cmd = CLIENT_CMD_SUBSCRIBE;
    } else if (strcmp(cmd, "unsubscribe") == 0) {
        rc = vread("%s", args.topic);
        if (rc != 1) {
            fprintf(stderr, "Not all arguments were initialized\n");
            args.cmd = CLIENT_CMD_UNDEFINED;
            return args;
        }
        args.cmd = CLIENT_CMD_UNSUBSCRIBE;
    } else {
        args.cmd = CLIENT_CMD_UNDEFINED;
    }

    command_args_print(args);
    printf("\n");

    return args;
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
    char id[ID_MAXLEN];
    struct sockaddr_in cli_addr;
    socklen_t cli_addr_len;
    struct packet recv_packet;
};

struct TCP_client *TCP_client_create(int fd, char *id)
{
    struct TCP_client *TCP_client = malloc(sizeof(struct TCP_client));

    TCP_client->fd = fd;
    strcpy(TCP_client->id, id);
    TCP_client->cli_addr_len = sizeof(struct sockaddr_in);

    return TCP_client;
}

void TCP_client_destroy(struct TCP_client *TCP_client)
{
    close(TCP_client->fd);
    free(TCP_client);
}

int TCP_client_send(struct TCP_client *TCP_client,
                    struct TCP_header TCP_msg)
{
    struct packet sent_packet;
    memset(&sent_packet, 0, sizeof(sent_packet));

    memcpy(sent_packet.msg, (char *)&TCP_msg, TCP_msg.msg_len);
    int rc = send_all(TCP_client->fd, &sent_packet, sizeof(sent_packet));

    printf("Sent message:\n");
    TCP_header_print(*(struct TCP_header *)sent_packet.msg, TCP_CTOS_MSG);
    printf("---First two bytes: %hu\n", *(uint16_t *)sent_packet.msg);

    return rc;
}

int TCP_client_recv(struct TCP_client *TCP_client)
{
    int rc = recv_all(TCP_client->fd, &TCP_client->recv_packet, sizeof(TCP_client->recv_packet));
    if (rc < 0)
        return -1;
    if (rc == 0)  // Connection ended.
        return 0;
    
    return rc;
}


// ------------------------ CLIENT FUNCTIONS ------------------------

void client_exit(struct poller *poller, struct TCP_client *TCP_client)
{
    poller_destroy(poller);
    TCP_client_destroy(TCP_client);
    // TODO?
    exit(0);
}

void client_set_payload(char *payload, struct command_args args)
{
    memset(payload, 0, CTOS_MAXLEN);
    strcat(payload, args.topic);
    args.store_and_forward ? strcat(payload, " 1") :
                                strcat(payload, " 0");
}

struct TCP_header client_compose_msg(uint8_t type, char *payload)
{
    struct TCP_ctos_msg TCP_msg;
    memset(&TCP_msg, 0, sizeof(TCP_msg));

    memcpy(&TCP_msg.msg_type, &type, sizeof(type));
    strcpy(TCP_msg.payload, payload);

    printf("payload: %s\n", payload);

    struct TCP_header TCP_hdr;
    memset(&TCP_hdr, 0, sizeof(TCP_hdr));

    uint16_t msg_len = sizeof(struct TCP_ctos_msg);
    memcpy(&TCP_hdr.msg_len, &msg_len, sizeof(msg_len));
    memcpy(TCP_hdr.msg, &TCP_msg, msg_len);

    printf("Composed message:\n");
    // TCP_header_print(TCP_hdr, TCP_CTOS_MSG);
    TCP_ctos_msg_print(*(struct TCP_ctos_msg *)TCP_hdr.msg);

    return TCP_hdr;
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
                struct command_args args = get_user_command();
                switch (args.cmd) {
                    case CLIENT_CMD_EXIT:
                        client_exit(poller, TCP_client);
                        break;
                    case CLIENT_CMD_SUBSCRIBE:
                        // Construct payload as the string "<TOPIC> <SF>".
                        char payload[CTOS_MAXLEN];
                        client_set_payload(payload, args);

                        // Compose and send subscribe message.
                        struct TCP_header subscribe_msg =
                            client_compose_msg(TCP_MSG_SUBSCRIBE, payload);
                        int rc = TCP_client_send(TCP_client, subscribe_msg);
                        DIE(rc < 0, "TCP_client_send failed");

                        break;
                    case CLIENT_CMD_UNSUBSCRIBE:
                        // Compose and send unsubscribe message.
                        struct TCP_header unsubscribe_msg =
                            client_compose_msg(TCP_MSG_UNSUBSCRIBE, args.topic);
                        rc = TCP_client_send(TCP_client, unsubscribe_msg);
                        DIE(rc < 0, "TCP_client_send failed");

                        break;
                    case CLIENT_CMD_UNDEFINED:
                        fprintf(stderr, "Unknown command\n");
                        break;
                }
            } else {  // Receive packet.
                int rc = TCP_client_recv(TCP_client);
                DIE(rc == -1, "TCP_client_recv failed");

                if (rc == 0)  // Connection ended.
                    client_exit(poller, TCP_client);

                // TODO: treat UDP subscription message
            }
        }
    }
}


int main(int argc, char *argv[])
{
    // Initial configuration.
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    // Check and parse arguments.
    if (!check_args(argc, argv))
        return -1;
    struct args args = get_args(argv);

    // Initialize socket.
    struct sockaddr_in cli_addr = client_initialize_sockaddr(args);
    int cli_fd = client_initialize_socket(cli_addr);
    DIE(cli_fd < 0, "client_connect_to_server_initialize_fd failed");
    struct TCP_client *TCP_client = TCP_client_create(cli_fd, args.id);

    // Send connection message to provide the client id to the server.
    struct TCP_header connection_msg = client_compose_msg(TCP_MSG_NEW, args.id);
    int rc = TCP_client_send(TCP_client, connection_msg);
    DIE(rc < 0, "TCP_client_send failed");

    // Run server.
    run_client(TCP_client);

    // Destroy sockets.
    TCP_client_destroy(TCP_client);

    return 0;
}