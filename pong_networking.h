#ifndef _PACKETS_H
#define _PACKETS_H

#include "pong_game.h"

#include "inttypes.h"
#include <stddef.h>
#include <limits.h>
#include <stdint.h>


#define MAX_CLIENTS                         1

/* general */
#define PACKET_NUMBER_SIZE                  4 
#define PACKET_ID_SIZE                      1 
#define PACKET_SIZE_SIZE                    4 
#define PACKET_CHECKSUM_SIZE                1 
#define PACKET_HEADER_SIZE                  (PACKET_NUMBER_SIZE + PACKET_ID_SIZE + PACKET_SIZE_SIZE)
#define PACKET_FOOTER_SIZE                  PACKET_CHECKSUM_SIZE
#define PACKET_SEPARATOR_SIZE               (sizeof(PACKET_SEPARATOR) - 1)

#define PACKET_MAX_SIZE                     PACKET_GAME_STATE_MAX_SIZE 
#define PACKET_MAX_DATA_SIZE                PACKET_GAME_STATE_MAX_DATA_SIZE
#define PACKET_FROM_CLIENT_MAX_SIZE         PACKET_MESSAGE_SIZE
#define PACKET_FROM_CLIENT_MAX_DATA_SIZE    PACKET_MESSAGE_DATA_SIZE
#define PACKET_FROM_SERVER_MAX_SIZE         PACKET_GAME_STATE_MAX_SIZE
#define PACKET_FROM_SERVER_MAX_DATA_SIZE    PACKET_GAME_STATE_MAX_DATA_SIZE

#define PACKET_READY_TRUE                   1
#define PACKET_READY_FALSE                  0
#define PACKET_READY_WAIT_TIME              1/100.0

#define DEFAULT_PORT                        "12345"
#define DEFAULT_IP                          "127.0.0.1"
#define PACKET_SEPARATOR                    "--"

/* specific */
#define PACKET_JOIN_ID                      1
#define PACKET_JOIN_SIZE                    (PACKET_HEADER_SIZE + PACKET_JOIN_DATA_SIZE + PACKET_FOOTER_SIZE)
#define PACKET_JOIN_DATA_SIZE               20
#define PACKET_ACCEPT_ID                    2
#define PACKET_ACCEPT_SIZE                  (PACKET_HEADER_SIZE + PACKET_ACCEPT_DATA_SIZE + PACKET_FOOTER_SIZE)
#define PACKET_ACCEPT_DATA_SIZE             1
#define PACKET_MESSAGE_ID                   3
#define PACKET_MESSAGE_SIZE                 (PACKET_HEADER_SIZE + PACKET_MESSAGE_DATA_SIZE + PACKET_FOOTER_SIZE)
#define PACKET_MESSAGE_DATA_SIZE            258
#define PACKET_LOBBY_ID                     4
#define PACKET_LOBBY_MAX_SIZE               (PACKET_HEADER_SIZE + PACKET_LOBBY_MAX_DATA_SIZE + PACKET_FOOTER_SIZE)
#define PACKET_LOBBY_MAX_DATA_SIZE          (1 + MAX_PLAYER_COUNT * 21)
#define PACKET_GAME_READY_ID                5
#define PACKET_GAME_READY_MAX_SIZE          (PACKET_HEADER_SIZE + PACKET_GAME_READY_MAX_DATA_SIZE + PACKET_FOOTER_SIZE)
#define PACKET_GAME_READY_MAX_DATA_SIZE     (10 + MAX_TEAM_COUNT * 17 + MAX_PLAYER_COUNT * 39)
#define PACKET_PLAYER_READY_ID              6
#define PACKET_PLAYER_READY_SIZE            (PACKET_HEADER_SIZE + PACKET_PLAYER_READY_DATA_SIZE + PACKET_FOOTER_SIZE)
#define PACKET_PLAYER_READY_DATA_SIZE       1
#define PACKET_GAME_STATE_ID                7
#define PACKET_GAME_STATE_MAX_SIZE          (PACKET_HEADER_SIZE + PACKET_GAME_STATE_MAX_DATA_SIZE + PACKET_FOOTER_SIZE)
#define PACKET_GAME_STATE_MAX_DATA_SIZE     (12 + MAX_TEAM_COUNT * 21 + MAX_PLAYER_COUNT * 18 + MAX_BALL_COUNT * 13 + MAX_POWERUP_COUNT * 17)
#define PACKET_PLAYER_INPUT_ID              8
#define PACKET_PLAYER_INPUT_SIZE            (PACKET_HEADER_SIZE + PACKET_PLAYER_INPUT_DATA_SIZE + PACKET_FOOTER_SIZE)
#define PACKET_PLAYER_INPUT_DATA_SIZE       1
#define PACKET_CHECK_STATUS_ID              9
#define PACKET_CHECK_STATUS_SIZE            (PACKET_HEADER_SIZE + PACKET_CHECK_STATUS_DATA_SIZE + PACKET_FOOTER_SIZE)
#define PACKET_CHECK_STATUS_DATA_SIZE       0
#define PACKET_GAME_END_ID                  10
#define PACKET_GAME_END_MAX_SIZE            (PACKET_HEADER_SIZE + PACKET_GAME_END_MAX_DATA_SIZE + PACKET_FOOTER_SIZE)
#define PACKET_GAME_END_MAX_DATA_SIZE       (11 + MAX_TEAM_COUNT * 5 + MAX_PLAYER_COUNT * 26)


