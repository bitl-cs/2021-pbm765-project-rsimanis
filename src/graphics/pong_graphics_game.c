#include "pong_graphics_game.h"
#include "../client/pong_client.h"
#include "pong_graphics.h"

void render_game_loading(){
    char *data = rend_info.data;
    int window_width, window_height;
    char team_count, team_id;
    float team_goal1X, team_goal1Y;
    float team_goal2X, team_goal2Y;
    char player_count;
    char player_id, ready, player_team_id, *name;
    float player_initial_X, player_initial_Y;
    float player_initial_width, player_initial_height;
    char i;

    // window
    window_width = big_endian_to_host_int32_t(*((int32_t *) data));
    // printf("window_width: %d\n", window_width);
    data += 4;
    window_height = big_endian_to_host_int32_t(*((int32_t *) data));
    // printf("window_height: %d\n", window_height);
    data += 4;

    // teams
    team_count = *data;
    // printf("team_count: %d\n", team_count);
    data += 1;
    for (i = 0; i < team_count; i++) {
        team_id = *data;
        // printf("team_id: %d\n", team_id);
        data += 1;
        team_goal1X = big_endian_to_host_float(*((float *) data));
        // printf("team_goal1x: %f\n", team_goal1X);
        data += 4;
        team_goal1Y = big_endian_to_host_float(*((float *) data));
        // printf("team_goal1y: %f\n", team_goal1Y);
        data += 4;
        team_goal2X = big_endian_to_host_float(*((float *) data));
        // printf("team_goal2x: %f\n", team_goal2X);
        data += 4;
        team_goal2Y = big_endian_to_host_float(*((float *) data));
        // printf("team_goal2y: %f\n", team_goal2Y);
        data += 4;
    } 

    // players
    player_count = *data;
    // printf("player_count: %d\n", player_count);
    data += 1;
    for (i = 0; i < player_count; i++) {
        player_id = *data;
        // printf("player_id: %d\n", player_id);
        data += 1;
        ready = *data;
        // printf("player_ready: %d\n", ready);
        data += 1;
        player_team_id = *data;
        // printf("player_team_id: %d\n", player_team_id);
        data += 1;
        name = data;
        // printf("player_name: %s\n", name);
        data += MAX_NAME_LENGTH + 1;
        player_initial_X = big_endian_to_host_float(*((float *) data));
        // printf("player_initial_x: %f\n", player_initial_X);
        data += 4;
        player_initial_Y = big_endian_to_host_float(*((float *) data));
        // printf("player_initial_y: %f\n", player_initial_Y);
        data += 4;
        player_initial_width = big_endian_to_host_float(*((float *) data));
        // printf("player_initial_width: %f\n", player_initial_width);
        data += 4;
        player_initial_height = big_endian_to_host_float(*((float *) data));
        // printf("player_initial_height: %f\n", player_initial_height);
        data += 4;

        render_rectangle(player_initial_X, player_initial_Y, player_initial_width, player_initial_height, GAME_PADDLE_COLOR);
    }
    send_player_ready(rend_info.client_id, rend_info.send_mem);

    /* Currently fake data */
    // int pl1x = 30;
    // int pl1y = 140;
    // int pw = 40;
    // int ph = 150;
    // int pl2x = 720;
    // int pl2y = 540;

    // int blx = 370;
    // int bly = 280;
    // int blr = 15;

    // int scr1 = 7;
    // int scr2 = 4;

    // /* Drawing the fake data */
    // render_rectangle(pl1x, pl1y, pw, ph, GAME_PADDLE_COLOR);
    // render_rectangle(pl2x, pl2y, pw, ph, GAME_PADDLE_COLOR);
    // render_circle(blx, bly, blr, RGB_RED);
    // render_int(GAME_SCORE_LEFT_FIELD_X, GAME_SCORE_FIELD_Y, scr1, GAME_SCORE_FIELD_TEXT_COLOR);
    // render_int(GAME_SCORE_RIGHT_FIELD_Y, GAME_SCORE_FIELD_Y, scr2, GAME_SCORE_FIELD_TEXT_COLOR);
}

