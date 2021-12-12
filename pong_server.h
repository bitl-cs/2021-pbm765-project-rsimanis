#ifndef _PONG_SERVER_H
#define _PONG_SERVER_H

#include "pong_game.h"
#include "pong_networking.h"

#include <time.h>

#define MAX_GAME_STATES                 (MAX_CLIENTS / 2)
#define CLIENT_ID_TAKEN_FALSE           -1
#define LOBBYLOOP_UPDATE_INTERVAL       1/70.0      /* in seconds */

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

typedef struct _client client;  /* forward declaration */
struct _client;
typedef struct _lobby {
    clock_t last_update;
    char max_players;
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
    player *player;
    server_recv_memory recv_mem;
    server_send_memory send_mem;
} client;

typedef struct _server_shared_memory {
    unsigned char client_count;
    client clients[MAX_CLIENTS];
    lobby lobby_1v1;
    lobby lobby_2v2;
    game_state game_states[MAX_GAME_STATES];
} server_shared_memory;

typedef struct _server_recv_send_thread_args {
    client *client;
    server_shared_memory *sh_mem;
} server_recv_send_thread_args;


/* init */
server_shared_memory *get_server_shared_memory(void);
void init_server(server_shared_memory *sh_mem);
void start_network(char *port, server_shared_memory *sh_mem);
void init_lobby(lobby *lobby, int max_players);

/* game */
void reset_lobby(lobby *lobby);
void lobbyloop(server_shared_memory *sh_mem);
void update_lobby(lobby *lobby, server_shared_memory *sh_mem);
void gameloop(server_shared_memory *sh_mem);

/* client processing */
void accept_clients(int server_socket, server_shared_memory *sh_mem);
client *init_client(server_shared_memory *sh_mem, int socket);
void process_client(client *client, server_shared_memory *sh_mem);
void remove_client(client *client, server_shared_memory *sh_mem);

/* packet processing */
void *receive_client_packets(void *arg);
void *send_server_packets(void *arg);

void process_client_packets(client *client, server_shared_memory *sh_mem);
void process_join(char *data, client *client);
void process_message_from_client(char *data, client *client);
void process_player_ready(client *client);
void process_player_input(char *data, client *client);
void process_check_status(client *client);
void process_game_type(char *data, client *client, server_shared_memory *sh_mem);

void send_server_packet(uint32_t pn, int32_t psize, server_send_memory *send_mem, char *buf, char *final_buf, int socket);
void send_accept(char player_id, client *client);
void send_message_from_server(char type, char source_id, char *message, client *client);
void send_lobby(client *client);
void send_game_ready(client *client);
void send_game_state(client *client);
void send_game_end(client *client);

/* helpers */
client *find_client_by_id(char id, server_shared_memory *sh_mem);
game_state *find_free_game_state(server_shared_memory *sh_mem);
void add_client_to_lobby(client* client, lobby *lobby);
int find_team_score(client *client);
int is_alphanum(char *data, size_t datalen);

/* debug */
void print_client(client *client);
void print_shared_memory(server_shared_memory *sh_mem);

#endif