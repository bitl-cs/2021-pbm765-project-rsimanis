#include "packets.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>


/* packet processing */
void send_join(char *name, int pn, int socket) {
    char packet[JOIN_PACKET_SIZE];
    char final_packet[2 * JOIN_PACKET_SIZE + PACKET_SEPARATOR_SIZE]; /* assume that all characters in array "packet" could get encoded */ 
    size_t offset = 0;

    /* add packet number */
    offset += insert_int32_t(pn, packet, sizeof(packet), offset);
    print_bytes(packet, offset);

    /* add packet id */
    int32_t pid = 1;
    offset += insert_int32_t(pid, packet, sizeof(packet), offset);
    print_bytes(packet, offset);

    /* check if the name exceeds the limit */
    size_t name_len = strlen(name);
    if (name_len > MAX_NAME_SIZE)
        name_len = MAX_NAME_SIZE;

    /* add the size of data segment */
    offset += insert_int64_t(JOIN_PACKET_DATA_SIZE, packet, sizeof(packet), offset);
    print_bytes(packet, offset);

    /* skip checksum for now */
    offset += PACKET_CHECKSUM_SIZE; 

    /* add data */
    offset += insert_str(name, name_len, packet, sizeof(packet), offset);
    print_bytes(packet, offset);

    /* add checksum */
    insert_char(xor_checksum((char *) &(packet[PACKET_HEADER_SIZE]), name_len), packet, sizeof(packet), PACKET_HEADER_SIZE - PACKET_CHECKSUM_SIZE);
    print_bytes(packet, offset);

    /* fill the unused data segment with null bytes */
    offset += insert_null_bytes(sizeof(packet) - name_len, packet, sizeof(packet), offset);
    print_bytes(packet, offset);
    printf("%lu\n", offset);

    /* encode */
    offset = encode(packet, sizeof(packet), final_packet, sizeof(final_packet));
    print_bytes(final_packet, offset);
    printf("%lu\n", offset);

    /* add separator */
    offset += insert_separator(final_packet, sizeof(final_packet), offset);
    print_bytes(final_packet, offset);
    printf("%lu\n", offset);

    /* send packet to socket */ 
    // send(socket, final_packet, offset, 0);
}

void process_join(void *data) {

}


/* utilities */
int encode(char *data, size_t datalen, char *buf, size_t buflen) {
    size_t i, buf_i;

    buf_i = 0;
    for (i = 0; i < datalen; i++) {
        if (buf_i >= buflen)
            return -1;
        if (data[i] == '-') {
            buf[buf_i++] = '?';
            buf[buf_i++] = '-';
        }
        else if (data[i] == '?') {
            buf[buf_i++] = '?';
            buf[buf_i++] = '*';
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
    for (i = 0; i < datalen; i++) {
        if (buf_i >= buflen)
            return -1;
        if (data[i] == '?') {
            if (i < datalen - 1) {
                next = data[++i];
                if (next == '-')
                    buf[buf_i++] = '-';
                else if (next == '*')
                    buf[buf_i++] = '?';
                else
                    return -2;
            }
            else
                return -2;
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


/* debug */
void print_packet_bytes(void *packet, size_t len) {

}

void print_bytes(char *start, size_t len) {
    size_t i;
    for (i = 0; i < len; i++)
        printf("%02hhx ", start[i]);
    putchar('\n');
}