#include "pong_graphics_lobby.h"
#include "../client/pong_client.h"
#include "pong_graphics.h"

void render_lobby(){
    char *data;
    char i, player_count, client_id, *name;

    /* Adding lobby player list title */
    render_string(LOBBY_NAME_FIELD_X, LOBBY_NAME_FIELD_INITIAL_Y - 1 * LOBBY_NAME_FIELD_LINE_SPACING, "Currently in lobby: ", LOBBY_NAME_FIELD_TEXT_COLOR);

    /* Adding player names and "SEND" buttons (for each player) */
    data = rend_info.data;
    player_count = *data;
    data += 1;
    for (i = 0; i < player_count; i++) {
        client_id = *data;
        data += 1;
        name = data;
        data += MAX_NAME_LENGTH + 1;
        if (rend_info.client_ids_in_lobby[i] != client_id) {
            rend_info.client_ids_in_lobby[i] = client_id;
            strcpy(rend_info.client_names_in_lobby[i], name);
        }
        if(rend_info.client_id != rend_info.client_ids_in_lobby[i]){
            render_button(CHAT_SEND_BUTTON_X, CHAT_SEND_BUTTON_INITIAL_Y + i * (CHAT_SEND_BUTTON_PADDING + CHAT_SEND_BUTTON_HEIGHT), 
                            CHAT_SEND_BUTTON_WIDTH, CHAT_SEND_BUTTON_HEIGHT, "Send message");
        }
            render_string(LOBBY_NAME_FIELD_X, LOBBY_NAME_FIELD_INITIAL_Y + i * LOBBY_NAME_FIELD_LINE_SPACING, name, LOBBY_NAME_FIELD_TEXT_COLOR);
    }

    /* Adding chat input field */
    render_chat_input_field(); 
    render_button(CHAT_SEND_ALL_BUTTON_X, CHAT_SEND_ALL_BUTTON_Y, CHAT_SEND_ALL_BUTTON_WIDTH, CHAT_SEND_ALL_BUTTON_HEIGHT, "SEND ALL");

    /* Adding quit button */
    render_quit_button();
}

void lobby_button_listener(int button, int event, int x, int y){
    int i;
    /* Finig the place (by index), where "Send" to player himself button must not be listened for (it does not exist)*/
    int player_id_index = match_id_to_index(rend_info.client_id);

    if(button == GLUT_LEFT_BUTTON && event == GLUT_DOWN){
        if(button_pressed(QUIT_BUTTON_X, QUIT_BUTTON_Y, QUIT_BUTTON_WIDTH, QUIT_BUTTON_HEIGHT, x, y)){
            printf("QUIT_BUTTON_CLICKED!\n");
            /* Setting "quit" input as pressed */
            rend_info.keys[2] = 1;
            
        }
        if(button_pressed(CHAT_SEND_ALL_BUTTON_X, CHAT_SEND_ALL_BUTTON_Y, CHAT_SEND_ALL_BUTTON_WIDTH, CHAT_SEND_ALL_BUTTON_HEIGHT, x, y)){
            printf("CLicked SEND ALL!\n");
            /* send message TO ALL to server */
            clear_input_buffer();

        

        }
        for (i = 0; i < 4; i++) {
            if(i != player_id_index && button_pressed(CHAT_SEND_BUTTON_X, CHAT_SEND_BUTTON_INITIAL_Y + i * (CHAT_SEND_BUTTON_PADDING + CHAT_SEND_BUTTON_HEIGHT) , 
                                CHAT_SEND_BUTTON_WIDTH, CHAT_SEND_BUTTON_HEIGHT, x, y)){
                printf("CLicked button id=%d!\n", i);
                /* send message TO ALL to server */
                // send_message() id = rend_info.client_ids_in_lobby[i] (SEND IF id >= 0)
                clear_input_buffer();
            }
        }
    }
}


