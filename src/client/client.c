#include "pong_client.h"

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

    /* initialize shared memory for client */
    client_shared_memory *sh_mem = get_client_shared_memory();
    sh_mem->send_mem.packet_ready = PACKET_READY_FALSE;

    /* initialize packet sending thread */
    client_send_thread_args csta;
    csta.send_mem = &sh_mem->send_mem;
    csta.socket = client_socket;
    pthread_t sending_thread_id;
    if (pthread_create(&sending_thread_id, NULL, send_client_packets, (void *) &csta) != 0)
        return -1;
    sleep(THREAD_INIT_WAIT_TIME); /* wait until thread's local variables from its argument (recv_thread_args) are initialized */

    /* TEST (while there is no front-end) */
    send_join("Raivis", &sh_mem->send_mem);

    /* validate and process incoming server packets */
    receive_server_packets(client_socket, sh_mem);

    return 0;
} 