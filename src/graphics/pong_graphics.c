#include "pong_graphics.h"
#include <GL/freeglut_std.h>
#include <string.h>

/* Keyboards */
void type_keyboard(unsigned char key, int x, int y) {
    // printf("Pressed key %c\n", key);
    if (key == '\b' && inp_data.input_text_len > 0)
        inp_data.input_buf[--inp_data.input_text_len] = '\0';
    else if (inp_data.input_text_len < INPUT_TEXT_MAX_LENGTH) {
        inp_data.input_buf[inp_data.input_text_len++] = key;
        inp_data.input_buf[inp_data.input_text_len] = '\0';
    }
}

void game_pressed_keyboard(unsigned char key, int x, int y) {
    if(key == 'w' || key == 'W')
        inp_data.keys[KEY_UP_INDEX] = KEY_PRESSED;
    if(key == 's' || key == 'S')
        inp_data.keys[KEY_DOWN_INDEX] = KEY_PRESSED;
    // if(key == 'l' || key == 'L')
    //     inp_data.keys[KEY_QUIT_INDEX] = KEY_PRESSED;
}

void game_released_keyboard(unsigned char key, int x, int y) {
    if(key == 'w' || key == 'W')
        inp_data.keys[KEY_UP_INDEX] = KEY_RELEASED;
    if(key == 's' || key == 'S')
        inp_data.keys[KEY_DOWN_INDEX] = KEY_RELEASED;
    // if(key == 'l' || key == 'L')
    //     inp_data.keys[KEY_QUIT_INDEX] = KEY_RELEASED;
}

void game_special_pressed_keyboard(int key, int x, int y) {
    if (key == GLUT_KEY_UP)
        inp_data.keys[KEY_UP_INDEX] = KEY_PRESSED;
    if (key == GLUT_KEY_DOWN)
        inp_data.keys[KEY_DOWN_INDEX] = KEY_PRESSED;
}

void game_special_released_keyboard(int key, int x, int y) {
    if (key == GLUT_KEY_UP)
        inp_data.keys[KEY_UP_INDEX] = KEY_RELEASED;
    if (key == GLUT_KEY_DOWN)
        inp_data.keys[KEY_DOWN_INDEX] = KEY_RELEASED;
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
            glVertex2f(ctosl(x + radius * cos(theta), 'x'), ctosl(y + radius * sin(theta), 'y'));
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

void render_char_arr(int x, int y, char *chars, int len, RGB color) {
    if (len >= 100)
        len = 99;
    char temp[100];
    strncpy(temp, chars, len);
    temp[len + 1] = '\0';
    render_string(x, y, temp, color);
}

void render_string_in_the_middle(int y, char *string, RGB color) {
    render_string(WINDOW_WIDTH / 2 - bitmapped_string_length(string) / 2, y, string, color);
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
    int i, height;
    mnode *mnode;

    height = (rend_data.max_displayed_message_count == CHAT_DISPLAYED_MESSAGE_COUNT_WITH_INPUT_FIELD) ? 
                                                             CHAT_WINDOW_HEIGHT_WITH_INPUT_FIELD : 
                                                             CHAT_WINDOW_HEIGHT_WITHOUT_INPUT_FIELD;
    render_outline(CHAT_WINDOW_X, CHAT_WINDOW_Y, CHAT_WINDOW_WIDTH, height, CHAT_OUTLINE_COLOR);

    mnode = rend_data.message_list_head;
    for (i = 0; i < rend_data.message_list_size; i++) { 
        render_chat_message(mnode, CHAT_MESSAGE_X, CHAT_MESSAGE_INITIAL_Y + i * CHAT_MESSAGE_LINE_SPACING);
        mnode = mnode->next;
    }
}

void render_chat_message(mnode *mnode, int x, int y) {
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
    render_string(CHAT_INPUT_FIELD_X + 10, CHAT_INPUT_FIELD_Y + 22, inp_data.input_buf, RGB_WHITE); 
}

void render_input_buffer(int x, int y, int maxlen, RGB color) {
    if (inp_data.input_text_len > maxlen)
        render_char_arr(x, y, inp_data.input_buf, maxlen, color);
    else
        render_string(x, y, inp_data.input_buf, color);
}

void clear_input_buffer() {
    inp_data.input_buf[0] = '\0';
    inp_data.input_text_len = 0;
}

void pop_front_message_from_chat() {
    pop_front(&rend_data.message_list_head);
    rend_data.message_list_size--;
}

void append_message_to_chat(char type, char *message) {
    if (rend_data.message_list_size == rend_data.max_displayed_message_count)
        pop_front_message_from_chat();
    push_back(type, message, &rend_data.message_list_head);
    rend_data.message_list_size++;
}

void append_chat_message_to_chat(char source_id, char *message) {
    int name_index = match_id_to_index(source_id);
    char message_with_username[MAX_NAME_LENGTH + 2 + MAX_MESSAGE_LENGTH + 1];

    if (name_index == -1) {
        printf("Invalid source_id=%d", source_id);
        return;
    }
    strcpy(message_with_username, rend_data.client_names_in_lobby[match_id_to_index(source_id)]);
    strcat(message_with_username, ": ");
    strcat(message_with_username, message);

    append_message_to_chat(PACKET_MESSAGE_TYPE_CHAT, message_with_username);
}

void append_info_message_to_chat(char *message) {
    append_message_to_chat(PACKET_MESSAGE_TYPE_INFO, message);
}

void append_error_message_to_chat(char *message) {
    append_message_to_chat(PACKET_MESSAGE_TYPE_ERROR, message);
}

int match_id_to_index(char source_id){
    int i;

    if (source_id < 0)
        return -1;

    for(i = 0; i < MAX_PLAYER_COUNT; i++) {
        if(rend_data.client_ids_in_lobby[i] == source_id)
            return i;
    }
    return -1;
}

void clear_chat() {
    int i;

    for (i = 0; i < rend_data.message_list_size; i++)
        pop_front(&rend_data.message_list_head);
    rend_data.message_list_size = 0;
}

/* Button Handling */
int button_pressed(int btn_x, int btn_y, int btn_width, int btn_height, int mouse_x, int mouse_y) {
    //printf("BTN_X : %d-%d, Y: %d-%d, Mouse: %d-%d\n", btn_x, btn_x + btn_width, btn_y, btn_y + btn_height, mouse_x, mouse_y);
    return mouse_x >= btn_x && mouse_x <= (btn_x + btn_width) && mouse_y >= btn_y && mouse_y <= (btn_y + btn_height);
}

void render_button(int x, int y, int width, int height, char* text) {
    int input_text_len;

    render_rectangle(x, y, width, height, RGB_WHITE);
    input_text_len = bitmapped_string_length(text);
    render_string(x + width / 2 - input_text_len / 2, y + height / 2 + bitmapped_string_height() / 2, text, RGB_BLACK);
}

void render_quit_button() {
    render_button(QUIT_BUTTON_X, QUIT_BUTTON_Y, QUIT_BUTTON_WIDTH, QUIT_BUTTON_HEIGHT, "QUIT");
}

/* Helpers */
/* coordinates to screen location */
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
    printf("%s\n", inp_data.input_buf);
}