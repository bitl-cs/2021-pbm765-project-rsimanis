#include "pong_graphics_join.h"
#include "../client/pong_client.h"

void render_join(){ // no pointer to data here needed
    /* Adding Join Button */
    render_button(JOIN_BUTTON_X, JOIN_BUTTON_Y, JOIN_BUTTON_WIDTH, JOIN_BUTTON_HEIGHT, JOIN_BUTTON_TEXT);

    /* Adding Join Name Field */
    render_outline(JOIN_NAME_FIELD_X, JOIN_NAME_FIELD_Y, JOIN_NAME_FIELD_WIDTH, JOIN_NAME_FIELD_HEIGHT, RGB_YELLOW);
    render_input_buffer(JOIN_NAME_FIELD_X + 10, JOIN_NAME_FIELD_Y + JOIN_NAME_FIELD_HEIGHT * 2 / 3, JOIN_NAME_FIELD_MAX_LEN, RGB_WHITE);

    /* Adding Join Name Field Title */
    render_string_in_the_middle(JOIN_NAME_FIELD_Y - 15, "Enter your name:", RGB_WHITE);
}

void join_button_listener(int button, int event, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && event == GLUT_DOWN) {
        if (button_pressed(JOIN_BUTTON_X, JOIN_BUTTON_Y, JOIN_BUTTON_WIDTH, JOIN_BUTTON_HEIGHT, x, y)) {
            printf("CLicked JOIN!\n");
            send_join(inp_data.input_buf, rend_data.send_mem);
        }
    }
}