void render_game() {
    char *data = rend_info.data;

    // draw game state
    int window_width, window_height;

    char team_count;
    char team_id;
    int team_score;
    float team_goal1X, team_goal1Y;
    float team_goal2X, team_goal2Y;

    char player_count;
    char player_id, player_team_id;
    float player_x, player_y;
    float player_width, player_height;

    char ball_count;
    float ball_x, ball_y;
    float ball_radius;
    char ball_type;

    char power_up_count;
    char power_up_type;
    float power_up_x, power_up_y;
    float power_up_width, power_up_height;
    char i;

    // window
    window_width = big_endian_to_host_int32_t(*((int32_t *) data));
    // printf("window_width: %d\n", window_width);
    data += 4;
    window_height = big_endian_to_host_int32_t(*((int32_t *) data));
    // printf("window_height: %d\n", window_height);
    data += 4;

    // teams
    team_count = *data;
    // printf("team_count: %d\n", team_count);
    data += 1;
    for (i = 0; i < team_count; i++) {
        team_id = *data;
        // printf("team_id: %d\n", team_id);
        data += 1;
        team_score = big_endian_to_host_int32_t(*((int32_t *) data));
        // printf("team_score: %d\n", team_score);
        data += 4;
        team_goal1X = big_endian_to_host_float(*((float *) data));
        // printf("team_goal1x: %f\n", team_goal1X);
        data += 4;
        team_goal1Y = big_endian_to_host_float(*((float *) data));
        // printf("team_goal1y: %f\n", team_goal1Y);
        data += 4;
        team_goal2X = big_endian_to_host_float(*((float *) data));
        // printf("team_goal2x: %f\n", team_goal2X);
        data += 4;
        team_goal2Y = big_endian_to_host_float(*((float *) data));
        // printf("team_goal2y: %f\n", team_goal2Y);
        data += 4;

        /* Rendering two scores (1 for each team) */
        if (i == 0)
            render_int(GAME_SCORE_LEFT_FIELD_X, GAME_SCORE_FIELD_Y, team_score, RGB_WHITE);
        else
            render_int(GAME_SCORE_RIGHT_FIELD_X, GAME_SCORE_FIELD_Y, team_score, RGB_WHITE);
    } 

    // players
    player_count = *data;
    // printf("player_count: %d\n", player_count);
    data += 1;
    for (i = 0; i < player_count; i++) {
        player_id = *data;
        // printf("player_id: %d\n", player_id);
        data += 1;
        player_team_id = *data;
        // printf("player_team_id: %d\n", player_team_id);
        data += 1;
        player_x = big_endian_to_host_float(*((float *) data));
        // printf("player_x: %f\n", player_x);
        data += 4;
        player_y = big_endian_to_host_float(*((float *) data));
        // printf("player_y: %f\n", player_y);
        data += 4;
        player_width = big_endian_to_host_float(*((float *) data));
        // printf("player_width: %f\n", player_width);
        data += 4;
        player_height = big_endian_to_host_float(*((float *) data));
        // printf("player_height: %f\n", player_height);
        data += 4;

        /* Rendering each player */
        render_rectangle(player_x, player_y, player_width, player_height, RGB_WHITE);
    }

    // balls
    ball_count = *data;
    // printf("ball_count: %d\n", ball_count);
    data += 1;
    for (i = 0; i < ball_count; i++) {
        ball_x = big_endian_to_host_float(*((float *) data));
        // printf("ball_x: %f\n", ball_x);
        data += 4;
        ball_y = big_endian_to_host_float(*((float *) data));
        // printf("ball_y: %f\n", ball_y);
        data += 4;
        ball_radius = big_endian_to_host_float(*((float *) data));
        // printf("ball_radius: %f\n", ball_radius);
        data += 4;
        ball_type = *data;
        // printf("ball_type: %d\n", ball_type);
        data += 1;
        /* Rendering each ball */
        render_circle(ball_x, ball_y, ball_radius, RGB_WHITE);
    }

    // power_ups
    power_up_count = *data;
    // printf("power_up_count: %d\n", power_up_count);
    data += 1;
    for (i = 0; i < power_up_count; i++) {
        power_up_type = *data;
        // printf("power_up_type: %d\n", power_up_type);
        data += 1;
        power_up_x = big_endian_to_host_float(*((float *) data));
        // printf("power_up_x: %f\n", power_up_x);
        data += 4;
        power_up_y = big_endian_to_host_float(*((float *) data));
        // printf("power_up_y: %f\n", power_up_y);
        data += 4;
        power_up_width = big_endian_to_host_float(*((float *) data));
        // printf("power_up_width: %f\n", power_up_width);
        data += 4;
        power_up_height = big_endian_to_host_float(*((float *) data));
        // printf("power_up_height: %f\n", power_up_height);
        data += 4;

        render_rectangle(power_up_x, power_up_y, power_up_width, power_up_height, RGB_BLUE);
    }
}