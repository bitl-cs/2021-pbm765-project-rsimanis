#include "pong_graphics_statistics.h"
#include "../networking/pong_networking.h"

void render_statistics() {
    // quit button
    glutMouseFunc(statistics_button_listener);
    render_quit_button(); 

    char status = PACKET_GAME_END_STATUS_SUCCESS;
    if (status == PACKET_GAME_END_STATUS_ERROR) {
        render_string(STATISTICS_ERROR_MSG_X, STATISTICS_ERROR_MSG_Y, "Game ended with error (check chat for more info)", RGB_WHITE);
    }
    else {
        int player_team_score = 11;
        int game_duration = 145;
        char team_score_1 = 4;
        char team_score_2 = 11;
        char player_score_1 = 4;
        char player_score_2 = 11;
        char *player_name_1 = "Vards1";
        char *player_name_2 = "Vards2";

        render_int_with_tag(STATISTICS_GENERAL_X, STATISTICS_GENERAL_INITIAL_Y, "Your team score", player_team_score, RGB_WHITE);
        render_int_with_tag(STATISTICS_GENERAL_X, STATISTICS_GENERAL_INITIAL_Y + 1 * STATISTICS_LINE_SPACING, 
                            "Game Duration", game_duration, RGB_WHITE);

        // draw left team
        render_string(STATISTICS_LEFT_TEAM_X, STATISTICS_LEFT_TEAM_INITIAL_Y, "Left team", STATISTICS_LEFT_TEAM_COLOR);
        render_int_with_tag(STATISTICS_LEFT_TEAM_X, STATISTICS_LEFT_TEAM_INITIAL_Y + 1 * STATISTICS_LINE_SPACING, 
                        "Score", team_score_1, STATISTICS_LEFT_TEAM_COLOR);
        render_int_with_tag(STATISTICS_LEFT_TEAM_X, STATISTICS_LEFT_TEAM_INITIAL_Y + 2 * STATISTICS_LINE_SPACING, 
                        player_name_1, player_score_1, STATISTICS_LEFT_TEAM_COLOR);

        // draw right team
        render_string(STATISTICS_RIGHT_TEAM_X, STATISTICS_RIGHT_TEAM_INITIAL_Y, "Right team", STATISTICS_RIGHT_TEAM_COLOR);
        render_int_with_tag(STATISTICS_RIGHT_TEAM_X, STATISTICS_RIGHT_TEAM_INITIAL_Y + 1 * STATISTICS_LINE_SPACING, 
                        "Score", team_score_2, STATISTICS_RIGHT_TEAM_COLOR);
        render_int_with_tag(STATISTICS_RIGHT_TEAM_X, STATISTICS_RIGHT_TEAM_INITIAL_Y + 2 * STATISTICS_LINE_SPACING, 
                        player_name_2, player_score_2, STATISTICS_RIGHT_TEAM_COLOR);
    }
}


void statistics_button_listener(int button, int event, int x, int y){
    if(button == GLUT_LEFT_BUTTON && event == GLUT_DOWN){
        if(button_pressed(QUIT_BUTTON_X, QUIT_BUTTON_Y, QUIT_BUTTON_WIDTH, QUIT_BUTTON_HEIGHT, x, y)){
            printf("QUIT_BUTTON_CLICKED!\n");
            /* quitGame(); */
                // close(client_socket);
                // draw join state
        }
    }
}