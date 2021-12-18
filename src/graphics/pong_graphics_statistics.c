#include "pong_graphics_statistics.h"
#include "../networking/pong_networking.h"

void render_statistics() {
    char *data;

    char status;
    char i;
    int playerTeamScore, gameDuration;

    char team_count;
    char id;
    int score;
    
    char player_count;
    char player_id, team_id;
    int player_score;
    char *name;

    data = rend_info.data;
    status = *data;
    if (status == PACKET_GAME_END_STATUS_ERROR) {
        render_string(STATISTICS_ERROR_MSG_X, STATISTICS_ERROR_MSG_Y, "Game ended with error (check chat for more info)", RGB_WHITE);
    }
    // printf("status: %d\n", status);
    data += 1;
    playerTeamScore = big_endian_to_host_int32_t(*((int32_t *) data));
    // printf("playerTeamScore: %d\n", playerTeamScore);
    data += 4;
    gameDuration = big_endian_to_host_int32_t(*((int32_t *) data));
    // printf("gameDuration: %d\n", gameDuration);
    data += 4;

    /* Player team info and game time info */
    render_int_with_tag(STATISTICS_GENERAL_X, STATISTICS_GENERAL_INITIAL_Y, "Your team score", playerTeamScore, RGB_WHITE);
    render_int_with_tag(STATISTICS_GENERAL_X, STATISTICS_GENERAL_INITIAL_Y + 1 * STATISTICS_LINE_SPACING, 
                            "Game Duration", gameDuration, RGB_WHITE);
    team_count = *data;
    // printf("team_count: %d\n", team_count);
    data += 1;
    if (status != PACKET_GAME_END_STATUS_ERROR) {
        id = *data;
        // printf("team_id: %d\n", id);
        data += 1;
        score = big_endian_to_host_int32_t(*((int32_t *) data));
        // printf("team_score: %d\n", score);
        data += 4;

        render_string(STATISTICS_LEFT_TEAM_X, STATISTICS_LEFT_TEAM_INITIAL_Y, "Left team", STATISTICS_LEFT_TEAM_COLOR);
        render_int_with_tag(STATISTICS_LEFT_TEAM_X, STATISTICS_LEFT_TEAM_INITIAL_Y + 1 * STATISTICS_LINE_SPACING, 
                    "Score", score, STATISTICS_LEFT_TEAM_COLOR);

        id = *data;
        // printf("team_id: %d\n", id);
        data += 1;
        score = big_endian_to_host_int32_t(*((int32_t *) data));
        // printf("team_score: %d\n", score);
        data += 4;
    
        render_string(STATISTICS_RIGHT_TEAM_X, STATISTICS_RIGHT_TEAM_INITIAL_Y, "Right team", STATISTICS_RIGHT_TEAM_COLOR);
        render_int_with_tag(STATISTICS_RIGHT_TEAM_X, STATISTICS_RIGHT_TEAM_INITIAL_Y + 1 * STATISTICS_LINE_SPACING, 
                "Score", score, STATISTICS_RIGHT_TEAM_COLOR);
    }

    player_count = *data;
    // printf("player_count: %d\n", player_count);
    data += 1;
    
    int left_i = 2, right_i = 2;
    if (status != PACKET_GAME_END_STATUS_ERROR) {
        for (i = 0; i < player_count; i++) {
            player_id = *data;
            // printf("player_id: %d\n", player_id);
            data += 1;
            team_id = *data;
            // printf("team_id: %d\n", team_id);
            data += 1;
            score = big_endian_to_host_int32_t(*((int32_t *) data));
            // printf("player_score: %d\n", score);
            data += 4;
            name = data;
            // printf("player_name: %s\n", name);
            data += MAX_NAME_LENGTH + 1;
            /* Rendering info about each player */

            if(team_id == 0){
                render_int_with_tag(STATISTICS_LEFT_TEAM_X, STATISTICS_LEFT_TEAM_INITIAL_Y + (left_i++) * STATISTICS_LINE_SPACING, 
                                    name, score, STATISTICS_LEFT_TEAM_COLOR);
            } else {
                render_int_with_tag(STATISTICS_RIGHT_TEAM_X, STATISTICS_RIGHT_TEAM_INITIAL_Y + (right_i++) * STATISTICS_LINE_SPACING, 
                                    name, score, STATISTICS_RIGHT_TEAM_COLOR);
            }
        }
    }
    render_quit_button(); 
}


void statistics_button_listener(int button, int event, int x, int y){
    if(button == GLUT_LEFT_BUTTON && event == GLUT_DOWN){
        if(button_pressed(QUIT_BUTTON_X, QUIT_BUTTON_Y, QUIT_BUTTON_WIDTH, QUIT_BUTTON_HEIGHT, x, y)){
            printf("QUIT_BUTTON_CLICKED!\n");
            /* Setting "quit" input as pressed */
            rend_info.keys[2] = 1;
        }
    }
}