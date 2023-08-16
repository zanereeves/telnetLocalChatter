#include "Server.h"


#define PORT "9034"

void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    pfds[i] = pfds[*fd_count-1];

    (*fd_count)--;
}


static int checkAddrErr(struct addrinfo* addr, struct addrinfo** ai) {
    int rv = getaddrinfo(NULL, PORT, addr, ai);
    if (rv == 0) {
        return rv;
    }
    fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
    exit(1);
}

struct addr_ai startAddr() {
    addr_ai combo;

    struct addrinfo addr, *ai;

    memset(&addr, 0, sizeof(addr));
    addr.ai_family = AF_UNSPEC;
    addr.ai_socktype = SOCK_STREAM;
    addr.ai_flags = AI_PASSIVE;
    int rv = checkAddrErr(&addr, &ai);

    combo.addr = addr;
    combo.ai = ai;
    return combo;
}


static void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

static void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
    if (*fd_count == *fd_size) {
        *fd_size *= 2; // Double it

        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read

    (*fd_count)++;
}

void newUser(int newfd, struct Server* server, char *remoteIP) {
    add_to_pfds(&server->pfds, newfd, &server->fd_count, &server->fd_size);

    printf("pollserver: new connection from %s on "
           "socket %d\n",
           inet_ntop(server->remoteaddr.ss_family,
                     get_in_addr((struct sockaddr*)&server->remoteaddr),
                     remoteIP, INET6_ADDRSTRLEN),
           newfd);

}

struct Server initServer() {
    struct Server server;

    struct addrinfo *p;
    int listener;
    int yes = 1;

    server.fd_count = 0;
    server.fd_size = 5;
    server.pfds = malloc(sizeof(struct pollfd*)*server.fd_size);

    server.addr = startAddr();



    for(p = server.addr.ai; p != NULL; p = p->ai_next) {

        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // If we got here, it means we didn't get bound
    if (p == NULL) {
        exit(1);
    }


    // Listen
    if (listen(listener, 10) == -1) {
        exit(1);
    }

    server.listener = listener;
    return server;
}