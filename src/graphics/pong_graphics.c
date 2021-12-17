#include "pong_graphics.h"

extern render_info rend_info;

void render(){
    /* clear buffer */
    glClear(GL_COLOR_BUFFER_BIT);

    /* if there is something to draw, put it in buffer */
    switch(rend_info.state) {
        case CLIENT_STATE_JOIN:
            render_join();
            break;
        case CLIENT_STATE_MENU:
            render_game_menu();
            break;
        case CLIENT_STATE_LOBBY:
            render_lobby();
            break;
        case CLIENT_STATE_GAME:
            render_game();
            break;
        case CLIENT_STATE_STATISTICS:
            render_statistics();
            break;
    }
    render_chat_message_window();

    /* put graphics on the screen */
    glutSwapBuffers();
}
