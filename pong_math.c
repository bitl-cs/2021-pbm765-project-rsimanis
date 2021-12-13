#include "pong_math.h"

int mod_i(int x) {
    return (x < 0) ? -x : x;
}

float mod_f(float x) {
    return (x < 0) ? -x : x;
}

int diff_signs_i(int x, int y) {
    return ((x ^ y) < 0);
}

int diff_signs_f(float x, float y) {
    return ((x < 0 && y > 0) || (x > 0 && y < 0));
}

void reverse_vec2f(vec2f *v) {
    v->x = -v->x;
    v->y = -v->y;
}