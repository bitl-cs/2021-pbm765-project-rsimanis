#include "pong_graphics.h"

render_info rend_info;

clock_t last_update = 0;

void gameloop(){
    clock_t now = clock();
    double diff = (double)(now - last_update) / CLOCKS_PER_SEC;
    char input;

    if(diff >= CLIENT_GAMELOOP_UPDATE_INTERVAL || last_update == 0){
        input = 0;
        if(rend_info.keys[0]){
            // printf("W is pressed!\n");
            input += 4;
            rend_info.keys[0] = 0;
        }
        if(rend_info.keys[1]){
            // printf("S is pressed!\n");
            input += 2;
            rend_info.keys[1] = 0;
        }
        if(rend_info.keys[2]){
            // printf("Q is pressed!\n");
            input += 1;
            rend_info.keys[2] = 0;
        }
        // printf("input: %d\n", input);
        glutPostRedisplay();
        last_update = now;
    }
}

int main(int argc, char** argv){
    rend_info.data = NULL;
    rend_info.state = CLIENT_STATE_LOBBY;
    rend_info.textlen = 0;
    init_list(PACKET_MESSAGE_TYPE_CHAT, "Chat ready...", &rend_info.message_list_head);
    rend_info.message_list_size = 1;
    
    glutInit(&argc, argv);
    glutInitWindowPosition(250, 200);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glMatrixMode(GL_PROJECTION);

    glutCreateWindow("Pong++");
    glutKeyboardFunc(type_keyboard);
    glutIdleFunc(gameloop);
    glutDisplayFunc(render);
    glutMainLoop();


    return 0;
}