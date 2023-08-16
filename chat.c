#include "Server.h"

#define PORT "9034"   // Port we're listening on


void checkChat(int i, int nbytes, int sender_fd, struct Server server) {
    if (nbytes <= 0) {
        // Got error or connection closed by client
        if (nbytes == 0) {
            // Connection closed
            printf("pollserver: socket %d hung up\n", sender_fd);
        } else {
            perror("recv");
        }

        close(server.pfds[i].fd); // Bye!

        del_from_pfds(server.pfds, i, &server.fd_count);
    }
}

void loopUsers(struct Server server, char buf[], int sender_fd, int nbytes) {
    for(int j = 0; j < server.fd_count; j++) {
        int dest_fd = server.pfds[j].fd;
        if (dest_fd != server.listener && dest_fd != sender_fd) {
            if (send(dest_fd, buf, nbytes, 0) == -1) {
                perror("send");
            }
        }
    }
}

int main(void)
{   // Listening socket descriptor
    int newfd;
    socklen_t addrlen;

    struct Server server = initServer();
    char buf[256];
    char remoteIP[INET6_ADDRSTRLEN];


    server.pfds[0].fd = server.listener;
    server.pfds[0].events = POLLIN;

    server.fd_count = 1;

    // Main loop
    for(;;) {
        int poll_count = poll(server.pfds, server.fd_count, -1);

        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }

        for(int i = 0; i < server.fd_count; i++) {
            if (server.pfds[i].revents & POLLIN) {

                if (server.pfds[i].fd == server.listener) {
                    addrlen = sizeof server.remoteaddr;
                    newfd = accept(server.listener,
                                   (struct sockaddr *)&server.remoteaddr,
                                   &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        newUser(newfd, &server, remoteIP);
                    }

                } else {
                    int nbytes = recv(server.pfds[i].fd, buf, sizeof buf, 0);
                    int sender_fd = server.pfds[i].fd;

                    // Check if user dropped out and if message is valid
                    checkChat(i, nbytes, sender_fd, server);
                    // send and receive messages
                    loopUsers(server, buf, sender_fd, nbytes);
                }
            }
        }
    }
    return 0;
}