#include "pong_client.h"
#include "pong_game.h"
#include "pong_networking.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>


/* init */
/* allocate shared memory for client */
client_shared_memory *get_client_shared_memory() {
    return mmap(NULL, sizeof(client_shared_memory), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
}


/* packet processing */
/* validate incoming packets */
void *receive_server_packets(void *arg) {
    /* process thread arguments */
    client_recv_thread_args *crta = (client_recv_thread_args *) arg;
    client_recv_memory *recv_mem = crta->recv_mem;
    int socket = crta->socket;

    /* initialize packet number counter for received packets */
    uint32_t recv_pn = 0;

    /* variables for code clarity */
    char *packet_ready = &recv_mem->packet_ready;
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
            /* wait until another packet is processed */
            while (*packet_ready) 
                sleep(PACKET_READY_WAIT_TIME);

            if (c == '-') {
                if (prevc == '?')
                    packet_buf[i++] = '-';
                else {
                    sep_count++;
                    if (sep_count == PACKET_SEPARATOR_SIZE) {
                        /* convert packet header to host endianess */ 
                        *pn = big_endian_to_host_uint32_t(*pn);
                        *psize = big_endian_to_host_int32_t(*psize);

                        // printstr("packet received");
                        // print_bytes(packet_buf, i);

                        /* verify packet */
                        if (verify_packet(recv_pn, packet_buf, i) != 0) {
                            *packet_ready = PACKET_READY_TRUE;
                            recv_pn = *pn + 1;
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
    client_send_thread_args *csta = (client_send_thread_args *) arg;
    client_send_memory *send_mem = csta->send_mem;
    int socket = csta->socket;

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
    client_recv_memory *recv_mem = &sh_mem->recv_mem;
    client_send_memory *send_mem = &sh_mem->send_mem;
    char *packet_ready = &recv_mem->packet_ready;
    unsigned char *pid = (unsigned char *) (recv_mem->packet_buf + PACKET_NUMBER_SIZE);
    char *pdata = recv_mem->packet_buf + PACKET_HEADER_SIZE;

    while(1) {
        if (*packet_ready == PACKET_READY_TRUE) {
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
                default:
                    printf("Invalid pid (%u)\n", (unsigned) *pid);
            }
            *packet_ready = PACKET_READY_FALSE;
        }
        else
            sleep(PACKET_READY_WAIT_TIME);
    }
}

void process_accept(char *data, client_send_memory *send_mem) {
    char status = *data;
    printf("RECEIVED ACCEPT, status=%d\n", status);

    // test
    send_game_type(GAME_TYPE_1V1, send_mem);

    // process
}

void process_message_from_server(char *data, client_send_memory *send_mem) {
    char *data_ptr = data;
    char type = *data_ptr;
    data_ptr += 1;
    char source_id = *data_ptr;
    data_ptr += 1;
    char *message = data_ptr;
    printf("Received MESSAGE, type=%d, source_id=%d, message=%s\n", type, source_id, message);
    
    // process
}

void process_lobby(char *data, client_send_memory *send_mem) {
    printf("Received LOBBY\n");
    char i, player_count, player_id, *name;

    player_count = *data;
    printf("player_count: %d\n", player_count);
    data += 1;
    for (i = 0; i < player_count; i++) {
        player_id = *data;
        data += 1;
        name = data;
        data += MAX_NAME_SIZE;
        printf("player_id: %d, name=%s\n", player_id, name);
    }
    // draw lobby
    // drawLobby(data, ...);
}

void process_game_ready(char *data, client_send_memory *send_mem) {
    printf("Received GAME_READY\n");
    int window_width, window_height;
    char team_count, team_id;
    float team_goal1X, team_goal1Y;
    float team_goal2X, team_goal2Y;
    char player_count;
    char player_id, ready, player_team_id, *name;
    float player_initial_X, player_initial_Y;
    float player_initial_width, player_initial_height;
    char i;

    window_width = big_endian_to_host_int32_t(*((int32_t *) data));
    printf("window_width: %d\n", window_width);
    data += 4;
    window_height = big_endian_to_host_int32_t(*((int32_t *) data));
    printf("window_height: %d\n", window_height);
    data += 4;
    team_count = *data;
    printf("team_count: %d\n", team_count);
    data += 1;
    for (i = 0; i < team_count; i++) {
        team_id = *data;
        printf("team_id: %d\n", team_id);
        data += 1;
        team_goal1X = big_endian_to_host_float(*((float *) data));
        printf("team_goal1x: %f\n", team_goal1X);
        data += 4;
        team_goal1Y = big_endian_to_host_float(*((float *) data));
        printf("team_goal1y: %f\n", team_goal1Y);
        data += 4;
        team_goal2X = big_endian_to_host_float(*((float *) data));
        printf("team_goal2x: %f\n", team_goal2X);
        data += 4;
        team_goal2Y = big_endian_to_host_float(*((float *) data));
        printf("team_goal2y: %f\n", team_goal2Y);
        data += 4;
    } 
    player_count = *data;
    printf("player_count: %d\n", player_count);
    data += 1;
    for (i = 0; i < player_count; i++) {
        player_id = *data;
        printf("player_id: %d\n", player_id);
        data += 1;
        ready = *data;
        printf("player_ready: %d\n", ready);
        data += 1;
        player_team_id = *data;
        printf("player_team_id: %d\n", player_team_id);
        data += 1;
        name = data;
        printf("player_name: %s\n", name);
        data += MAX_NAME_SIZE;
        player_initial_X = big_endian_to_host_float(*((float *) data));
        printf("player_initial_x: %f\n", player_initial_X);
        data += 4;
        player_initial_Y = big_endian_to_host_float(*((float *) data));
        printf("player_initial_y: %f\n", player_initial_Y);
        data += 4;
        player_initial_width = big_endian_to_host_float(*((float *) data));
        printf("player_initial_width: %f\n", player_initial_width);
        data += 4;
        player_initial_height = big_endian_to_host_float(*((float *) data));
        printf("player_initial_height: %f\n", player_initial_height);
        data += 4;
    }
    // initialize game screen

    send_player_ready(0, send_mem);
}

void process_game_state(char *data, client_send_memory *send_mem) {
    printf("Received GAME_STATE\n");
    // draw game state
}

void process_game_end(char *data, client_send_memory *send_mem) {
    printf("Received GAME_END\n");
    // draw statistics
}

void process_return_to_menu(client_send_memory *send_mem) {
    printf("Received RETURN_TO_MENU\n");
    // draw main menu
}


void send_client_packet(uint32_t pn, int32_t psize, client_send_memory *send_mem, char *buf, char *final_buf, int socket) {
    send_packet(pn, send_mem->pid, psize, send_mem->pdata, send_mem->datalen, 
                    buf, PACKET_HEADER_SIZE + PACKET_FROM_CLIENT_MAX_DATA_SIZE, 
                    final_buf, (PACKET_HEADER_SIZE + PACKET_FROM_CLIENT_MAX_DATA_SIZE) * 2 + PACKET_SEPARATOR_SIZE, 
                    socket);
}

void send_join(char *name, client_send_memory *send_mem) {
    size_t namelen;

    /* check if name is not too long */
    namelen = strlen(name);
    if (namelen > MAX_NAME_SIZE - 1)
        namelen = MAX_NAME_SIZE - 1;

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
    if (mlen > MAX_MESSAGE_SIZE - 1)
        mlen = MAX_MESSAGE_SIZE - 1;

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