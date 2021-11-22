#include "packets.h"
#include <string.h>
#include <stdio.h>

typedef unsigned char byte;

int main() {
    PacketHeader hdr;
    hdr.packet_number = 1;
    hdr.packet_id = 1;
    hdr.packet_size = 12;
    char *data = "Mr. Rozkalns";
    hdr.checksum = xor_checksum(data, strlen(data));
    byte packet[100];

    byte *hdr_ptr = (byte *) &hdr;
    for (size_t i = 0; i < sizeof(hdr); i++) {
        packet[i] = hdr_ptr[i];
    }

    for (size_t i = 0; i < strlen(data); i++) {
        packet[sizeof(hdr) + i] = data[i];
    }

    for (size_t i = 0; i < sizeof(hdr) + strlen(data); i++) {
        printf("%02x", packet[i]);
    }
    putchar('\n');

    return 0;
}