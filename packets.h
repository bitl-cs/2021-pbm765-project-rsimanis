#ifndef _PACKETS_H
#define _PACKETS_H

#include <inttypes.h>
#include <stddef.h>
#include <limits.h>
#include "pong.h"


/* game */
#define MAX_NAME_SIZE                       20
#define KEYBOARD_INPUT_SIZE                 1
#define GAME_STATISTICS_SIZE                112
#define GAMEBOARD_STATE_SIZE                176
#define GAME_STATE_SIZE                     GAMEBOARD_STATE_SIZE + 0 /* ?????? */

/* general */
#define PACKET_NUMBER_SIZE                  4
#define PACKET_ID_SIZE                      4
#define PACKET_SIZE_SIZE                    8
#define PACKET_CHECKSUM_SIZE                1
#define PACKET_HEADER_SIZE                  (PACKET_NUMBER_SIZE + PACKET_ID_SIZE + PACKET_SIZE_SIZE + PACKET_CHECKSUM_SIZE)
#define PACKET_SEPARATOR_SIZE               2
#define PACKET_STATUS_SIZE                  4
#define PACKET_GAME_TYPE_SIZE               4
#define PACKET_MAX_SIZE                     GAME_STATE_PACKET_SIZE 
#define PACKET_MAX_DATA_SIZE                GAME_STATE_PACKET_MAX_DATA_SIZE
#define PACKET_MAX_ERROR_MESSAGE_SIZE       100

/* specific */
#define CLIENT_PACKET_MAX_DATA_SIZE         JOIN_PACKET_DATA_SIZE
#define SERVER_PACKET_MAX_DATA_SIZE         GAME_STATE_PACKET_MAX_DATA_SIZE

#define JOIN_PACKET_SIZE                    (PACKET_HEADER_SIZE + JOIN_PACKET_DATA_SIZE)
#define JOIN_PACKET_DATA_SIZE               MAX_NAME_SIZE

#define LOBBY_PACKET_SIZE                   (PACKET_HEADER_SIZE + LOBBY_PACKET_DATA_SIZE)
#define LOBBY_PACKET_MAX_DATA_SIZE              (PACKET_STATUS_SIZE + PACKET_MAX_ERROR_MESSAGE_SIZE)
 
#define GAME_TYPE_PACKET_SIZE               (PACKET_HEADER_SIZE + GAME_TYPE_PACKET_DATA_SIZE)
#define GAME_TYPE_PACKET_DATA_SIZE          PACKET_GAME_TYPE_SIZE
 
#define PLAYER_QUEUE_PACKET_SIZE            (PACKET_HEADER_SIZE + PLAYER_QUEUE_PACKET_DATA_SIZE)
#define PLAYER_QUEUE_PACKET_MAX_DATA_SIZE   (PACKET_STATUS_SIZE + PACKET_MAX_ERROR_MESSAGE_SIZE)
 
#define GAME_READY_PACKET_SIZE              (PACKET_HEADER_SIZE + GAME_READY_PACKET_DATA_SIZE)
#define GAME_READY_PACKET_MAX_DATA_SIZE     (PACKET_STATUS_SIZE + PACKET_MAX_ERROR_MESSAGE_SIZE)
 
#define PLAYER_READY_PACKET_SIZE            (PACKET_HEADER_SIZE + PLAYER_READY_PACKET_DATA_SIZE)
#define PLAYER_READY_PACKET_DATA_SIZE       0

#define GAME_STATE_PACKET_SIZE              (PACKET_HEADER_SIZE + GAME_STATE_PACKET_MAX_DATA_SIZE)
#define GAME_STATE_PACKET_MAX_DATA_SIZE     GAMEBOARD_STATE_SIZE

#define PLAYER_INPUT_PACKET_SIZE            (PACKET_HEADER_SIZE + INPUT_PACKET_DATA_SIZE)
#define PLAYER_INPUT_PACKET_DATA_SIZE       KEYBOARD_INPUT_SIZE

#define CHECK_STATUS_PACKET_SIZE            (PACKET_HEADER_SIZE + CHECK_STATUS_PACKET_DATA_SIZE)
#define CHECK_STATUS_PACKET_DATA_SIZE       0

#define GAME_END_PACKET_SIZE                (PACKET_HEADER_SIZE + GAME_TYPE_PACKET_DATA_SIZE)
#define GAME_END_PACKET_MAX_DATA_SIZE       GAME_STATISTICS_SIZE

/* global constants */
static const char *PACKET_SEPARATOR = "--";

/* packet processing */
void send_packet(int32_t pn, int pid, char *data, size_t datalen, size_t packet_data_size, int socket);
void send_join(char *name, int32_t pn, int socket);
void process_join(void *data);
void send_lobby(int status, char *error, int32_t pn, int socket);
void process_lobby(void *data);
void send_game_type(int type, int32_t pn, int socket);
void process_game_type(void *data);
void send_player_queue(int status, char *error, int32_t pn, int socket);
void process_player_queue(void *data);
void send_game_ready(int status, char *error, int32_t pn, int socket);
void process_game_ready(void *data);
void send_player_ready(int32_t pn, int socket);
void process_player_ready();
void send_game_state(game_state *gs, int32_t pn, int socket);
void process_game_state(void *data);
void send_player_input(char input, int32_t pn, int socket);
void process_player_input(void *data);
void send_check_status(int32_t pn, int socket);
void process_check_status();
void send_game_end(int status, char *error, game_statistics *gs, int32_t pn, int socket);
void process_game_end(void *data);

/* utilities */
int encode(char *data, size_t datalen, char *buf, size_t buflen);
int decode(char *data, size_t datalen, char *buf, size_t buflen);
char xor_checksum(char *data, size_t len);

/* debug */
void print_bytes(char *start, size_t len);
void print_bytes_full(char *start, size_t len);

/* helpers */
size_t insert_bytes(char *data, size_t datalen, char *buf, size_t buflen, size_t offset);
size_t insert_int32_t(int32_t x, char *buf, size_t buflen, size_t offset);
size_t insert_int64_t(int64_t x, char *buf, size_t buflen, size_t offset);
size_t insert_char(char x, char *buf, size_t buflen, size_t offset);
size_t insert_str(char *str, size_t strlen, char *buf, size_t buflen, size_t offset);
size_t insert_null_bytes(int count, char *buf, size_t buflen, size_t offset);
size_t insert_separator(char *buf, size_t buflen, size_t offset);

void host_to_network_bytes(char *start, size_t len);
short host_to_network_short(short x);
int host_to_network_int(int x);
long host_to_network_long(long x);
float host_to_network_float(float x);
double host_to_network_double(double x);
int32_t host_to_network_int32_t(int32_t x);
int64_t host_to_network_int64_t(int64_t x);

void network_to_host_bytes(char *start, size_t len);
short network_to_host_short(short x);
int network_to_host_int(int x);
long network_to_host_long(long x);
float network_to_host_float(float x);
double network_to_host_double(double x);
int32_t network_to_host_int32_t(int32_t x);
int64_t network_to_host_int64_t(int64_t x);

int is_little_endian_system();
char printable_char(char c);

#endif