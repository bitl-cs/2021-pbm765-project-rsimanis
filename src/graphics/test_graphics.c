#include "pong_graphics.h"
#include "pong_graphics_join.h"

render_data rend_data;
input_data inp_data;

void gameloop() {

}

void render() {

}

void init_window(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitWindowPosition(250, 200);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutCreateWindow("Pong++");
    glMatrixMode(GL_PROJECTION);
    glutIdleFunc(gameloop);
    glutDisplayFunc(render);
    glutKeyboardFunc(type_keyboard);
    glutMouseFunc(join_button_listener);
    glutMainLoop();
}

int main(int argc, char** argv){
    int i;

    rend_data.data = NULL;
    rend_data.state = 0;
    inp_data.input_text_len = 0;
    rend_data.max_displayed_message_count = CHAT_DISPLAYED_MESSAGE_COUNT_WITHOUT_INPUT_FIELD;
    rend_data.message_list_size = 1;
    for (i = 0; i < MAX_PLAYER_COUNT; i++)
        rend_data.client_ids_in_lobby[i] = -1;
    init_list(PACKET_MESSAGE_TYPE_CHAT, "Chat ready...", &rend_data.message_list_head);
    rend_data.frame_counter = 0;
    rend_data.last_update = 0;
    
    init_window(argc, argv);

    return 0;
}