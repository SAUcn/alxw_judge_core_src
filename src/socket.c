#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "run.c"

int init_socket(const char* path)
{
    int server_sockfd;
    int client_sockfd;
    struct sockaddr_un server_addr;
    struct sockaddr_un client_addr;
    char buffer[BUFF_SIZE];
    char* ret;

    unlink(path);

    server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, path);
    bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_sockfd, 5);
    socklen_t len = sizeof(client_addr);

    while(1)
    {
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &len);
        read(client_sockfd, buffer, BUFF_SIZE);
        printf("Get new task from client\n");

        ret = runit(buffer);

        write(client_sockfd, ret, strlen(ret));
        close(client_sockfd);
    }

}