/* structs */
typedef struct _recv_memory_config {
    char *recv_memory;
    char *packet_ready;
    char *packet;
    uint32_t *pn;
    unsigned char *pid;
    int32_t *psize;
    char *pdata;
    int32_t pdata_buflen;
} recv_memory_config;

typedef struct _send_memory_config {
    char *send_memory;
    char *packet_ready;
    unsigned char *pid;
    int32_t *datalen;
    char *pdata;
    int32_t pdata_buflen;
} send_memory_config;

typedef struct _recv_thread_args {
    int socket;
    recv_memory_config *recv_mem_cfg;
} recv_thread_args;

typedef struct _send_thread_args {
    int socket;
    send_memory_config *send_mem_cfg;
} send_thread_args;


/* intialization */
int get_host_parameter(int argc, char **argv, char *host);
int get_port_parameter(int argc, char **argv, char *port);
int get_server_socket(char *port);
int get_client_socket(char *host, char *port);
void get_recv_memory_config(char *recv_mem, int32_t pdata_buflen, recv_memory_config *recv_mem_cfg);
void get_send_memory_config(char *send_mem, int32_t pdata_buflen, send_memory_config *send_mem_cfg);
int init_recv_thread(int socket, recv_memory_config *recv_mem_cfg);
int init_send_thread(int socket, send_memory_config *send_mem_cfg, void *(*send_packets)(void *arg));
void *receive_packets(void *arg);

/* packets */
void send_packet(uint32_t pn, int32_t psize, send_memory_config *send_mem_cfg, int socket);
void send_join(char *name, send_memory_config *send_mem_cfg);
// void send_accept(char player_id, send_memory_config *send_mem_cfg);
// void send_message(char type, char source_id, char *message, send_memory_config *send_mem_cfg);
// void send_lobby(game_lobby *game_lobby, send_memory_config *send_mem_cfg);
// void send_game_ready(game_state *game_state, send_memory_config *send_mem_cfg);
// void send_player_ready(char player_id, send_memory_config *send_mem_cfg);
// void send_game_state(game_state *game_state, send_memory_config *send_mem_cfg);
// void send_player_input(char input, send_memory_config *send_mem_cfg);
// void send_check_status(send_memory_config *send_mem_cfg);
// void send_game_end(game_state *game_state, send_memory_config *send_mem_cfg);

/* debug */
void print_bytes(void *start, size_t len);
void print_bytes_full(void *start, size_t len);
void print_recv_memory(recv_memory_config *recv_mem_cfg);
void print_send_memory(send_memory_config *send_mem_cfg);

/* helpers */
int encode(char *data, size_t datalen, char *buf, size_t buflen);
int decode(char *data, size_t datalen, char *buf, size_t buflen);
char xor_checksum(char *data, size_t len);
int verify_packet(uint32_t recv_pn, recv_memory_config *recv_mem_cfg, int32_t decoded_psize, char decoded_checksum);

size_t insert_bytes(char *data, size_t datalen, char *buf, size_t buflen, size_t offset);
size_t insert_int32_t(int32_t x, char *buf, size_t buflen, size_t offset);
size_t insert_int64_t(int64_t x, char *buf, size_t buflen, size_t offset);
size_t insert_char(char x, char *buf, size_t buflen, size_t offset);
size_t insert_str(char *str, size_t strlen, char *buf, size_t buflen, size_t offset);
size_t insert_null_bytes(int count, char *buf, size_t buflen, size_t offset);
size_t insert_separator(char *buf, size_t buflen, size_t offset);

size_t insert_bytes_as_big_endian(char *data, size_t datalen, char *buf, size_t buflen, size_t offset);
size_t insert_short_as_big_endian(short x, char *buf, size_t buflen, size_t offset);
size_t insert_int_as_big_endian(int x, char *buf, size_t buflen, size_t offset);
size_t insert_long_as_big_endian(long x, char *buf, size_t buflen, size_t offset);
size_t insert_float_as_big_endian(float x, char *buf, size_t buflen, size_t offset);
size_t insert_double_as_big_endian(double x, char *buf, size_t buflen, size_t offset);
size_t insert_int32_t_as_big_endian(int32_t x, char *buf, size_t buflen, size_t offset);
size_t insert_uint32_t_as_big_endian(uint32_t x, char *buf, size_t buflen, size_t offset);
size_t insert_int64_t_as_big_endian(int64_t x, char *buf, size_t buflen, size_t offset);
size_t insert_uint64_t_as_big_endian(uint64_t x, char *buf, size_t buflen, size_t offset);

void host_to_big_endian_bytes(char *start, size_t len);
short host_to_big_endian_short(short x);
int host_to_big_endian_int(int x);
long host_to_big_endian_long(long x);
float host_to_big_endian_float(float x);
double host_to_big_endian_double(double x);
int32_t host_to_big_endian_int32_t(int32_t x);
uint32_t host_to_big_endian_uint32_t(uint32_t x);
int64_t host_to_big_endian_int64_t(int64_t x);
uint64_t host_to_big_endian_uint64_t(uint64_t x);

void big_endian_to_host_bytes(char *start, size_t len);
short big_endian_to_host_short(short x);
int big_endian_to_host_int(int x);
long big_endian_to_host_long(long x);
float big_endian_to_host_float(float x);
double big_endian_to_host_double(double x);
int32_t big_endian_to_host_int32_t(int32_t x);
uint32_t big_endian_to_host_uint32_t(uint32_t x);
int64_t big_endian_to_host_int64_t(int64_t x);
uint64_t big_endian_to_host_uint64_t(uint64_t x);

int is_little_endian_system();
char printable_char(char c);

int validate_number(char *str);
int validate_ip(char *str);

#endif