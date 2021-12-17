#ifndef _PONG_GRAPHICS_GAME_H
#define _PONG_GRAPHICS_GAME_H

#include "pong_graphics_general.h"

/* Score Field */
#define GAME_SCORE_FIELD_Y                      50
#define GAME_SCORE_LEFT_FIELD_X                 (WINDOW_WIDTH / 4)
#define GAME_SCORE_RIGHT_FIELD_Y                (WINDOW_WIDTH - GAME_SCORE_LEFT_FIELD_X)
#define GAME_SCORE_FIELD_TEXT_COLOR             RGB_WHITE

/* Paddles */
#define GAME_PADDLE_COLOR                       RGB_WHITE

void render_game();

#endif
