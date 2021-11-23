#include "packets.h"
#include <string.h>
#include <stdio.h>

typedef unsigned char *byte_pointer;

void here() {
    printf("here\n");
}

void show_bytes(byte_pointer start, size_t len) {
    size_t i;
    for (i = 0; i < len; i++)
        printf(" %02hhx", start[i]);    //line:data:show_bytes_printf
    printf("\n");
}

void show_int(int x) {
    show_bytes((byte_pointer) &x, sizeof(int)); //line:data:show_bytes_amp1
}

int main() {
    send_join("abcd", 2, 0);
    send_join("Mr. ?Rozkalns-", 2, 0);
    char name[] = "Mr. Rozkalns";
    send_join(name, 2, 0);
    send_join("Mr. Rozkalnsasdfjasjdfkljasdflkjadfj", 2, 0);
    char name2[] = "Mr. Rozkalnsasdfasdfasdfasdfasdfa";
    send_join(name2, 2, 0);

    send_join("--?--", 2, 0);
    send_join("", 2, 0);
    send_join("a", 2, 0);
    send_join("ab", 2, 0);
    return 0;
}