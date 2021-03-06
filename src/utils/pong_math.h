#ifndef _PONG_MATH_H
#define _PONG_MATH_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct _vec2i {
    int x;
    int y;
} vec2i;

typedef struct _vec2f {
    float x;
    float y;
} vec2f;

int mod_i(int x);
float mod_f(float x);
float max(float a, float b);
float min(float a, float b);
float rand_f();
float rand_f_max(float float_min, float float_max);

int diff_signs_i(int x, int y);
int diff_signs_f(float x, float y);

void add_vec2f(vec2f *v, vec2f *addv);
void rev_vec2f(vec2f *v);
float mag_vec2f(vec2f *v);
void mult_vec2f(vec2f *v, float a);
void rand_vec2f(vec2f *v, float mag);
void print_vec2f(vec2f *v);
float angle_with_horizon_vec2f(vec2f *v);
float angle_with_vertical_vec2f(vec2f *v);

#endif