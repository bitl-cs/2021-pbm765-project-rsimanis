#ifndef _PONG_SERVER_H
#define _PONG_SERVER_H

#include "pong_game.h"
#include "pong_networking.h"


#define CLIENT_COUNT_SIZE                   1
#define CLIENT_TAKEN_SIZE                   1
#define CLIENT_STATE_SIZE                   1
#define SERVER_RECV_MEMORY_SIZE             (PACKET_READY_SIZE + PACKET_FROM_CLIENT_MAX_SIZE)
#define SERVER_SEND_MEMORY_SIZE             (PACKET_READY_SIZE + PACKET_ID_SIZE + PACKET_SIZE_SIZE + PACKET_FROM_SERVER_MAX_DATA_SIZE)
#define SERVER_SHARED_CLIENT_DATA_SIZE      (CLIENT_TAKEN_SIZE + MAX_NAME_SIZE + INPUT_SIZE + CLIENT_STATE_SIZE + SERVER_RECV_MEMORY_SIZE + SERVER_SEND_MEMORY_SIZE)
#define SERVER_SHARED_MEMORY_SIZE           (CLIENT_COUNT_SIZE + MAX_CLIENTS * SERVER_SHARED_CLIENT_DATA_SIZE + (MAX_CLIENTS + (2 - 1)) / 2 * GAME_STATE_SIZE) /* max memory if everyone plays 1v1 */

typedef struct _server_server_shared_memory_config {
    char *shared_memory;
    unsigned char *client_count;
    char *shared_client_data;
    char *shared_gamestate_data;
} server_shared_memory_config;

typedef struct _client_data {
    int id;
    int socket;
    char *client_data;
    unsigned char *taken;
    char *name;
    char *input;
    char *state;
    recv_memory_config recv_mem_cfg;
    send_memory_config send_mem_cfg;
} client_data;

typedef struct _server_recv_thread_args {
    client_data *cd;
    server_shared_memory_config *sh_mem_cfg;
} server_recv_thread_args;

/* init */
void get_shared_memory(server_shared_memory_config *sh_mem_cfg);
void start_network(char *port, server_shared_memory_config *sh_mem_cfg);

/* game */
void gameloop(server_shared_memory_config *sh_mem_cfg);

/* client processing */
void accept_clients(int server_socket, server_shared_memory_config *sh_mem_cfg);
int find_free_client_id(server_shared_memory_config *sh_mem_cfg);
void process_client(int id, int socket, server_shared_memory_config *sh_mem_cfg);
void remove_client(client_data *cd, server_shared_memory_config *sh_mem_cfg);
char *get_client_data_ptr(int id, server_shared_memory_config *sh_mem_cfg);
void get_client_data(int id, int socket, server_shared_memory_config *sh_mem_cfg, client_data *client_data);

/* packet processing */
void *receive_client_packets(void *arg);
void *send_server_packets(void *arg);
void process_client_packets(client_data *cd);
void process_join(char *data, client_data *cd);
void process_message_from_client(char *data, client_data *cd);
void process_player_ready(client_data *cd);
void process_player_input(char *data, client_data *cd);
void process_check_status(client_data *cd);

/* debug */
void print_client_data(client_data *cd);
void print_shared_memory(server_shared_memory_config *sh_mem_cfg);

#endif