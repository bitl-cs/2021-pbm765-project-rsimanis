#ifndef _PONG_SERVER_H
#define _PONG_SERVER_H

#include "pong_game.h"
#include "pong_networking.h"


#define SERVER_SHARED_CLIENT_DATA_SIZE      (1 + MAX_NAME_SIZE + INPUT_SIZE + 1 + PACKET_FROM_CLIENT_MAX_SIZE)
#define SERVER_SHARED_MEMORY_SIZE           (1 + MAX_CLIENTS * (SERVER_SHARED_CLIENT_DATA_SIZE + (GAME_STATE_SIZE + (2 - 1)) / 2)) /* max memory if everyone plays 1v1 */
#define SERVER_RECV_MEMORY_SIZE             (1 + PACKET_HEADER_SIZE + PACKET_FROM_CLIENT_MAX_SIZE)
#define SERVER_SEND_MEMORY_SIZE             (1 + PACKET_ID_SIZE + PACKET_SIZE_SIZE + PACKET_FROM_SERVER_MAX_DATA_SIZE)

typedef struct _client_data {
    char *client_data;
    unsigned char *taken;
    char *name;
    char *input;
    char *packet_ready;
    char *packet_buf;
} client_data;

typedef struct _shared_memory_config {
    char *shared_memory;
    unsigned char *client_count;
    char *shared_client_data;
    char *shared_gamestate_data;
} shared_memory_config;

typedef struct proc_inc_packets_args {
    int id;
    int socket;
    int send_pn;
    client_data *client_data;
} proc_inc_packets_args;


/* server */
void accept_clients(int server_socket, shared_memory_config *sh_mem_cfg);
void get_shared_memory(shared_memory_config *sh_mem_cfg);
void gameloop(shared_memory_config *sh_mem_cfg);
void start_network(char *port, shared_memory_config *sh_mem_cfg);

/* client processing */
int find_free_client_id(shared_memory_config *sh_mem_cfg);
void process_client(int id, int socket, shared_memory_config *sh_mem_cfg, client_data client_data);
void remove_client(int id, int socket, shared_memory_config *sh_mem_cfg);
void *process_incoming_client_packets(void *arg);
char *get_client_data_ptr(int id, shared_memory_config *sh_mem_cfg);
void get_client_data(int id, shared_memory_config *sh_mem_cfg, client_data *client_data);

/* debug */
void print_shared_memory(shared_memory_config *sh_mem_cfg);

#endif