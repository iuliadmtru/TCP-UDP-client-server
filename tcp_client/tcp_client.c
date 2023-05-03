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

    struct chat_packet sent_packet;
    struct chat_packet recv_packet;

    /* TODO 2.2: Multiplexeaza intre citirea de la tastatura si primirea unui
         mesaj, ca sa nu mai fie impusa ordinea.
    */
    struct pollfd pfds[32];
    int nfds;

    nfds = 0;

    pfds[nfds].fd = STDIN_FILENO;
    pfds[nfds].events = POLLIN;
    nfds++;

    // se creaza socketul tastaturii
    pfds[nfds].fd = sockfd;
    pfds[nfds].events = POLLIN;
    nfds++;

    while (1)
    {
        // se asteapta pana cand se primesc date pe cel putin unul din file descriptori
        poll(pfds, nfds, -1);

        if ((pfds[1].revents & POLLIN) != 0)
        {
            // se trateaza noua conexiune...
            int rc = recv_all(sockfd, &recv_packet, sizeof(recv_packet));
            printf("%s\n", recv_packet.message);
        }
        else if ((pfds[0].revents & POLLIN) != 0)
        {
            // se citesc date de la utilizator...
            fgets(buf, sizeof(buf), stdin);
            sent_packet.len = strlen(buf) + 1;
            strcpy(sent_packet.message, buf);

            send_all(sockfd, &sent_packet, sizeof(sent_packet));
        }
        else
        {
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

    // Parsam port-ul ca un numar
    uint16_t port;
    int rc = sscanf(argv[2], "%hu", &port);
    DIE(rc != 1, "Given port is invalid");

    // Obtinem un socket TCP pentru conectarea la server
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket");

    // Completăm in serv_addr adresa serverului, familia de adrese si portul
    // pentru conectare
    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);

    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    rc = inet_pton(AF_INET, argv[1], &serv_addr.sin_addr.s_addr);
    DIE(rc <= 0, "inet_pton");

    // Ne conectăm la server
    rc = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    DIE(rc < 0, "connect");

    run_client(sockfd);

    // Inchidem conexiunea si socketul creat
    close(sockfd);

    return 0;
}
