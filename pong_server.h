#ifndef _PONG_SERVER_H
#define _PONG_SERVER_H

#include "pong_game.h"
#include "pong_networking.h"


#define SHARED_CLIENT_COUNT_SIZE            sizeof(char)
#define SHARED_CLIENT_PACKET_READY_SIZE     sizeof(char) 
#define SHARED_CLIENT_TAKEN_SIZE            sizeof(char) 
#define SHARED_CLIENT_DATA_SIZE             (SHARED_CLIENT_TAKEN_SIZE + MAX_NAME_SIZE + INPUT_SIZE + SHARED_CLIENT_PACKET_READY_SIZE + CLIENT_PACKET_MAX_SIZE)
#define SHARED_MEMORY_SIZE                  (SHARED_CLIENT_COUNT_SIZE + MAX_CLIENTS * (SHARED_CLIENT_DATA_SIZE + GAME_STATE_SIZE / 2))

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
void *process_incoming_packets(void *arg);
char *get_client_data_ptr(int id, shared_memory_config *sh_mem_cfg);
client_data get_client_data(int id, shared_memory_config *sh_mem_cfg);

/* debug */
void print_shared_memory(shared_memory_config *sh_mem_cfg);

#endif