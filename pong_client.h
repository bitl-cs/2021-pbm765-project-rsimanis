#ifndef _PONG_CLIENT_H
#define _PONG_CLIENT_H

#include "pong_networking.h"


#define CLIENT_SHARED_MEMORY_SIZE           (CLIENT_RECV_MEMORY_SIZE + CLIENT_SEND_MEMORY_SIZE)  /* 2 * packet_ready_size + ... */
#define CLIENT_RECV_MEMORY_SIZE             (1 + PACKET_HEADER_SIZE + PACKET_FROM_SERVER_MAX_SIZE)
#define CLIENT_SEND_MEMORY_SIZE             (1 + PACKET_ID_SIZE + PACKET_SIZE_SIZE + PACKET_FROM_CLIENT_MAX_DATA_SIZE)


typedef struct _client_shared_memory_config {
    char *shared_memory;
    recv_memory_config recv_mem_cfg;
    send_memory_config send_mem_cfg;
} client_shared_memory_config;

typedef struct _client_thread_args {
    int client_socket;
    client_shared_memory_config *sh_mem_cfg;
} client_thread_args;


void get_client_shared_memory(client_shared_memory_config *sh_mem_cfg);
void process_server_packets(recv_memory_config *recv_mem_cfg);
void *receive_server_packets(void *arg);
void *send_client_packets(void *arg);

#endif