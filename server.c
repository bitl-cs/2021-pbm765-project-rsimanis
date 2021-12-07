#include "pong_networking.h"
#include "pong_server.h"

#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
    shared_memory_config sh_mem_config;
    int pid = 0;
    char port[6] = DEFAULT_PORT;

    printf("Starting server...\n");
    get_port_parameter(argc, argv, port);

    get_shared_memory(&sh_mem_config);

    /* split in two processes - network and gameloop */
    pid = fork();
    if (pid == 0)
        start_network(port, &sh_mem_config);
    else
        gameloop(&sh_mem_config);

    return 0;
}