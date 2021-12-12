#ifndef _PONG_SERVER_H
#define _PONG_SERVER_H

#include "pong_game.h"
#include "pong_networking.h"

typedef struct _server_recv_memory {
    char packet_ready;
    char packet_buf[PACKET_FROM_CLIENT_MAX_SIZE];
} server_recv_memory;

typedef struct _server_send_memory {
    char packet_ready;
    unsigned char pid;
    int32_t datalen;
    char pdata[PACKET_FROM_SERVER_MAX_DATA_SIZE];
} server_send_memory;

typedef struct _client client;
typedef struct _lobby {
    char player_count;
    client *players[MAX_PLAYER_COUNT];
} lobby;

typedef struct _client {
    char id;
    int socket;
    char name[MAX_NAME_SIZE];
    char state;
    lobby *lobby;
    game_state *game_state;
    server_recv_memory recv_mem;
    server_send_memory send_mem;
} client;

typedef struct _server_shared_memory {
    unsigned char client_count;
    client clients[MAX_CLIENTS];
    lobby lobby_1v1;
    lobby lobby_2v2;
    game_state game_states[(MAX_CLIENTS + (2 - 1)) / 2];
} server_shared_memory;

typedef struct _server_recv_send_thread_args {
    client *client;
    server_shared_memory *sh_mem;
} server_recv_send_thread_args;


/* init */
server_shared_memory *get_server_shared_memory(void);
void start_network(char *port, server_shared_memory *sh_mem);
void init_lobby(lobby *lob);

/* game */
void gameloop(server_shared_memory *sh_mem);

/* client processing */
void accept_clients(int server_socket, server_shared_memory *sh_mem);
client *init_client(server_shared_memory *sh_mem, int socket);
void process_client(client *client, server_shared_memory *sh_mem);
void remove_client(client *client, server_shared_memory *sh_mem);

/* packet processing */
void *receive_client_packets(void *arg);
void *send_server_packets(void *arg);

void process_client_packets(client *client);
void process_join(char *data, client *client);
void process_message_from_client(char *data, client *client);
void process_player_ready(client *client);
void process_player_input(char *data, client *client);
void process_check_status(client *client);
void process_game_type(char *data, client *client);

void send_server_packet(uint32_t pn, int32_t psize, server_send_memory *send_mem, char *buf, char *final_buf, int socket);
void send_accept(char player_id, client *client);
void send_message_from_server(char type, char source_id, char *message, client *client);
void send_lobby(client *client);
void send_game_ready(client *client);
void send_game_state(client *client);
void send_game_end(client *client);

/* helpers */
int find_team_score(client *client);
int is_alphanum(char *data, size_t datalen);

/* debug */
void print_client(client *client);
void print_shared_memory(server_shared_memory *sh_mem);

#endif