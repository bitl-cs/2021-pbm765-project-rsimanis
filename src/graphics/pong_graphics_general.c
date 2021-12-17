#include "pong_graphics_general.h"
#include "../networking/pong_networking.h"

render_info rend_info;

/* Keyboards */
void type_keyboard(unsigned char key, int x, int y) {
    // printf("Pressed key %c\n", key);
    if (key == '\b' && rend_info.textlen > 0)
        rend_info.input_buf[--rend_info.textlen] = '\0';
    else if (rend_info.textlen < INPUT_TEXT_MAX_LENGTH) {
        rend_info.input_buf[rend_info.textlen++] = key;
        rend_info.input_buf[rend_info.textlen + 1] = '\0';
    }
    print_input_buffer();
}

void game_keyboard(unsigned char key, int x, int y) {
    if(key == 'w' || key == 'W'){
        rend_info.keys[0] = 1;
    }
    if(key == 's' || key == 'S'){
        rend_info.keys[1] = 1;
    }
    if(key == 'q' || key == 'Q'){
        rend_info.keys[2] = 1;
    }
}

/* Shapes */
void render_outline(int x, int y, int width, int height, RGB color) {
    glPointSize(3);
    glColor3f(color.r, color.g, color.b);

    glBegin(GL_LINE_LOOP);
        glVertex2f(ctosl(x + width, 'x'), ctosl(y, 'y'));
        glVertex2f(ctosl(x, 'x'), ctosl(y, 'y'));
        glVertex2f(ctosl(x, 'x'), ctosl(y + height, 'y'));
        glVertex2f(ctosl(x + width, 'x'), ctosl(y + height, 'y'));
    glEnd();
}

void render_rectangle(int x, int y, int width, int height, RGB color) {
    glPointSize(1);
    glColor3f(color.r, color.g, color.b);

    glBegin(GL_POLYGON);
        glVertex2f(ctosl(x + width, 'x'), ctosl(y, 'y'));
        glVertex2f(ctosl(x, 'x'), ctosl(y, 'y'));
        glVertex2f(ctosl(x, 'x'), ctosl(y + height, 'y'));
        glVertex2f(ctosl(x + width, 'x'), ctosl(y + height, 'y'));
    glEnd();
}

void render_circle(int x, int y, int radius, RGB color) {
    int i;
    float theta;

    glColor3f(color.r, color.g, color.b);
    glBegin(GL_POLYGON);
        for(i = 0; i < 360; i++){
            theta = i * M_PI / 180;
            glVertex2f(ctosl(x + radius + radius * cos(theta), 'x'), ctosl(y + radius + radius * sin(theta), 'y'));
        }
    glEnd();
}

/* Strings */
void render_char(int x, int y, const unsigned char c, RGB color) {
    glColor3f(color.r, color.g, color.b); 
    glRasterPos2f(ctosl(x, 'x'), ctosl(y, 'y'));
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
}

void render_string(int x, int y, char* string, RGB color) {
    glColor3f(color.r, color.g, color.b); 
    glRasterPos2f(ctosl(x, 'x'), ctosl(y, 'y'));
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char *) string);
}

void render_int(int x, int y, int number, RGB color) {
    char temp[10]; 
    sprintf(temp, "%d", number);
    render_string(x, y, temp, color);
}

void render_int_with_tag(int x, int y, char *tag, int num, RGB color) {
    char temp[100];
    sprintf(temp, "%s: %d", tag, num);
    render_string(x, y, temp, color);
}

int bitmapped_string_length(char *text) {
    return glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (unsigned char *) text);
}

int bitmapped_string_height() {
    return glutBitmapHeight(GLUT_BITMAP_HELVETICA_18);
}

