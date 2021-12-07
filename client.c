#include "pong_client.h"

#include <stdio.h>
#include <pthread.h>

int main(int argc, char **argv) {
    char port[6] = DEFAULT_PORT;
    char host[255] = DEFAULT_IP;
    int client_socket = -1;

    get_port_parameter(argc, argv, port);
    get_host_parameter(argc, argv, host);

    client_socket = get_client_socket(host, port);
    if (client_socket == -1) {
        printf(" Fatal ERROR: Could not open client socket - exiting...\n");
        return -1;
    }

    /* allocate shared memory for client */
    client_shared_memory_config sh_mem_cfg;
    get_client_shared_memory(&sh_mem_cfg);
    sh_mem_cfg.recv_packet_ready = 0;
    sh_mem_cfg.send_packet_ready = 0;

    /* initialize packet receiving and sending threads */
    client_thread_args cta;
    cta.client_socket = client_socket;
    cta.sh_mem_cfg = &sh_mem_cfg;

    pthread_t receiving_thread_id;
    if (pthread_create(&receiving_thread_id, NULL, receive_server_packets, (void *) &cta) != 0) {
        printf("Error creating processing thread\n");
        return -1;
    }

    pthread_t sending_thread_id;
    if (pthread_create(&sending_thread_id, NULL, send_client_packets, (void *) &cta) != 0) {
        printf("Error creating sending thread\n");
        return -1;
    }

    /* process already validated incoming packets */
    process_server_packets(&sh_mem_cfg);

    return 0;
} 