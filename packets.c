#include "packets.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>


/* packet processing */

/* send generic packet */
/* data must already be in correct endianess */
void send_packet(int32_t pn, int32_t pid, char *data, size_t datalen, size_t packet_data_size, int socket) {
    char packet[PACKET_MAX_SIZE];
    char final_packet[2 * PACKET_MAX_SIZE + PACKET_SEPARATOR_SIZE]; /* assume that all characters in array "packet" could get encoded */ 
    size_t offset = 0;

    /* add packet number */
    offset += insert_int32_t(host_to_network_int32_t(pn), packet, sizeof(packet), offset);
    // print_bytes(packet, offset);

    /* add packet id */
    offset += insert_int32_t(host_to_network_int32_t(pid), packet, sizeof(packet), offset);
    // print_bytes(packet, offset);

    /* add the size of data segment */
    offset += insert_int64_t(host_to_network_int64_t(packet_data_size), packet, sizeof(packet), offset);
    // print_bytes(packet, offset);

    /* add checksum */
    offset += insert_char(xor_checksum(data, datalen), packet, sizeof(packet), offset);
    // print_bytes(packet, offset);

    /* add data */
    offset += insert_bytes(data, datalen, packet, sizeof(packet), offset);
    // print_bytes(packet, offset);

    /* fill the unused data segment with null bytes */
    offset += insert_null_bytes(packet_data_size - offset + PACKET_HEADER_SIZE, packet, sizeof(packet), offset);
    // print_bytes(packet, offset);
    // printf("%lu\n", offset);

    /* encode */
    offset = encode(packet, offset, final_packet, sizeof(final_packet));
    // print_bytes(final_packet, offset);
    // printf("%lu\n", offset);

    /* add separator */
    offset += insert_separator(final_packet, sizeof(final_packet), offset);
    print_bytes(final_packet, offset);
    printf("%lu\n", offset);
    putchar('\n');
    // print_bytes_full(final_packet, offset);
    // putchar('\n');

    /* send packet to socket */ 
    send(socket, final_packet, offset, 0);
}

void send_join(char *name, int32_t pn, int socket) {
    printf("sent join\n");
    /* check if the name exceeds the limit */
    size_t namelen = strlen(name);
    if (namelen > MAX_NAME_SIZE - 1)
        namelen = MAX_NAME_SIZE - 1;

    send_packet(pn, 1, name, namelen, JOIN_PACKET_DATA_SIZE, socket);
}

void process_join(void *data) {
    printf("received join\n");
}

void send_lobby(int status, char *error, int32_t pn, int socket) {
    printf("sent lobby\n");
    char data[LOBBY_PACKET_MAX_DATA_SIZE];
    size_t offset = 0;

    offset += insert_int32_t(host_to_network_int32_t((int32_t) status), data, sizeof(data), offset);
    if (error != NULL && status == 1) {
        /* check if the error exceeds the limit */
        size_t error_len = strlen(error);
        if(error_len > PACKET_MAX_ERROR_MESSAGE_SIZE - 1)
            error_len = PACKET_MAX_ERROR_MESSAGE_SIZE - 1;

        offset += insert_str(error, error_len, data, sizeof(data), offset);
        send_packet(pn, 2, data, offset, LOBBY_PACKET_MAX_DATA_SIZE, socket);
    } 
    else {
        send_packet(pn, 2, data, offset, PACKET_STATUS_SIZE, socket);
    }
}
 
void process_lobby(void *data){
    printf("received lobby\n");
    print_bytes(data, LOBBY_PACKET_MAX_DATA_SIZE);
    putchar('\n');
}

void send_game_type(int type, int32_t pn, int socket){
    printf("sent game_type\n");
    char data[GAME_TYPE_PACKET_DATA_SIZE];
    size_t offset = 0;

    offset += insert_int32_t(host_to_network_int32_t((int32_t) type), data, sizeof(data), offset);
    send_packet(pn, 3, data, offset, GAME_TYPE_PACKET_DATA_SIZE, socket);
}
 
void process_game_type(void *data) {
    printf("received game_type\n");
    print_bytes(data, GAME_TYPE_PACKET_DATA_SIZE);
    putchar('\n');
 
}

