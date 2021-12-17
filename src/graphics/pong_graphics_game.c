#include "pong_graphics_game.h"

void render_game(){
    /* Currently fake data */
    int pl1x = 30;
    int pl1y = 140;
    int pw = 40;
    int ph = 150;
    int pl2x = 720;
    int pl2y = 540;

    int blx = 370;
    int bly = 280;
    int blr = 15;

    int scr1 = 7;
    int scr2 = 4;

    /* Drawing the fake data */
    render_rectangle(pl1x, pl1y, pw, ph, GAME_PADDLE_COLOR);
    render_rectangle(pl2x, pl2y, pw, ph, GAME_PADDLE_COLOR);
    render_circle(blx, bly, blr, RGB_RED);
    render_int(GAME_SCORE_LEFT_FIELD_X, GAME_SCORE_FIELD_Y, scr1, GAME_SCORE_FIELD_TEXT_COLOR);
    render_int(GAME_SCORE_RIGHT_FIELD_Y, GAME_SCORE_FIELD_Y, scr2, GAME_SCORE_FIELD_TEXT_COLOR);
}