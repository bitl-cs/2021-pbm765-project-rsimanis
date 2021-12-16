#include "graphics.h"
#include "message_list.h"
#include "pong_networking.h"

#include <GL/freeglut_std.h>

// char state = 3;
// char buffer[BUFFER_MAX_SIZE + 1];
// int bufsize = 0;
// char *data;

// char keys[3]; /* [w, s, q] */
// char input = 0;
render_info rend_info;

void empty_input_buffer() {
    rend_info.input_buf[0] = '\0';
    rend_info.textlen = 0;
}

float coords_to_screen_location(float coord, char coord_type) // coord_type is either 'x' or 'y'
{
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

/* Button Listeners */
void join_button_listener(int button, int event, int x, int y){
    if(button == GLUT_LEFT_BUTTON && event == GLUT_DOWN){
        if(button_pressed(PLAY_BUTTON_X, PLAY_BUTTON_Y, PLAY_BUTTON_WIDTH, PLAY_BUTTON_HEIGHT, x, y)){
            printf("CLicked JOIN!\n");

            /* Send join packet to server */
        }
    }
}

void game_type_button_listener(int button, int event, int x, int y){
    if(button == GLUT_LEFT_BUTTON && event == GLUT_DOWN){
        if(button_pressed(ONE_V_ONE_X, ONE_V_ONE_Y, GAME_TYPE_BUTON_WIDTH, GAME_TYPE_BUTON_HEIGHT, x, y)){
            printf("CLicked 1V1!\n");
            /* Send game type packet to server (1v1) */
        } else if(button_pressed(TWO_V_TWO_X, TWO_V_TWO_Y, GAME_TYPE_BUTON_WIDTH, GAME_TYPE_BUTON_HEIGHT, x, y)){
            printf("Clicked 2V2!\n");
            /* Send game type packet to server (2v2) */
        }
            
        
    }
}

void lobby_send_message_listener(int button, int event, int x, int y){
    int i;

    if(button == GLUT_LEFT_BUTTON && event == GLUT_DOWN){
        if(button_pressed(CHAT_SEND_ALL_BUTTON_X, CHAT_SEND_ALL_BUTTON_Y, CHAT_SEND_ALL_BUTTON_WIDTH, CHAT_SEND_ALL_BUTTON_HEIGHT, x, y)){
            printf("CLicked SEND ALL!\n");
            /* send message TO ALL to server */
            empty_input_buffer();
        }
        for (i = 0; i < 4; i++) {
            if(button_pressed(CHAT_SEND_BUTTON_X, CHAT_SEND_BUTTON_INITIAL_Y + i * (CHAT_SEND_BUTTON_PADDING + CHAT_SEND_BUTTON_HEIGHT) , 
                                CHAT_SEND_BUTTON_WIDTH, CHAT_SEND_BUTTON_HEIGHT, x, y)){
                printf("CLicked button id=%d!\n", i);
                /* send message TO ALL to server */
                // send_message() id = rend_info.client_ids_in_lobby[i] (SEND IF id >= 0)
                empty_input_buffer();
            }
        }
    }
}


/* Button Click Detection Helper */
int button_pressed(int btn_x, int btn_y, int btn_width, int btn_height, int mouse_x, int mouse_y){
    //printf("Checking if buton pressed!\n");
    //printf("BTN_X : %d-%d, Y: %d-%d, Mouse: %d-%d\n", btn_x, btn_x + btn_width, btn_y, btn_y + btn_height, mouse_x, mouse_y);
    return mouse_x >= btn_x && mouse_x <= (btn_x + btn_width) && mouse_y >= btn_y && mouse_y <= (btn_y + btn_height);
}

/* Keyboard listener */
void type_keyboard(unsigned char key, int x, int y){
    // printf("Pressed key %c\n", key);
    if (key == '\b' && rend_info.textlen > 0)
        rend_info.input_buf[--rend_info.textlen] = '\0';
    else if (rend_info.textlen < INPUT_MESSAGE_MAX_LEN) {
        rend_info.input_buf[rend_info.textlen++] = key;
        rend_info.input_buf[rend_info.textlen + 1] = '\0';
    }
    print_input_buf();
}

void game_keyboard(unsigned char key, int x, int y){
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

/* Object Drawing */
void draw_rectangle(int x, int y, int width, int height, RGB rgb){
    
    glPointSize(1);
    glColor3f(rgb.r, rgb.g, rgb.b);

    glBegin(GL_POLYGON);
        glVertex2f(coords_to_screen_location(x + width, 'x'), coords_to_screen_location(y, 'y'));
        glVertex2f(coords_to_screen_location(x, 'x'), coords_to_screen_location(y, 'y'));
        glVertex2f(coords_to_screen_location(x, 'x'), coords_to_screen_location(y + height, 'y'));
        glVertex2f(coords_to_screen_location(x + width, 'x'), coords_to_screen_location(y + height, 'y'));
    glEnd();
}

void draw_outline(int x, int y, int width, int height, RGB rgb){
    
    glPointSize(3);
    glColor3f(rgb.r, rgb.g, rgb.b);

    glBegin(GL_LINE_LOOP);
        glVertex2f(coords_to_screen_location(x + width, 'x'), coords_to_screen_location(y, 'y'));
        glVertex2f(coords_to_screen_location(x, 'x'), coords_to_screen_location(y, 'y'));
        glVertex2f(coords_to_screen_location(x, 'x'), coords_to_screen_location(y + height, 'y'));
        glVertex2f(coords_to_screen_location(x + width, 'x'), coords_to_screen_location(y + height, 'y'));
    glEnd();
    
}

void draw_circle(int x, int y, int radius){
    int i;
    float theta;

    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_POLYGON);
        for(i = 0; i < 360; i++){
            theta = i * M_PI / 180;
            glVertex2f(coords_to_screen_location(x + radius + radius * cos(theta), 'x'), coords_to_screen_location(y + radius + radius * sin(theta), 'y'));
        }
    glEnd();

}

void draw_button(int x, int y, int width, int height,  char* text, void (*callback)(int, int, int, int)){
    // RGB button_color = {1.0f, 1.0f, 1.0f};
    // RGB button_text_color = {0.0f, 0.0f, 0.0f};

    glutMouseFunc(callback);
    draw_rectangle(x, y, width, height, RGB_WHITE);
    // render_string(x - x/75 + width / 3, y + y/60 + height / 2, text, RGB_BLACK);
    render_string(x + 15, y + y/60 + height / 2, text, RGB_BLACK);

}

/* Drawing Chat component */
void draw_chat(){
    int i;
    lnode *mnode;
    // RGB chat_window_color = {1.0, 1.0, 0.5};
    draw_outline(CHAT_WINDOW_X, CHAT_WINDOW_Y, CHAT_WINDOW_WIDTH, CHAT_WINDOW_HEIGHT, CHAT_OUTLINE_COLOR);

    mnode = rend_info.message_list_head;
    for (i = 0; i < rend_info.message_list_size; i++) { 
        render_chat_message(mnode, CHAT_MESSAGE_X, CHAT_MESSAGE_INITIAL_Y + i * CHAT_LINE_SPACING);
        mnode = mnode->next;
    }
}

void draw_type_field() {
    draw_outline(CHAT_TYPE_FIELD_X, CHAT_TYPE_FIELD_Y, CHAT_TYPE_FIELD_WIDTH, CHAT_TYPE_FIELD_HEIGHT, RGB_GREEN);
    render_string(CHAT_TYPE_FIELD_X + 10, CHAT_TYPE_FIELD_Y + 22, rend_info.input_buf, TEXT_COLOR); 
}

void render_chat_message(lnode *mnode, int x, int y) {
    switch (mnode->message_type) {
        case PACKET_MESSAGE_TYPE_CHAT:
            render_string(x, y, mnode->message, CHAT_MESSAGE_COLOR);
            break;
        case PACKET_MESSAGE_TYPE_INFO: 
            render_string(x, y, mnode->message, INFO_MESSAGE_COLOR);
            break;
        case PACKET_MESSAGE_TYPE_ERROR: 
            render_string(x, y, mnode->message, ERROR_MESSAGE_COLOR);
            break;
        default:
            printf("Invalid message type (%d)\n", mnode->message_type);
    }

}

void print_input_buf(){
    printf("%s\n", rend_info.input_buf);
}

/* Drawing Screens */
void draw_join(){ // no pointer to data here needed
    /* Play Button Data */
    int space_between_components = 5;
    /* User Input Frame Data */
    int text_frame_width = 160;
    int text_frame_height = 40;
    int text_frame_x = (WINDOW_WIDTH - text_frame_width) / 2;
    int text_frame_y = ((WINDOW_HEIGHT - text_frame_height) / 2) - space_between_components - text_frame_height;

    /* Adding Join Button */
    draw_button(PLAY_BUTTON_X, PLAY_BUTTON_Y, PLAY_BUTTON_WIDTH, PLAY_BUTTON_HEIGHT, JOIN_BUTTON_TEXT, join_button_listener);
    /*Adding Text Frame */
    draw_outline(text_frame_x, text_frame_y, text_frame_width, text_frame_height, TEXT_FRAME_COLOR);

    render_string(text_frame_x + text_frame_width / 20, text_frame_y + text_frame_height * 2/ 3, rend_info.input_buf, TEXT_COLOR);
}

void draw_game_main_menu(){

    /* Adding Buttons */
    draw_button(ONE_V_ONE_X, ONE_V_ONE_Y, GAME_TYPE_BUTON_WIDTH, GAME_TYPE_BUTON_HEIGHT, ONE_V_ONE_BUTTON_TEXT, game_type_button_listener);
    draw_button(TWO_V_TWO_X, TWO_V_TWO_Y, GAME_TYPE_BUTON_WIDTH, GAME_TYPE_BUTON_HEIGHT, TWO_V_TWO_BUTTON_TEXT, game_type_button_listener);
}

void draw_lobby(){
    int i;
    char name1[] = "Mr. Rozklans";
    char name2[] = "1234567890123456789";
    char name3[] = "Mr. Kurgs";
    char name4[] = "Mr. Bankroft";

    render_string(LOBBY_NAME_X, LOBBY_NAME_INITIAL_Y - 1 * LOBBY_NAME_SPACING, "Currently in lobby: ", NAME_COLOR);
    render_string(LOBBY_NAME_X, LOBBY_NAME_INITIAL_Y, name1, NAME_COLOR);
    render_string(LOBBY_NAME_X, LOBBY_NAME_INITIAL_Y + 1 * LOBBY_NAME_SPACING, name2, NAME_COLOR);
    render_string(LOBBY_NAME_X, LOBBY_NAME_INITIAL_Y + 2 * LOBBY_NAME_SPACING, name3, NAME_COLOR);
    render_string(LOBBY_NAME_X, LOBBY_NAME_INITIAL_Y + 3 * LOBBY_NAME_SPACING, name4, NAME_COLOR);
    for (i = 0; i < 4; i++) {
        draw_button(CHAT_SEND_BUTTON_X, CHAT_SEND_BUTTON_INITIAL_Y + i * (CHAT_SEND_BUTTON_PADDING + CHAT_SEND_BUTTON_HEIGHT), 
                        CHAT_SEND_BUTTON_WIDTH, CHAT_SEND_BUTTON_HEIGHT, "Send message", lobby_send_message_listener);
    }

    draw_button(CHAT_SEND_ALL_BUTTON_X, CHAT_SEND_ALL_BUTTON_Y, CHAT_SEND_ALL_BUTTON_WIDTH, CHAT_SEND_ALL_BUTTON_HEIGHT, "SEND ALL", lobby_send_message_listener);
}

void draw_game_state(){
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
    draw_rectangle(pl1x, pl1y, pw, ph, PADDLE_COLOR);
    draw_rectangle(pl2x, pl2y, pw, ph, PADDLE_COLOR);
    draw_circle(blx, bly, blr);
    render_int(SCORE_1_X, SCORE_HEIGHT, scr1, SCORE_COLOR);
    render_int(SCORE_2_X, SCORE_HEIGHT, scr2, SCORE_COLOR);
}

void draw_statistics() {
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

/* String Rendering */
void render_string(int x, int y, char* string, RGB rgb){ 
    //glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
    glColor3f(rgb.r, rgb.g, rgb.b); 
    glRasterPos2f(coords_to_screen_location(x, 'x'), coords_to_screen_location(y, 'y'));
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char *) string);

}

void render_char(int x, int y, const unsigned char c, RGB rgb){ 
    //glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
    glColor3f(rgb.r, rgb.g, rgb.b); 
    glRasterPos2f(coords_to_screen_location(x, 'x'), coords_to_screen_location(y, 'y'));
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
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


void render(){
    /* clear buffer */
    glClear(GL_COLOR_BUFFER_BIT);

    /* if there is something to draw, put it in buffer */
    switch(rend_info.state){
        case(DRAW_JOIN):
            draw_join();
            break;
        case(DRAW_MAIN_MENU):
            draw_game_main_menu();
            break;
        case(DRAW_LOBBY):
            draw_lobby();
            draw_type_field();
            break;
        case(DRAW_GAME_STATE):
            draw_game_state();
            break;
        case(DRAW_STATISTICS):
            draw_statistics();
            break;
    }
    draw_chat();

    /* put graphics on the screen */
    glutSwapBuffers();
}
