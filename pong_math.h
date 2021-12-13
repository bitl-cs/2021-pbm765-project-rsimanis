#ifndef _PONG_MATH_H
#define _PONG_MATH_H

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

int diff_signs_i(int x, int y);
int diff_signs_f(float x, float y);

void reverse_vec2f(vec2f *v);

#endif