#include "pong_graphics_join.h"

extern render_info rend_info;

void render_join(){ // no pointer to data here needed
    /* Play Button Data */
    int space_between_components = 5;
    /* User Input Frame Data */
    int text_frame_width = 160;
    int text_frame_height = 40;
    int text_frame_x = (WINDOW_WIDTH - text_frame_width) / 2;
    int text_frame_y = ((WINDOW_HEIGHT - text_frame_height) / 2) - space_between_components - text_frame_height;

    glutMouseFunc(join_button_listener);
    /* Adding Join Button */
    render_button(JOIN_BUTTON_X, JOIN_BUTTON_Y, JOIN_BUTTON_WIDTH, JOIN_BUTTON_HEIGHT, JOIN_BUTTON_TEXT);

    /*Adding Text Frame */
    render_outline(text_frame_x, text_frame_y, text_frame_width, text_frame_height, RGB_YELLOW);

    render_string(text_frame_x + text_frame_width / 20, text_frame_y + text_frame_height * 2/ 3, rend_info.input_buf, RGB_WHITE);
}

void join_button_listener(int button, int event, int x, int y){
    if(button == GLUT_LEFT_BUTTON && event == GLUT_DOWN){
        if(button_pressed(QUIT_BUTTON_X, QUIT_BUTTON_Y, QUIT_BUTTON_WIDTH, QUIT_BUTTON_HEIGHT, x, y)){
            printf("QUIT_BUTTON_CLICKED!\n");
            /* quitGame(); */
                // close(client_socket);
                // draw join state
        }
        if(button_pressed(JOIN_BUTTON_X, JOIN_BUTTON_Y, JOIN_BUTTON_WIDTH, JOIN_BUTTON_HEIGHT, x, y)){
            printf("CLicked JOIN!\n");

            /* Send join packet to server */
        }
    }
}