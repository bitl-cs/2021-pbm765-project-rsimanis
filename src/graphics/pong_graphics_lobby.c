#include "pong_graphics_lobby.h"

void render_lobby(){
    int i;
    char name1[] = "Mr. Rozklans";
    char name2[] = "1234567890123456789";
    char name3[] = "Mr. Kurgs";
    char name4[] = "Mr. Bankroft";

    render_string(LOBBY_NAME_FIELD_X, LOBBY_NAME_FIELD_INITIAL_Y - 1 * LOBBY_NAME_FIELD_LINE_SPACING, "Currently in lobby: ", LOBBY_NAME_FIELD_TEXT_COLOR);
    render_string(LOBBY_NAME_FIELD_X, LOBBY_NAME_FIELD_INITIAL_Y, name1, LOBBY_NAME_FIELD_TEXT_COLOR);
    render_string(LOBBY_NAME_FIELD_X, LOBBY_NAME_FIELD_INITIAL_Y + 1 * LOBBY_NAME_FIELD_LINE_SPACING, name2, LOBBY_NAME_FIELD_TEXT_COLOR);
    render_string(LOBBY_NAME_FIELD_X, LOBBY_NAME_FIELD_INITIAL_Y + 2 * LOBBY_NAME_FIELD_LINE_SPACING, name3, LOBBY_NAME_FIELD_TEXT_COLOR);
    render_string(LOBBY_NAME_FIELD_X, LOBBY_NAME_FIELD_INITIAL_Y + 3 * LOBBY_NAME_FIELD_LINE_SPACING, name4, LOBBY_NAME_FIELD_TEXT_COLOR);

    render_chat_input_field(); /* Adding chat input field */

    glutMouseFunc(lobby_button_listener);
    for (i = 0; i < 4; i++) {
        render_button(CHAT_SEND_BUTTON_X, CHAT_SEND_BUTTON_INITIAL_Y + i * (CHAT_SEND_BUTTON_PADDING + CHAT_SEND_BUTTON_HEIGHT), 
                        CHAT_SEND_BUTTON_WIDTH, CHAT_SEND_BUTTON_HEIGHT, "Send message");
    }

    render_button(CHAT_SEND_ALL_BUTTON_X, CHAT_SEND_ALL_BUTTON_Y, CHAT_SEND_ALL_BUTTON_WIDTH, CHAT_SEND_ALL_BUTTON_HEIGHT, "SEND ALL");
    render_quit_button();
}

void lobby_button_listener(int button, int event, int x, int y){
    int i;

    if(button == GLUT_LEFT_BUTTON && event == GLUT_DOWN){
        if(button_pressed(QUIT_BUTTON_X, QUIT_BUTTON_Y, QUIT_BUTTON_WIDTH, QUIT_BUTTON_HEIGHT, x, y)){
            printf("QUIT_BUTTON_CLICKED!\n");
            /* quitGame(); */
                // close(client_socket);
                // draw join state
        }
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

