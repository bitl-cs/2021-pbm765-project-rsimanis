#include "pong_graphics_menu.h"
#include "../client/pong_client.h"

void render_menu(){
    /* Adding menu title */
    render_string_in_the_middle(MENU_TITLE_Y, MENU_TITLE_TEXT, RGB_WHITE);

    /* Adding game type buttons */
    render_button(ONE_VS_ONE_BUTTON_X, ONE_VS_ONE_BUTTON_Y, GAME_TYPE_BUTON_WIDTH, GAME_TYPE_BUTON_HEIGHT, ONE_VS_ONE_BUTTON_TEXT);
    render_button(TWO_VS_TWO_BUTTON_X, TWO_VS_TWO_BUTTON_Y, GAME_TYPE_BUTON_WIDTH, GAME_TYPE_BUTON_HEIGHT, TWO_VS_TWO_BUTTON_TEXT);

    /* Adding quit button */
    render_quit_button();
}

void menu_button_listener(int button, int event, int x, int y){
    if (button == GLUT_LEFT_BUTTON && event == GLUT_DOWN) {
        if (button_pressed(QUIT_BUTTON_X, QUIT_BUTTON_Y, QUIT_BUTTON_WIDTH, QUIT_BUTTON_HEIGHT, x, y)) {
            printf("QUIT_BUTTON_CLICKED!\n");
            /* Setting "quit" input as pressed */
            inp_data.keys[2] = 1;
        }
        if (button_pressed(ONE_VS_ONE_BUTTON_X, ONE_VS_ONE_BUTTON_Y, GAME_TYPE_BUTON_WIDTH, GAME_TYPE_BUTON_HEIGHT, x, y)) {
            printf("CLicked 1V1!\n");
            /* Send game type packet to server (1v1) */
            send_game_type(PACKET_GAME_TYPE_TYPE_1V1, rend_data.send_mem);
        } 
        else if (button_pressed(TWO_VS_TWO_BUTTON_X, TWO_VS_TWO_BUTTON_Y, GAME_TYPE_BUTON_WIDTH, GAME_TYPE_BUTON_HEIGHT, x, y)) {
            printf("Clicked 2V2!\n");
            /* Send game type packet to server (2v2) */
            send_game_type(PACKET_GAME_TYPE_TYPE_2V2, rend_data.send_mem);
        }
    }
}

