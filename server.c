#include "pong_networking.h"
#include "pong_server.h"

#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
    int pid = 0;
    char port[6] = DEFAULT_PORT;

    printf("Starting server...\n");
    get_port_parameter(argc, argv, port);

    server_shared_memory *sh_mem = get_server_shared_memory(); 

    /* split in two processes - network and gameloop */
    pid = fork();
    if (pid == 0)
        start_network(port, sh_mem);
    else {
        // pid = fork();
        // if (pid == 0)
        //     lobbyloop(sh_mem);
        // else
            gameloop(sh_mem);

    }

    return 0;
}