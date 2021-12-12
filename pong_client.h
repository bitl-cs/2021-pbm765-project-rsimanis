#ifndef _PONG_CLIENT_H
#define _PONG_CLIENT_H

#include "pong_networking.h"
#include "pong_server.h"


typedef struct _client_recv_memory {
    char packet_ready;
    char packet_buf[PACKET_FROM_SERVER_MAX_SIZE];
} client_recv_memory;

typedef struct _client_send_memory {
    char packet_ready;
    unsigned char pid;
    int32_t datalen;
    char pdata[PACKET_FROM_SERVER_MAX_DATA_SIZE];
} client_send_memory;

typedef struct _client_shared_memory {
    client_recv_memory recv_mem;
    client_send_memory send_mem;
} client_shared_memory;

typedef struct _client_recv_thread_args {
    int socket;
    client_recv_memory *recv_mem;
} client_recv_thread_args;

typedef struct _client_send_thread_args {
    int socket;
    client_send_memory *send_mem;
} client_send_thread_args;


/* init */
client_shared_memory *get_client_shared_memory(void);

/* packet processing */
void *receive_server_packets(void *arg);
void *send_client_packets(void *arg);

void process_server_packets(client_shared_memory *sh_mem);
void process_accept(char *data, client_send_memory *sh_mem);
void process_message_from_server(char *data, client_send_memory *sh_mem);
void process_lobby(char *data, client_send_memory *sh_mem);
void process_game_ready(char *data, client_send_memory *sh_mem);
void process_game_state(char *data, client_send_memory *sh_mem);
void process_game_end(char *data, client_send_memory *sh_mem);

void send_client_packet(uint32_t pn, int32_t psize, client_send_memory *send_mem, char *buf, char *final_buf, int socket);
void send_join(char *name, client_send_memory *sh_mem);
void send_message_from_client(char target_id, char source_id, char *message, client_send_memory *sh_mem);
void send_player_ready(char player_id, client_send_memory *sh_mem);
void send_player_input(char input, client_send_memory *sh_mem);
void send_check_status(client_send_memory *sh_mem);
void send_game_type(char type, client_send_memory *sh_mem);

#endif