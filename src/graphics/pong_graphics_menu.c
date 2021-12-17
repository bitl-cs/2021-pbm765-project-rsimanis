#include "pong_graphics_menu.h"

void render_game_menu(){
    /* Adding Buttons */
    glutMouseFunc(menu_button_listener); // this must be moved to client, where the drawing states are changed
    render_button(ONE_VS_ONE_BUTTON_X, ONE_VS_ONE_BUTTON_Y, GAME_TYPE_BUTON_WIDTH, GAME_TYPE_BUTON_HEIGHT, ONE_VS_ONE_BUTTON_TEXT);
    render_button(TWO_VS_TWO_BUTTON_X, TWO_VS_TWO_BUTTON_Y, GAME_TYPE_BUTON_WIDTH, GAME_TYPE_BUTON_HEIGHT, TWO_VS_TWO_BUTTON_TEXT);
    render_quit_button();
}

void menu_button_listener(int button, int event, int x, int y){
    if(button == GLUT_LEFT_BUTTON && event == GLUT_DOWN){
        if(button_pressed(QUIT_BUTTON_X, QUIT_BUTTON_Y, QUIT_BUTTON_WIDTH, QUIT_BUTTON_HEIGHT, x, y)){
            printf("QUIT_BUTTON_CLICKED!\n");
            /* quitGame(); */
                // close(client_socket);
                // draw join state
        }
        if(button_pressed(ONE_VS_ONE_BUTTON_X, ONE_VS_ONE_BUTTON_Y, GAME_TYPE_BUTON_WIDTH, GAME_TYPE_BUTON_HEIGHT, x, y)){
            printf("CLicked 1V1!\n");
            /* Send game type packet to server (1v1) */
        } else if(button_pressed(TWO_VS_TWO_BUTTON_X, TWO_VS_TWO_BUTTON_Y, GAME_TYPE_BUTON_WIDTH, GAME_TYPE_BUTON_HEIGHT, x, y)){
            printf("Clicked 2V2!\n");
            /* Send game type packet to server (2v2) */
        }
            
        
    }
}

