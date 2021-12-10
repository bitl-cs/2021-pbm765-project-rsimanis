#include "pong_client.h"
#include "pong_networking.h"

#include <stdio.h>
#include <pthread.h>

int main(int argc, char **argv) {
    char port[6] = DEFAULT_PORT;
    char host[255] = DEFAULT_IP;
    int client_socket = -1;

    get_port_parameter(argc, argv, port);
    get_host_parameter(argc, argv, host);

    /* establish connection with server */
    client_socket = get_client_socket(host, port);
    if (client_socket == -1) {
        printf(" Fatal ERROR: Could not open client socket - exiting...\n");
        return -1;
    }

    /* allocate shared memory for client */
    client_shared_memory_config sh_mem_cfg;
    get_client_shared_memory(&sh_mem_cfg);

    /* initialize packet receiving thread */
    if (init_recv_thread(client_socket, &(sh_mem_cfg.recv_mem_cfg)) < 0) {
        printf(" Fatal ERROR: Could not create packet receiving thread - exiting...\n");
        return -1;
    }

    /* initialize packet sending thread */
    if (init_send_thread(client_socket, &(sh_mem_cfg.send_mem_cfg), send_client_packets) < 0) {
        printf(" Fatal ERROR: Could not create packet sending thread - exiting...\n");
        return -1;
    }

    /* process already validated incoming packets */
    process_server_packets(&(sh_mem_cfg.recv_mem_cfg));

    send_join("Raivis", &(sh_mem_cfg.send_mem_cfg));

    return 0;
} 