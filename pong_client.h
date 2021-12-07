#ifndef _PONG_CLIENT_H
#define _PONG_CLIENT_H

#include "pong_networking.h"


#define SHARED_PACKET_READY_SIZE        sizeof(char) 
#define SHARED_MEMORY_SIZE              (2 * SHARED_PACKET_READY_SIZE + SERVER_PACKET_MAX_SIZE + PACKET_ID_SIZE + CLIENT_PACKET_MAX_DATA_SIZE) 

typedef struct _client_shared_memory_config {
    char *shared_memory;
    char *recv_packet_ready;
    char *recv_packet_buf;
    packet_info recv_packet_info;
    char *send_packet_ready;
    unsigned char *send_packet_id;
    char *send_packet_data;
} client_shared_memory_config;

typedef struct _client_thread_args {
    int client_socket;
    client_shared_memory_config *sh_mem_cfg;
} client_thread_args;

void get_client_shared_memory(client_shared_memory_config *sh_mem_cfg);
void process_server_packets(client_shared_memory_config *sh_mem_cfg);
void *receive_server_packets(void *arg);
void *send_client_packets(void *arg);

#endif