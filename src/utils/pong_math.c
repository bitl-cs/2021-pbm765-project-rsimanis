#include "pong_math.h"


int mod_i(int x) {
    return (x < 0) ? -x : x;
}

float mod_f(float x) {
    return (x < 0) ? -x : x;
}

float max(float a, float b) {
    return (a > b) ? a : b;
}

float min(float a, float b) {
    return (a < b) ? a : b;
}

int diff_signs_i(int x, int y) {
    return ((x ^ y) < 0);
}

int diff_signs_f(float x, float y) {
    return ((x < 0 && y > 0) || (x > 0 && y < 0));
}

void rev_vec2f(vec2f *v) {
    mult_vec2f(v, -1);
}

float mag_vec2f(vec2f *v) {
    return pow(pow(v->x, 2.0) + pow(v->y, 2.0), 0.5); 
}

void norm_vec2f(vec2f *v) {
    float mag;

    mag = mag_vec2f(v);
    v->x = v->x / mag;
    v->y = v->y / mag;
}

void mult_vec2f(vec2f *v, float a) {
    v->x = v->x * a;
    v->y = v->y * a;
}

void rand_vec2f(vec2f *v, float mag) {
    v->x = rand_f();
    v->y = rand_f();
    norm_vec2f(v);
    mult_vec2f(v, mag);
}

/* random float in between [0, float_max] */
float rand_f_max(float float_min, float float_max) {
    float r = ((float) rand() / (float) RAND_MAX) * float_max; // [0,float_max]
    if (r < float_min)
        return float_min;
    return r;
}

/* random float in between [-1.0, 1.0] */
float rand_f() {
    float r;

    r = (float) rand() / (float) RAND_MAX;
    if (((float) rand() / (float) RAND_MAX) < 0.5)
        r *= -1;
    return r;
}

void print_vec2f(vec2f *v) {
    printf("(%f, %f)\n", v->x, v->y);
}

void add_vec2f(vec2f *v, vec2f *addv) {
    v->x += addv->x;
    v->y += addv->y;
}

float angle_with_horizon_vec2f(vec2f *v) {
    return mod_f(atan(v->y / v->x) * 180 / M_PI);
}

float angle_with_vertical_vec2f(vec2f *v) {
    return 90 - angle_with_horizon_vec2f(v);
}