#include "pong_client.h"

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

render_data rend_data;
input_data inp_data;

int main(int argc, char **argv) {
    char port[6] = DEFAULT_PORT;
    char host[255] = DEFAULT_IP;
    int client_socket = -1;

    /* get port and host from argv */
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

    /* initialize thread arguments */
    client_thread_args cta;
    cta.sh_mem = sh_mem;
    cta.socket = client_socket;

    /* initialize packet sending thread */
    pthread_t sending_thread_id;
    if (pthread_create(&sending_thread_id, NULL, send_client_packets, (void *) &cta) != 0)
        return -1;
    sleep(THREAD_INIT_WAIT_TIME); /* wait until thread's local variables from its argument are initialized */

    /* initialize packet receiving thread */
    pthread_t receiving_thread_id;
    if (pthread_create(&receiving_thread_id, NULL, receive_server_packets, (void *) &cta) != 0)
        return -1;
    sleep(THREAD_INIT_WAIT_TIME); /* wait until thread's local variables from its argument are initialized */

    /* initialize global variables for OpenGL */
    init_input_data();
    init_render_data(sh_mem);

    /* initialize graphics thread */
    init_graphics_window(argc, argv);

    return 0;
} 