/* Chat */
void render_chat_message_window() {
    int i;
    lnode *mnode;
    // RGB chat_window_color = {1.0, 1.0, 0.5};
    render_outline(CHAT_WINDOW_X, CHAT_WINDOW_Y, CHAT_WINDOW_WIDTH, CHAT_WINDOW_HEIGHT, CHAT_OUTLINE_COLOR);

    mnode = rend_info.message_list_head;
    for (i = 0; i < rend_info.message_list_size; i++) { 
        render_chat_message(mnode, CHAT_MESSAGE_X, CHAT_MESSAGE_INITIAL_Y + i * CHAT_MESSAGE_LINE_SPACING);
        mnode = mnode->next;
    }
}

void render_chat_message(lnode *mnode, int x, int y) {
    switch (mnode->message_type) {
        case PACKET_MESSAGE_TYPE_CHAT:
            render_string(x, y, mnode->message, CHAT_NORMAL_MESSAGE_COLOR);
            break;
        case PACKET_MESSAGE_TYPE_INFO: 
            render_string(x, y, mnode->message, CHAT_INFO_MESSAGE_COLOR);
            break;
        case PACKET_MESSAGE_TYPE_ERROR: 
            render_string(x, y, mnode->message, CHAT_ERROR_MESSAGE_COLOR);
            break;
        default:
            printf("Invalid message type (%d)\n", mnode->message_type);
    }
}

void render_chat_input_field() {
    render_outline(CHAT_INPUT_FIELD_X, CHAT_INPUT_FIELD_Y, CHAT_INPUT_FIELD_WIDTH, CHAT_INPUT_FIELD_HEIGHT, RGB_GREEN);
    render_string(CHAT_INPUT_FIELD_X + 10, CHAT_INPUT_FIELD_Y + 22, rend_info.input_buf, RGB_WHITE); 
}

void render_input_buffer(int x, int y, RGB color) {
    render_string(x, y, rend_info.input_buf, color);
}

void empty_input_buffer() {
    rend_info.input_buf[0] = '\0';
    rend_info.textlen = 0;
}

/* Button Handling */
int button_pressed(int btn_x, int btn_y, int btn_width, int btn_height, int mouse_x, int mouse_y) {
    //printf("BTN_X : %d-%d, Y: %d-%d, Mouse: %d-%d\n", btn_x, btn_x + btn_width, btn_y, btn_y + btn_height, mouse_x, mouse_y);
    return mouse_x >= btn_x && mouse_x <= (btn_x + btn_width) && mouse_y >= btn_y && mouse_y <= (btn_y + btn_height);
}

void render_button(int x, int y, int width, int height, char* text) {
    int textlen;

    render_rectangle(x, y, width, height, RGB_WHITE);
    textlen = bitmapped_string_length(text);
    render_string(x + width / 2 - textlen / 2, y + height / 2 + bitmapped_string_height() / 2, text, RGB_BLACK);
}

void render_quit_button() {
    render_button(QUIT_BUTTON_X, QUIT_BUTTON_Y, QUIT_BUTTON_WIDTH, QUIT_BUTTON_HEIGHT, "QUIT");
}

/* Helpers */
float ctosl(float coord, char coord_type) {
    float ret_val;

    if(coord_type == 'x'){ // for x coordinate
        if(coord >= 0 && coord <= WINDOW_WIDTH){ // if x coord is on screen
            ret_val = ((coord / WINDOW_WIDTH) - 0.5) * 2;
        } else {
            printf("Given coordinate %f is not on a screen, returned %c coord as 0.0!\n", coord, coord_type);
        }
    } else if(coord_type == 'y'){ //for y coordinate
        if(coord >= 0 && coord <= WINDOW_HEIGHT){ //if coord is on screen
            ret_val = ((coord / WINDOW_HEIGHT) - 0.5) * (-2);
        } else {
            printf("Given coordinate %f is not on a screen, returned %c coord as 0.0!\n", coord, coord_type);
        }
    } else {
        printf("No such dimension %c!\n", coord_type);
    }

    /* for debugging */
    //printf("Converted %f to %f\n", coord, ret_val);

    return ret_val;
}

/* Debug */
void print_input_buffer() {
    printf("%s\n", rend_info.input_buf);
}