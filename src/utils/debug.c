#include "debug.h"

void here() {
    printf("here\n");
}

void printint(char *tag, int x) {
    printf("%s: %d\n", tag, x);
}

void printfloat(char *tag, float x) {
    printf("%s: %f\n", tag, x);
}

void printstr(char *str) {
    printf("%s\n", str);
}

void printstrt(char *tag, char *str) {
    printf("%s: %s\n", tag, str);
}