void send_player_queue(int status, char *error, int32_t pn, int socket) {
    char data[PLAYER_QUEUE_PACKET_MAX_DATA_SIZE];
    size_t offset = 0;

    offset += insert_int32_t(host_to_network_int32_t((int32_t) status), data, sizeof(data), offset);
    if (error != NULL && status == 1) {
        /* check if the error exceeds the limit */
        size_t error_len = strlen(error);
        if(error_len > PACKET_MAX_ERROR_MESSAGE_SIZE - 1)
            error_len = PACKET_MAX_ERROR_MESSAGE_SIZE - 1;

        offset += insert_str(error, error_len, data, sizeof(data), offset);
        send_packet(pn, 4, data, offset, PLAYER_QUEUE_PACKET_MAX_DATA_SIZE, socket);
    } 
    else {
        send_packet(pn, 4, data, offset, PACKET_STATUS_SIZE, socket);
    }
    printf("sent player_queue\n");

}

void process_player_queue(void *data) {
    printf("received player_queue\n");

}

void send_game_ready(int status, char *error, int32_t pn, int socket) {
    char data[GAME_READY_PACKET_MAX_DATA_SIZE];
    size_t offset = 0;

    offset += insert_int32_t(host_to_network_int32_t((int32_t) status), data, sizeof(data), offset);
    if (error != NULL && status == 1) {
        /* check if the error exceeds the limit */
        size_t error_len = strlen(error);
        if(error_len > PACKET_MAX_ERROR_MESSAGE_SIZE - 1)
            error_len = PACKET_MAX_ERROR_MESSAGE_SIZE - 1;

        offset += insert_str(error, error_len, data, sizeof(data), offset);
        send_packet(pn, 5, data, offset, GAME_READY_PACKET_MAX_DATA_SIZE, socket);
    } 
    else {
        send_packet(pn, 5, data, offset, PACKET_STATUS_SIZE, socket);
    }
    printf("sent game_ready\n");
}

void process_game_ready(void *data) {
    printf("received game_ready\n");

}

void send_player_ready(int32_t pn, int socket) {
    send_packet(pn, 6, NULL, 0, PLAYER_READY_PACKET_DATA_SIZE, socket);
    printf("sent player_ready\n");
}

void process_player_ready() {
    printf("received player_ready\n");

}

void send_game_state(void *game_state, int32_t pn, int socket) {
    char data[GAME_STATE_PACKET_MAX_DATA_SIZE];
    size_t offset = 0;

    /* fix endianess */

    offset += insert_bytes(game_state, GAMEBOARD_STATE_SIZE, data, sizeof(data), offset);
    send_packet(pn, 7, data, offset, GAME_STATE_PACKET_MAX_DATA_SIZE, socket);
    printf("sent game_state\n");
}

void process_game_state(void *data) {
    printf("received game_state\n");

}

void send_player_input(char input, int pn, int socket) {
    char data[PLAYER_INPUT_PACKET_DATA_SIZE];
    size_t offset = 0;

    offset += insert_char(input, data, sizeof(data), offset);
    send_packet(pn, 8, data, offset, PLAYER_INPUT_PACKET_DATA_SIZE, socket);
    printf("sent player_input\n");
}

void process_player_input(void *data) {
    printf("received player_input\n");

}

void send_check_status(int pn, int socket) {
    send_packet(pn, 9, NULL, 0, CHECK_STATUS_PACKET_DATA_SIZE, socket);
    printf("sent check_status\n");
}

void process_check_status() {
    printf("received check_status\n");

}

/* UPDATE: takes pointer to char arrays containing name */
/* game statistics data follows immediatelly after game_statistics pointer in the same order as it is defined in API doc */
void send_game_end(int status, char *error, void *game_statistics, int pn, int socket) {
    char data[GAME_END_PACKET_MAX_DATA_SIZE];
    size_t offset = 0;

    offset += insert_int32_t(host_to_network_int32_t((int32_t) status), data, sizeof(data), offset);
    if (error != NULL && status == 1) {
        /* check if the error exceeds the limit */
        size_t error_len = strlen(error);
        if(error_len > PACKET_MAX_ERROR_MESSAGE_SIZE - 1)
            error_len = PACKET_MAX_ERROR_MESSAGE_SIZE - 1;

        offset += insert_str(error, error_len, data, sizeof(data), offset);
        send_packet(pn, 10, data, offset, GAME_END_PACKET_MAX_DATA_SIZE, socket);
    } 
    else {
        offset += insert_bytes(game_statistics, GAME_STATISTICS_SIZE, data, sizeof(data), offset);
        send_packet(pn, 10, data, offset, PACKET_STATUS_SIZE, socket);
    }
    printf("sent game_end\n");
}

