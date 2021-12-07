#include "pong_networking.h"
#include "pong_server.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

void show_long(long x) {
    show_bytes((byte_pointer) &x, sizeof(long)); //line:data:show_bytes_amp1
}

void print_arr(char *start, size_t len) {
    size_t i;

    for (i = 0; i < len; i++) {
        printf("%c ", start[i]);
    }
    putchar('\n');
}

struct test_str {
    char taken;
};

void update(struct test_str *ts) {
    ts->taken = '2';
}

int main() {
    // send_join("abcd", 2, 0);
    // send_join("????????????????????????????", 2, 0);
    // char name[] = "Mr. Rozkalns";
    // send_join(name, 2, 0);
    // send_join("Mr. Rozkalnsasdfjasjdfkljasdflkjadfj", 2, 0);
    // char name2[] = "Mr. Rozkalnsasdfasdfasdfasdfasdfa";
    // send_join(name2, 2, 0);

    // send_join("--?--", 2, 0);
    // send_join("", 2, 0)char *name
    // send_join("a", 2, 0);
    // send_join("ab", 2, 0);

    // send_lobby(0, NULL, 2, 0);
    // send_lobby(0, "adsfasdf", 2, 0);
    // send_lobby(1, "adsfasdf", 2, 0);
    // send_lobby(1, "", 2, 0);
    // send_lobby(1, "asd-asdf?-asdf--", 2, 0);
    // send_lobby(1, "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678", 2, 0);
    // send_lobby(1, "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", 2, 0);
    // send_lobby(1, "01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 2, 0);
    // send_lobby(1, "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678912345690", 2, 0);
    // send_lobby(1, "?", 4 , 0);
    // send_lobby(1, "??????????", 8, 0);
    // send_lobby(1, "abcd", 2, 0);

    // send_player_ready(1, 0);

    char buf[10];
    memset(buf, 0, 10);
    print_bytes(buf, 10);
    insert_separator(buf, 10, 0);
    print_bytes(buf, 10);



    return 0;
}

