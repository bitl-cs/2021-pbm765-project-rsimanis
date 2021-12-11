#include "pong_client.h"
#include "pong_networking.h"

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

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
    pthread_t receiving_thread_id;
    client_recv_thread_args crta;
    crta.recv_mem_cfg = &sh_mem_cfg.recv_mem_cfg;
    crta.socket = client_socket;
    if (pthread_create(&receiving_thread_id, NULL, receive_server_packets, (void *) &crta) != 0)
        return -1;
    sleep(THREAD_INIT_WAIT_TIME); /* wait until thread's local variables from its argument (recv_thread_args) are initialized */

    /* initialize packet sending thread */
    pthread_t sending_thread_id;
    send_thread_args sta;
    sta.socket = client_socket;
    sta.send_mem_cfg = &sh_mem_cfg.send_mem_cfg;
    if (pthread_create(&sending_thread_id, NULL, send_client_packets, (void *) &sta) != 0)
        return -1;
    sleep(THREAD_INIT_WAIT_TIME); /* wait until thread's local variables from its argument (recv_thread_args) are initialized */

    send_join("Raivis", &(sh_mem_cfg.send_mem_cfg));

    /* process already validated incoming packets */
    process_server_packets(&sh_mem_cfg.recv_mem_cfg, &sh_mem_cfg.send_mem_cfg);


    return 0;
} 