void process_game_end(void *data) {
    printf("received game_end\n");

}

/* utilities */
int encode(char *data, size_t datalen, char *buf, size_t buflen) {
    size_t i, buf_i;

    buf_i = 0;
    for (i = 0; i < datalen && buf_i < buflen; i++) {
        if (data[i] == '-') {
            buf[buf_i++] = '?';
            if (buf_i < buflen)
                buf[buf_i++] = '-';
            else
                break;
        }
        else if (data[i] == '?') {
            buf[buf_i++] = '?';
            if (buf_i < buflen)
                buf[buf_i++] = '*';
            else
                break;
        }
        else
            buf[buf_i++] = data[i];
    }
    return buf_i;
}

int decode(char *data, size_t datalen, char *buf, size_t buflen) {
    size_t i, buf_i;
    char next;

    buf_i = 0;
    for (i = 0; i < datalen && buf_i < buflen; i++) {
        if (data[i] == '?') {
            if (i < datalen - 1) {
                next = data[++i];
                if (next == '-')
                    buf[buf_i++] = '-';
                else if (next == '*')
                    buf[buf_i++] = '?';
                else 
                    break;
            }
            else
                break;
        }
        else
            buf[buf_i++] = data[i];
    }
    return buf_i;
}

char xor_checksum(char *data, size_t len) {
    char rez;
    size_t i;

    if (len == 0)
        return 0;

    rez = data[0];
    for (i = 1; i < len; i++)
        rez ^= data[i];
    return rez;
}

int verify_packet(char *packet, int *current_pn, long decoded_size) {
    int32_t *recv_pn = (int32_t *) packet;
    int64_t *recv_data_size = (int64_t *) (packet + PACKET_NUMBER_SIZE + PACKET_ID_SIZE);
    char *recv_checksum = packet + PACKET_NUMBER_SIZE + PACKET_ID_SIZE + PACKET_SIZE_SIZE;
    char *data = packet + PACKET_HEADER_SIZE;

    // printf("recv_pn: %d\n", *recv_pn);
    // printf("current_pn: %d\n", *current_pn);
    if (*recv_pn < *current_pn)
        return 0;

    if (*recv_data_size != decoded_size - PACKET_HEADER_SIZE)
        return 0;

    if (*recv_checksum != xor_checksum(data, *recv_data_size))
        return 0;

    return 1;
}

/* debug */
void print_bytes(char *start, size_t len) {
    size_t i;

    for (i = 0; i < len; i++)
        printf("%02hhx ", start[i]);
    putchar('\n');
}

void print_bytes_full(char *start, size_t len) {
    size_t i;

    if (len > 999) {
        printf("Cannot print more than 999 bytes! You asked for %lu\n", len);
        return;
    }

    printf("Printing %lu bytes...\n", len);
    printf("[NPK] [C] [HEX] [DEC] [ BINARY ]\n");
    printf("================================\n");
    for (i = 0; i < len; i++) {
        printf(" %3lu | %c | %02X | %3d | %c%c%c%c%c%c%c%c\n", i, printable_char(start[i]), start[i], start[i],
            start[i] & 0x80 ? '1' : '0',
            start[i] & 0x40 ? '1' : '0',
            start[i] & 0x20 ? '1' : '0',
            start[i] & 0x10 ? '1' : '0',
            start[i] & 0x08 ? '1' : '0',
            start[i] & 0x04 ? '1' : '0',
            start[i] & 0x02 ? '1' : '0',
            start[i] & 0x01 ? '1' : '0'
        );
    }
}


/* helpers */
size_t insert_bytes(char *data, size_t datalen, char *buf, size_t buflen, size_t offset) {
    size_t i;
    for (i = 0; i < datalen && i + offset < buflen; i++)
        buf[i + offset] = data[i];
    return i;
}

