// #include "pong_networking.h"
// #include "pong_server.h"
// #include "pong_networking.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

#define PACKET_SEPARATOR "--"

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

int is_little_endian_system() {
    volatile uint32_t i = 0x01234567;
    return (*((uint8_t *) (&i))) == 0x67;
}
size_t insert_bytes_as_big_endian(char *data, size_t datalen, char *buf, size_t buflen, size_t offset) {
    size_t i;

    if (is_little_endian_system()) {
        /* reverse */
        for (i = 0; i < datalen && i + offset < buflen; i++)
            buf[i + offset] = data[datalen - i - 1];
    }
    else {
        /* store as it is */
        for (i = 0; i < datalen && i + offset < buflen; i++)
            buf[i + offset] = data[i];
    }
    return i;
}

size_t insert_int32_t_as_big_endian(int32_t x, char *buf, size_t buflen, size_t offset) {
    return insert_bytes_as_big_endian((char *) &x, sizeof(x), buf, buflen, offset);
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

    // mnode *head;
    // init_list("pirmais", &head);
    // print_list(head);
    // putchar('\n');
    // push_back("otrais", &head);
    // print_list(head);
    // putchar('\n');
    // pop_front(&head);
    // print_list(head);
    // putchar('\n');
    // push_back("tresais", &head);
    // print_list(head);
    // putchar('\n');
    // push_back("ceturtais", &head);
    // print_list(head);
    // putchar('\n');
    // push_back("piektais", &head);
    // print_list(head);
    // putchar('\n');
    // pop_front(&head);
    // print_list(head);
    // putchar('\n');
    // pop_front(&head);
    // print_list(head);
    // putchar('\n');
    // free_list(&head);
    return 0;
}

