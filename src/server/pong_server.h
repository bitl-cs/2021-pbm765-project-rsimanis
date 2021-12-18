#ifndef _PONG_SERVER_H
#define _PONG_SERVER_H

#include "../game/pong_game.h"
#include "../networking/pong_networking.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <arpa/inet.h>

#define MAX_GAME_STATES                 (MAX_CLIENTS / 2)   /* max number of game states at any moment (this number is achieved if maximum number of clients has connected and everyone plays a 1v1 match) */
#define CLIENT_ID_TAKEN_FALSE           -1                  /* when client memory is not occupied, its id is set to this value */
#define LOBBY_UPDATE_INTERVAL           1/5.0               /* time interval (in seconds) between two lobby updates */

#define STATISTICS_DURATION             20 

#define CLIENT_STATE_JOIN               0                   /* player sees the join screen */
#define CLIENT_STATE_MENU               1                   /* player sees the main menu (1v1 and 2v2 buttons) */
#define CLIENT_STATE_LOBBY              2                   /* player sees his/her lobby */
#define CLIENT_STATE_LOADING            3                   /* player sees the game loading screen before match */
#define CLIENT_STATE_GAME               4                   /* player sees the game - he is playing pong (hopefully) */ 
#define CLIENT_STATE_STATISTICS         5                   /* player sees the statistics screen (or error if game finished erroneously*/

typedef struct _server_recv_memory {
    char packet_buf[PACKET_FROM_CLIENT_MAX_SIZE];
} server_recv_memory;

typedef struct _server_send_memory {
    char packet_ready;
    unsigned char pid;
    int32_t datalen;
    char pdata[PACKET_FROM_SERVER_MAX_DATA_SIZE];
} server_send_memory;

typedef struct _client client;  /* forward declaration */
typedef struct _lobby {
    clock_t last_update;
    char max_clients;
    char client_count;
    client *clients[MAX_PLAYER_COUNT];
} lobby;

typedef struct _client {
    char id;
    int socket;
    char name[MAX_NAME_LENGTH + 1];
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

typedef struct _server_send_thread_args {
    client *client;
    server_shared_memory *sh_mem;
} server_send_thread_args;


/* init */
server_shared_memory *get_server_shared_memory(void);
void init_server(server_shared_memory *sh_mem);
void start_network(char *port, server_shared_memory *sh_mem);
void init_lobby(lobby *lobby, int max_clients);
void init_client(char id, int socket, client *c);

void init_teams(game_state *gs, int player_count);
void init_back_players(game_state *gs, lobby *lobby);
void init_front_players(game_state *gs, lobby *lobby);
void init_power_ups(game_state *gs);
void init_game_1v1(game_state *gs, lobby *lobby);
void init_game_2v2(game_state *gs, lobby *lobby);

/* game */
void gameloop(server_shared_memory *sh_mem);
void update_game(game_state *gs, server_shared_memory *sh_mem);
void reset_lobby(lobby *lobby);
void update_lobby(lobby *lobby, server_shared_memory *sh_mem);
void send_lobby_to_all_clients_in_the_lobby(lobby *lobby, server_shared_memory *sh_mem);
void send_game_ready_to_all_players(game_state *gs, server_shared_memory *sh_mem);
void send_game_state_to_all_players(game_state *gs, server_shared_memory *sh_mem);
void send_game_statistics_to_all_players(game_state *gs, server_shared_memory *sh_mem);
void send_return_to_menu_to_all_players(game_state *gs, server_shared_memory *sh_mem);
void send_return_to_menu_to_player(player *p, server_shared_memory *sh_mem);
void end_game_for_all_players(game_state *gs, server_shared_memory *sh_mem);
void finish_game_state(game_state *gs, server_shared_memory *sh_mem);
void set_all_player_client_status_to_game(game_state *gs, server_shared_memory *sh_mem);

/* client processing */
void accept_clients(int server_socket, server_shared_memory *sh_mem);
char get_free_client(int socket, server_shared_memory *sh_mem);
void process_client(client *client, server_shared_memory *sh_mem);
void remove_client(client *client, server_shared_memory *sh_mem);
void reset_client(client *client);
void remove_client_from_lobby(client *client);

/* packet processing */
void receive_client_packets(client *client, server_shared_memory *sh_mem);
void process_client_packets(client *client, server_shared_memory *sh_mem);
void process_join(char *data, client *client, server_shared_memory *sh_mem);
void process_message_from_client(char *data, client *client, server_shared_memory *sh_mem);
void process_player_ready(client *client, server_shared_memory *sh_mem);
void process_player_input(char *data, client *client, server_shared_memory *sh_mem);
void process_check_status(client *client);
void process_game_type(char *data, client *client, server_shared_memory *sh_mem);

void *send_server_packets(void *arg);
void send_server_packet(uint32_t pn, int32_t psize, server_send_memory *send_mem, char *buf, char *final_buf, int socket);
void send_accept(char player_id, client *client);
void send_message_from_server(char type, char source_id, char *message, client *client);
void send_lobby(client *client);
void send_game_ready(client *client);
void send_game_state(client *client);
void send_game_end(client *client);
void send_return_to_menu(client *client);
void send_return_to_join(client *client);

/* helpers */
void remove_client_from_game_state(client *client);
char get_free_game_state(server_shared_memory *sh_mem);
void add_client_to_lobby(client* client, lobby *lobby);
int is_alphanum(char *data, size_t datalen);

/* debug */
void print_client(client *client);
void print_lobby(lobby *lobby);
void print_shared_memory(server_shared_memory *sh_mem);

#endif