size_t insert_int32_t(int32_t x, char *buf, size_t buflen, size_t offset) {
    return insert_bytes((char *) &x, sizeof(int32_t), buf, buflen, offset);
}

size_t insert_int64_t(int64_t x, char *buf, size_t buflen, size_t offset) {
    return insert_bytes((char *) &x, sizeof(int64_t), buf, buflen, offset);
}

size_t insert_char(char x, char *buf, size_t buflen, size_t offset) {
    return insert_bytes(&x, sizeof(char), buf, buflen, offset);
}

size_t insert_str(char *str, size_t strlen, char *buf, size_t buflen, size_t offset) {
    size_t rez;

    rez = insert_bytes(str, strlen, buf, buflen - 1, offset);
    rez += insert_null_bytes(1, buf, buflen, offset + rez);
    return rez;
}

size_t insert_null_bytes(int count, char *buf, size_t buflen, size_t offset) {
    int i;
    size_t rez = 0;

    for (i = 0; i < count && i + offset < buflen; i++) {
        buf[i + offset] = '\0';
        rez++;
    }
    return rez;
}

size_t insert_separator(char *buf, size_t buflen, size_t offset) {
    return insert_bytes((char *) PACKET_SEPARATOR, PACKET_SEPARATOR_SIZE, buf, buflen, offset);
}


int is_little_endian_system() {
    volatile uint32_t i = 0x01234567;
    return (*((uint8_t *) (&i))) == 0x67;
}

/* if needed reverses "len" bytes starting at "start", so that the order is BIG ENDIAN */
void host_to_network_bytes(char *start, size_t len) {
    size_t i;
    char temp;

    /* if already running on big endian machine */
    if (!is_little_endian_system())
        return;

    /* reverse byte order */
    for (i = 0; i < len / 2; i++) {
        temp = start[i];
        start[i] = start[len - i - 1];
        start[len - i - 1] = temp;
    }
}

short host_to_network_short(short x) {
    host_to_network_bytes((char *) &x, sizeof(short));
    return x;
}

int host_to_network_int(int x) {
    host_to_network_bytes((char *) &x, sizeof(int));
    return x;
}

long host_to_network_long(long x) {
    host_to_network_bytes((char *) &x, sizeof(long));
    return x;
}

float host_to_network_float(float x) {
    host_to_network_bytes((char *) &x, sizeof(float));
    return x;
}

double host_to_network_double(double x) {
    host_to_network_bytes((char *) &x, sizeof(double));
    return x;
}

int32_t host_to_network_int32_t(int32_t x) {
    host_to_network_bytes((char *) &x, sizeof(int32_t));
    return x;
}

int64_t host_to_network_int64_t(int64_t x) {
    host_to_network_bytes((char *) &x, sizeof(int64_t));
    return x;
}

/* if needed reverses "len" bytes starting at "start", so that the order is LITTLE ENDIAN */
void network_to_host_bytes(char *start, size_t len) {
    size_t i;
    char temp;

    /* if already running on big endian machine */
    if (!is_little_endian_system())
        return;

    /* reverse byte order */
    for (i = 0; i < len / 2; i++) {
        temp = start[i];
        start[i] = start[len - i - 1];
        start[len - i - 1] = temp;
    }
}

short network_to_host_short(short x) {
    network_to_host_bytes((char *) &x, sizeof(short));
    return x;
}

int network_to_host_int(int x) {
    network_to_host_bytes((char *) &x, sizeof(int));
    return x;
}

long network_to_host_long(long x) {
    network_to_host_bytes((char *) &x, sizeof(long));
    return x;
}

float network_to_host_float(float x) {
    network_to_host_bytes((char *) &x, sizeof(float));
    return x;
}

double network_to_host_double(double x) {
    network_to_host_bytes((char *) &x, sizeof(double));
    return x;
}

int32_t network_to_host_int32_t(int32_t x) {
    network_to_host_bytes((char *) &x, sizeof(int32_t));
    return x;
}

int64_t network_to_host_int64_t(int64_t x) {
    network_to_host_bytes((char *) &x, sizeof(int64_t));
    return x;
}


char printable_char(char c) {
    if (isprint(c) != 0)
        return c;
    return ' ';
}