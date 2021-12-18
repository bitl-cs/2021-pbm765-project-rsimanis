#include "pong_client.h"
#include "../graphics/pong_graphics.h"
#include "../graphics/pong_graphics_join.h"
#include "../graphics/pong_graphics_menu.h"
#include "../graphics/pong_graphics_lobby.h"
#include "../graphics/pong_graphics_game.h"
#include "../graphics/pong_graphics_statistics.h"
#include <GL/freeglut_std.h>

extern render_info rend_info;

/* init */
/* allocate shared memory for client */
client_shared_memory *get_client_shared_memory() {
    return mmap(NULL, sizeof(client_shared_memory), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
}

void init_render_info(client_shared_memory *sh_mem) {
    int i;

    rend_info.client_id = -1;
    rend_info.data = sh_mem->recv_mem.packet_buf + PACKET_HEADER_SIZE;
    rend_info.state = STATE_JOIN;
    rend_info.send_mem = &sh_mem->send_mem;
    rend_info.input_text_len = 0;
    rend_info.max_displayed_message_count = CHAT_DISPLAYED_MESSAGE_COUNT_WITHOUT_INPUT_FIELD;
    clear_client_lobby();
    append_info_message_to_chat("Chat ready...");
    rend_info.frame_counter = 0;
    rend_info.last_update = 0;
}

void init_graphics_window(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitWindowPosition(500, 200);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutCreateWindow("Pong++");
    glMatrixMode(GL_PROJECTION);
    glutIdleFunc(client_gameloop);
    glutDisplayFunc(client_render);
    glutKeyboardFunc(type_keyboard);
    glutMouseFunc(join_button_listener);
    glutMainLoop();
}

/* gameloop */
void client_gameloop() {
    clock_t now = clock();
    double diff = (double)(now - rend_info.last_update) / CLOCKS_PER_SEC;
    char input;

    if (diff >= CLIENT_GAMELOOP_UPDATE_INTERVAL || rend_info.last_update == 0) {
        if (rend_info.frame_counter % 3 == 0) { /* assuming that gameloop update internal is equal to 1/60 */
            input = 0;
            if (rend_info.keys[0]) {
                // printf("W is pressed!\n");
                input += 4;
                rend_info.keys[0] = 0;
            }
            if (rend_info.keys[1]) {
                // printf("S is pressed!\n");
                input += 2;
                rend_info.keys[1] = 0;
            }
            if (rend_info.keys[2]) {
                // printf("Q is pressed!\n");
                input += 1;
                rend_info.keys[2] = 0;
            }
            // printf("input: %d\n", input);
            send_player_input(input, rend_info.send_mem);
        } 
        glutPostRedisplay();
        rend_info.frame_counter++;
        rend_info.last_update = now;
    }
}

void client_render() {
    /* clear buffer */
    glClear(GL_COLOR_BUFFER_BIT);

    /* if there is something to draw, put it in buffer */
    switch (rend_info.state) {
        case STATE_JOIN:
            render_join();
            break;
        case STATE_MENU:
            render_menu();
            break;
        case STATE_LOBBY:
            render_lobby();
            break;
        case STATE_GAME_LOADING:
            render_game_loading();
            break;
        case STATE_GAME:
            render_game();
            break;
        case STATE_STATISTICS:
            render_statistics();
            break;
    }
    render_chat_message_window();

    /* put graphics on the screen */
    glutSwapBuffers();
}

/* packet processing */
void *receive_server_packets(void *arg) {
    /* process thread arguments */
    client_thread_args *cta = (client_thread_args *) arg;
    client_shared_memory *sh_mem = cta->sh_mem;
    int socket = cta->socket;

    /* initialize packet number counter for received packets */
    uint32_t recv_pn = 0;

    /* variables for code clarity */
    client_recv_memory *recv_mem = &sh_mem->recv_mem;
    char *packet_buf = recv_mem->packet_buf;
    uint32_t *pn = (uint32_t *) packet_buf;
    int32_t *psize = (int32_t *) (packet_buf + PACKET_NUMBER_SIZE + PACKET_ID_SIZE);

    /* variables for underlying algorithm */
    char c = 0, prevc = 0;
    char sep_count = 0;
    int32_t i = 0;
    int len;

    while (1) {
        if ((len = recv(socket, &c, 1, 0)) > 0) {
            if (c == '-') {
                if (prevc == '?')
                    packet_buf[i++] = '-';
                else {
                    sep_count++;
                    if (sep_count == PACKET_SEPARATOR_SIZE) {
                        // printstr("packet received");
                        // print_bytes(packet_buf, i);

                        /* verify packet */
                        if (verify_packet(recv_pn, packet_buf, i) != 0) {
                            /* convert packet header to host endianess */ 
                            *pn = big_endian_to_host_uint32_t(*pn);
                            *psize = big_endian_to_host_int32_t(*psize);

                            /* update expected packet number */
                            recv_pn = *pn + 1;

                            /* process packet data */
                            process_server_packets(sh_mem);
                        }

                        c = prevc = i = sep_count = 0;
                        continue;
                    }
                }
            }
            else if (c == '*')
                packet_buf[i++] = (prevc == '?') ? '?' : '*';
            else {
                if (prevc == '?' || (sep_count > 0 && sep_count != PACKET_SEPARATOR_SIZE)) {
                    c = prevc = i = sep_count = 0;
                    continue;
                }
                if (c != '?')
                    packet_buf[i++] = c;
            }
            prevc = c;
        }
        else if (len == 0) {
            printf("Server disconnected\n");
            exit(0);
        }
        else {
            printf("Recv() error (errno=%d)\n", errno);
            exit(-1);
        }
    }
    return NULL;
}

/* send packets to server */
void *send_client_packets(void *arg) {
    /* process thread arguments */
    client_thread_args *cta = (client_thread_args *) arg;
    client_send_memory *send_mem = &cta->sh_mem->send_mem;
    int socket = cta->socket;

    /* initialize packet number counter for sent packets */
    uint32_t send_pn = 0;

    /* allocate buffers for storing packets before they are sent (buffering) */
    char buf[PACKET_HEADER_SIZE + PACKET_FROM_CLIENT_MAX_DATA_SIZE];
    char final_buf[sizeof(buf) * 2 + PACKET_SEPARATOR_SIZE];

    while(1) {
        if (send_mem->packet_ready == PACKET_READY_TRUE) {
            switch (send_mem->pid) {
                case PACKET_JOIN_ID:
                    send_client_packet(send_pn++, PACKET_JOIN_DATA_SIZE, send_mem, buf, final_buf, socket);
                    break;
                case PACKET_MESSAGE_ID:
                    send_client_packet(send_pn++, PACKET_MESSAGE_DATA_SIZE, send_mem, buf, final_buf, socket);
                    break;
                case PACKET_PLAYER_READY_ID:
                    send_client_packet(send_pn++, PACKET_PLAYER_READY_DATA_SIZE, send_mem, buf, final_buf, socket);
                    break;
                case PACKET_PLAYER_INPUT_ID:
                    send_client_packet(send_pn++, PACKET_PLAYER_READY_DATA_SIZE, send_mem, buf, final_buf, socket);
                    break;
                case PACKET_CHECK_STATUS_ID:
                    send_client_packet(send_pn++, PACKET_CHECK_STATUS_DATA_SIZE, send_mem, buf, final_buf, socket);
                    break;
                case PACKET_GAME_TYPE_ID:
                    send_client_packet(send_pn++, PACKET_GAME_TYPE_DATA_SIZE, send_mem, buf, final_buf, socket);
                    break;
                default:
                    printf("Invalid pid (%u)\n", (unsigned) send_mem->pid);
            }
            send_mem->packet_ready = PACKET_READY_FALSE;
        }
        else
            sleep(PACKET_READY_WAIT_TIME);
    }

    return NULL;
}

/* process already validated packets */
void process_server_packets(client_shared_memory *sh_mem) {
    client_send_memory *send_mem = &sh_mem->send_mem;
    unsigned char *pid = (unsigned char *) (sh_mem->recv_mem.packet_buf + PACKET_NUMBER_SIZE);
    char *pdata = sh_mem->recv_mem.packet_buf + PACKET_HEADER_SIZE;

    switch (*pid) {
        case PACKET_ACCEPT_ID:
            process_accept(pdata, send_mem);
            break;
        case PACKET_MESSAGE_ID:
            process_message_from_server(pdata, send_mem);
            break;
        case PACKET_LOBBY_ID:
            process_lobby(pdata, send_mem);
            break;
        case PACKET_GAME_READY_ID:
            process_game_ready(pdata, send_mem);
            break;
        case PACKET_GAME_STATE_ID:
            process_game_state(pdata, send_mem);
            break;
        case PACKET_GAME_END_ID:
            process_game_end(pdata, send_mem);
            break;
        case PACKET_RETURN_TO_MENU_ID:
            process_return_to_menu(send_mem);
            break;
        case PACKET_RETURN_TO_JOIN_ID:
            process_return_to_join(send_mem);
            break;
        default:
            printf("Invalid pid (%u)\n", (unsigned) *pid);
    }
}

void process_accept(char *data, client_send_memory *send_mem) {
    char status = *data;
    printf("RECEIVED ACCEPT, status=%d\n", status);

    if (rend_info.state == STATE_JOIN) {
        if (status >= PACKET_ACCEPT_STATUS_SUCCESS) {
            clear_input_buffer();
            clear_chat();
            append_info_message_to_chat("Successfully joined game");
            glutKeyboardFunc(NULL);
            glutMouseFunc(menu_button_listener);
            rend_info.client_id = status;
            rend_info.state = STATE_MENU;
        }
        else
            printf("Accept status error (status=%d)\n", status);
    }
    else if (rend_info.state != STATE_MENU)
        printf("process_join: Invalid client state: %d\n", rend_info.state);
}

void process_message_from_server(char *data, client_send_memory *send_mem) {
    char *data_ptr = data;
    char type = *data_ptr;
    data_ptr += 1;
    char source_id = *data_ptr;
    data_ptr += 1;
    char *message = data_ptr;
    printf("Received MESSAGE, type=%d, source_id=%d, message=%s\n", type, source_id, message);

    switch (type) {
        case PACKET_MESSAGE_TYPE_CHAT:
            append_chat_message_to_chat(source_id, message);
            break;
        case PACKET_MESSAGE_TYPE_INFO:
            append_info_message_to_chat(message);
            break;
        case PACKET_MESSAGE_TYPE_ERROR:
            append_error_message_to_chat(message);
            break;
        default:
            printf("Invalid message type=%d from server\n", type);
    }    
}

void process_lobby(char *data, client_send_memory *send_mem) {
    printf("Received LOBBY\n");

    if (rend_info.state == STATE_MENU) {
        clear_chat();
        append_info_message_to_chat("Successfully joined lobby");
        glutKeyboardFunc(type_keyboard);
        glutMouseFunc(lobby_button_listener);
        rend_info.max_displayed_message_count = CHAT_DISPLAYED_MESSAGE_COUNT_WITH_INPUT_FIELD;
        rend_info.state = STATE_LOBBY;
        printf("here\n");
    }
    else if (rend_info.state != STATE_LOBBY)
        printf("process_lobby: Invalid client state: %d\n", rend_info.state);
}

void process_game_ready(char *data, client_send_memory *send_mem) {
    printf("Received GAME_READY\n");

    if (rend_info.state == STATE_LOBBY) {
        glutKeyboardFunc(NULL);
        glutMouseFunc(NULL);
        rend_info.max_displayed_message_count = CHAT_DISPLAYED_MESSAGE_COUNT_WITHOUT_INPUT_FIELD;
        rend_info.state = STATE_GAME_LOADING;
    }
    else if (rend_info.state != STATE_GAME_LOADING)
        printf("process_game_ready: Invalid client state: %d\n", rend_info.state);

    // initialize game screen

    // send_player_ready(0, send_mem);
}

void process_game_state(char *data, client_send_memory *send_mem) {
    printf("Received GAME_STATE\n");

    if (rend_info.state == STATE_GAME_LOADING) {
        glutKeyboardFunc(game_keyboard);
        rend_info.state = STATE_GAME;
    }
    else if (rend_info.state != STATE_GAME)
        printf("process_game_state: Invalid client state: %d\n", rend_info.state);

}

void process_game_end(char *data, client_send_memory *send_mem) {
    printf("Received GAME_END\n");

    if (rend_info.state == STATE_GAME) {
        glutMouseFunc(statistics_button_listener);
        glutKeyboardFunc(NULL);
        rend_info.state = STATE_STATISTICS;
    }
    else if (rend_info.state != STATE_STATISTICS)
        printf("process_game_statistics: Invalid client state: %d\n", rend_info.state);
    char i;
    char status;
    int playerTeamScore, gameDuration;

    char team_count;
    char id;
    int score;
    
    char player_count;
    char player_id, team_id;
    int player_score;
    char *name;


    status = *data;
    printf("status: %d\n", status);
    data += 1;
    playerTeamScore = big_endian_to_host_int32_t(*((int32_t *) data));
    printf("playerTeamScore: %d\n", playerTeamScore);
    data += 4;
    gameDuration = big_endian_to_host_int32_t(*((int32_t *) data));
    printf("gameDuration: %d\n", gameDuration);
    data += 4;

    team_count = *data;
    printf("team_count: %d\n", team_count);
    data += 1;
    if (status != PACKET_GAME_END_STATUS_ERROR) {
        for (i = 0; i < team_count; i++) {
            id = *data;
            printf("team_id: %d\n", id);
            data += 1;
            score = big_endian_to_host_int32_t(*((int32_t *) data));
            printf("team_score: %d\n", score);
            data += 4;
        }
    }

    player_count = *data;
    printf("player_count: %d\n", player_count);
    data += 1;
    if (status != PACKET_GAME_END_STATUS_ERROR) {
        for (i = 0; i < player_count; i++) {
            player_id = *data;
            printf("player_id: %d\n", player_id);
            data += 1;
            team_id = *data;
            printf("team_id: %d\n", team_id);
            data += 1;
            score = big_endian_to_host_int32_t(*((int32_t *) data));
            printf("player_score: %d\n", score);
            data += 4;
            name = data;
            printf("player_name: %s\n", name);
            data += MAX_NAME_LENGTH + 1;
        }
    }
    // draw statistics
}

void process_return_to_menu(client_send_memory *send_mem) {
    printf("Received RETURN_TO_MENU\n");
    if (rend_info.state == STATE_STATISTICS) {
        rend_info.state = STATE_MENU;
    }
    // draw main menu
}

void process_return_to_join(client_send_memory *send_mem) {
    printf("Received RETURN_TO_JOIN\n");
    clear_client_lobby();
    clear_chat();
    clear_input_buffer();
    rend_info.max_displayed_message_count = CHAT_DISPLAYED_MESSAGE_COUNT_WITHOUT_INPUT_FIELD;
    append_info_message_to_chat("Chat ready...");
    glutKeyboardFunc(type_keyboard);
    glutMouseFunc(join_button_listener);
    rend_info.client_id = -1;
    rend_info.state = STATE_JOIN;
}



void send_client_packet(uint32_t pn, int32_t psize, client_send_memory *send_mem, char *buf, char *final_buf, int socket) {
    send_packet(pn, send_mem->pid, psize, send_mem->pdata, send_mem->datalen, 
                    buf, PACKET_FROM_CLIENT_MAX_SIZE, 
                    final_buf, PACKET_FROM_CLIENT_MAX_SIZE * 2 + PACKET_SEPARATOR_SIZE, 
                    socket);
}

void send_join(char *name, client_send_memory *send_mem) {
    size_t namelen;

    /* check if name is not too long */
    namelen = strlen(name);
    if (namelen > MAX_NAME_LENGTH)
        namelen = MAX_NAME_LENGTH;

    /* wait until send buffer becomes available */
    while (send_mem->packet_ready == PACKET_READY_TRUE)
        sleep(PACKET_READY_WAIT_TIME);

    /* when send buffer is available, fill it with join packet data */
    send_mem->pid = PACKET_JOIN_ID;
    send_mem->datalen = insert_str(name, namelen, send_mem->pdata, sizeof(send_mem->pdata), 0);
    send_mem->packet_ready = PACKET_READY_TRUE;
}

void send_message_from_client(char type, char source_id, char *message, client_send_memory *send_mem) {
    size_t mlen, offset;

    mlen = strlen(message);
    if (mlen > MAX_MESSAGE_LENGTH)
        mlen = MAX_MESSAGE_LENGTH;

    while (send_mem->packet_ready == PACKET_READY_TRUE)
        sleep(PACKET_READY_WAIT_TIME);

    send_mem->pid = PACKET_MESSAGE_ID;
    offset = insert_char(type, send_mem->pdata, sizeof(send_mem->pdata), 0); 
    offset += insert_char(source_id, send_mem->pdata, sizeof(send_mem->pdata), offset); 
    offset += insert_str(message, mlen, send_mem->pdata, sizeof(send_mem->pdata), offset); 
    send_mem->datalen = offset;
    send_mem->packet_ready = PACKET_READY_TRUE;
}

void send_player_ready(char player_id, client_send_memory *send_mem) {
    while (send_mem->packet_ready == PACKET_READY_TRUE)
        sleep(PACKET_READY_WAIT_TIME);

    send_mem->pid = PACKET_PLAYER_READY_ID;
    send_mem->datalen = insert_char(player_id, send_mem->pdata, sizeof(send_mem->pdata), 0);
    send_mem->packet_ready = PACKET_READY_TRUE;
}

void send_player_input(char input, client_send_memory *send_mem) {
    while (send_mem->packet_ready == PACKET_READY_TRUE)
        sleep(PACKET_READY_WAIT_TIME);

    send_mem->pid = PACKET_PLAYER_INPUT_ID;
    send_mem->datalen = insert_char(input, send_mem->pdata, sizeof(send_mem->pdata), 0);
    send_mem->packet_ready = PACKET_READY_TRUE;
}

void send_check_status(client_send_memory *send_mem) {
    // do nothing
}

void send_game_type(char type, client_send_memory *send_mem) {
    while (send_mem->packet_ready == PACKET_READY_TRUE)
        sleep(PACKET_READY_WAIT_TIME);

    send_mem->pid = PACKET_GAME_TYPE_ID;
    send_mem->datalen = insert_char(type, send_mem->pdata, sizeof(send_mem->pdata), 0);
    send_mem->packet_ready = PACKET_READY_TRUE;
}

void clear_client_lobby(){
       /* Setting each players id to -1 */
    int i;
    for(i = 0; i < MAX_PLAYER_COUNT; i++){
        rend_info.client_ids_in_lobby[i] = -1;
    }

}