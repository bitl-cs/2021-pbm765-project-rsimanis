#include "packets.h"
#include <stdio.h>

int encode(char *data, char *buf, size_t len, size_t buflen) {
    size_t i, buf_i;

    buf_i = 0;
    for (i = 0; i < len; i++) {
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

int decode(char *data, char *buf, size_t len, size_t buflen) {
    size_t i, buf_i;
    char next;

    buf_i = 0;
    for (i = 0; i < len; i++) {
        if (buf_i >= buflen)
            return -1;
        if (data[i] == '?') {
            if (i < len - 1) {
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
        return -1;

    rez = data[0];
    for (i = 1; i < len; i++)
        rez ^= data[i];
    return rez;
}

/* debug */
void print_packet_bytes(void *packet, size_t len) {

}