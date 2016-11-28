#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "socket.c"

int main(int argc, char* argv[])
{
    
    if (argc < 2)
    {
        fprintf(stderr, "Error listen socket !\n");
        return 1;
    }

    printf("Daemon listern on socket: %s\n", argv[1]);

    initcall();

    init_socket(argv[1]);
    
    return 0;
}

