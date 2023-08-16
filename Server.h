#ifndef TELNETCHAT_SERVER_H
#define TELNETCHAT_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>


typedef struct addr_ai {
    struct addrinfo addr;
    struct addrinfo* ai;
} addr_ai;

struct Server {
    int listener;

    struct pollfd* pfds;

    addr_ai addr;

    struct sockaddr_storage remoteaddr; // Client Address
    int fd_count;
    int fd_size;
};

struct Server initServer();

void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);

void newUser(int newfd, struct Server* server, char *remoteIP);

#endif //TELNETCHAT_SERVER_H
