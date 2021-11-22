#ifndef _PACKETS_H
#define _PACKETS_H

#include <inttypes.h>
#include <stddef.h>
#include <limits.h>

#define KEYBOARD_INPUT_SIZE                 1
#define MAX_CLIENT_PACKET_DATA_SIZE         20
#define MAX_GAME_STATE_PACKET_DATA_SIZE     176
#define MAX_SERVER_PACKET_DATA_SIZE         MAX_GAME_STATE_PACKET_DATA_SIZE
#define MAX_USERNAME_SIZE                   20
#define PACKET_HEADER_SIZE                  sizeof(PacketHeader)
#define PACKET_ID_SIZE                      4
#define PACKET_SEPARATOR_SIZE               2

typedef struct _packet_header {
    int32_t packet_number;
    int32_t packet_id; 
    int64_t packet_size; 
    char checksum;
} PacketHeader;

int encode(char *data, char *buf, size_t len, size_t buflen);
int decode(char *data, char *buf, size_t len, size_t buflen);
char xor_checksum(char *data, size_t len);

/* debug */
void print_packet_bytes(void *packet, size_t len);

#endif