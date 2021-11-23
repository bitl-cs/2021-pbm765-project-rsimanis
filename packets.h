#ifndef _PACKETS_H
#define _PACKETS_H

#include <inttypes.h>
#include <stddef.h>
#include <limits.h>
#include "pong.h"


/* game */
#define MAX_NAME_SIZE                       20
#define KEYBOARD_INPUT_SIZE                 1

/* general */
#define PACKET_NUMBER_SIZE                  4
#define PACKET_ID_SIZE                      4
#define PACKET_SIZE_SIZE                    8
#define PACKET_CHECKSUM_SIZE                1
#define PACKET_HEADER_SIZE                  PACKET_NUMBER_SIZE + PACKET_ID_SIZE + PACKET_SIZE_SIZE + PACKET_CHECKSUM_SIZE
#define PACKET_SEPARATOR_SIZE               2

/* specific */
#define CLIENT_PACKET_MAX_DATA_SIZE         JOIN_PACKET_DATA_SIZE
#define SERVER_PACKET_MAX_DATA_SIZE         GAME_STATE_PACKET_MAX_DATA_SIZE

#define JOIN_PACKET_SIZE                    PACKET_HEADER_SIZE + JOIN_PACKET_DATA_SIZE 
#define JOIN_PACKET_DATA_SIZE               MAX_NAME_SIZE

#define GAME_STATE_PACKET_MAX_DATA_SIZE     176

#define INPUT_PACKET_DATA_SIZE              KEYBOARD_INPUT_SIZE


/* global constants */
static const char *PACKET_SEPARATOR = "--";

/* packet processing */
void send_join(char *name, int pn, int socket);
void process_join(void *data);
void send_lobby(int status, char *error, int pn, int socket);
void process_lobby(void *data);
void send_game_type(int type);
void process_game_type(void *data);
void send_player_queue(int status, char *error);
void process_player_queue(void *data);
void send_game_ready(int status, char *error);
void process_game_ready(void *data);
void send_player_ready();
void process_player_ready();
void send_game_state(game_state *gs);
void process_game_state(void *data);
void send_player_input(char input);
void process_player_input(void *data);
void send_check_status();
void process_check_status();
void send_game_end(int status, char *error, game_statistics *gs);
void process_game_end(void *data);

/* utilities */
int encode(char *data, size_t datalen, char *buf, size_t buflen);
int decode(char *data, size_t datalen, char *buf, size_t buflen);
char xor_checksum(char *data, size_t len);

/* helpers */
size_t insert_bytes(char *data, size_t datalen, char *buf, size_t buflen, size_t offset);
size_t insert_int32_t(int32_t x, char *buf, size_t buflen, size_t offset);
size_t insert_int64_t(int64_t x, char *buf, size_t buflen, size_t offset);
size_t insert_char(char x, char *buf, size_t buflen, size_t offset);
size_t insert_str(char *str, size_t strlen, char *buf, size_t buflen, size_t offset);
size_t insert_null_bytes(int count, char *buf, size_t buflen, size_t offset);
size_t insert_separator(char *buf, size_t buflen, size_t offset);

/* debug */
void print_packet_bytes(void *packet, size_t len);
void print_bytes(char *start, size_t len);

#endif