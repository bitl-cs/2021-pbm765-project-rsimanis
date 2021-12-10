#include "pong_client.h"
#include "pong_game.h"
#include "pong_networking.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>


/* allocate shared memory for client */
void get_client_shared_memory(client_shared_memory_config *sh_mem_cfg) {
    char *sh_mem_ptr;

    sh_mem_ptr = (char *) mmap(NULL, CLIENT_SHARED_MEMORY_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    sh_mem_cfg->shared_memory = sh_mem_ptr;
    get_recv_memory_config(sh_mem_ptr, CLIENT_RECV_MEMORY_SIZE, &(sh_mem_cfg->recv_mem_cfg));
    sh_mem_ptr += CLIENT_RECV_MEMORY_SIZE;
    get_send_memory_config(sh_mem_ptr, CLIENT_SEND_MEMORY_SIZE, &(sh_mem_cfg->send_mem_cfg));
}

/* process already validated packets */
void process_server_packets(recv_memory_config *recv_mem_cfg) {
    while(1) {
        if (*(recv_mem_cfg->packet_ready) == PACKET_READY_TRUE) {
            switch (*(recv_mem_cfg->pid)) {
                case PACKET_ACCEPT_ID:
                    process_accept(recv_mem_cfg->pdata);
                    break;
                case PACKET_MESSAGE_ID:
                    process_message_from_server(recv_mem_cfg->pdata);
                    break;
                case PACKET_LOBBY_ID:
                    process_lobby(recv_mem_cfg->pdata);
                    break;
                case PACKET_GAME_READY_ID:
                    process_game_ready(recv_mem_cfg->pdata);
                    break;
                case PACKET_GAME_STATE_ID:
                    process_game_state(recv_mem_cfg->pdata);
                    break;
                case PACKET_GAME_END_ID:
                    process_game_end(recv_mem_cfg->pdata);
                    break;
                default:
                    printf("Invalid pid (%u)\n", (unsigned) *(recv_mem_cfg->pid));
            }
            *(recv_mem_cfg->packet_ready) = PACKET_READY_FALSE;
        }
        else
            sleep(PACKET_READY_WAIT_TIME);
    }
}

/* send packets to server */
void *send_client_packets(void *arg) {
    /* process thread arguments */
    send_thread_args *sta = (send_thread_args *) arg;
    int socket = sta->socket;
    send_memory_config *send_mem_cfg = sta->send_mem_cfg;

    /* initialize packet number counter for sent packets */
    uint32_t send_pn = 0;

    *(send_mem_cfg->packet_ready) = PACKET_READY_FALSE;
    while(1) {
        if (*(send_mem_cfg->packet_ready) == PACKET_READY_TRUE) {
            switch (*(send_mem_cfg->pid)) {
                case PACKET_JOIN_ID:
                    send_packet(send_pn++, PACKET_JOIN_DATA_SIZE, send_mem_cfg, socket);
                    break;
                case PACKET_MESSAGE_ID:
                    send_packet(send_pn++, PACKET_MESSAGE_DATA_SIZE, send_mem_cfg, socket);
                    break;
                case PACKET_PLAYER_READY_ID:
                    send_packet(send_pn++, PACKET_PLAYER_READY_DATA_SIZE, send_mem_cfg, socket);
                    break;
                case PACKET_PLAYER_INPUT_ID:
                    send_packet(send_pn++, PACKET_PLAYER_READY_DATA_SIZE, send_mem_cfg, socket);
                    break;
                case PACKET_CHECK_STATUS_ID:
                    send_packet(send_pn++, PACKET_CHECK_STATUS_DATA_SIZE, send_mem_cfg, socket);
                    break;
                default:
                    printf("Invalid pid (%u)\n", (unsigned) *(send_mem_cfg->pid));
            }
            *(send_mem_cfg->packet_ready) = PACKET_READY_FALSE;
        }
        else
            sleep(PACKET_READY_WAIT_TIME);
    }

    return NULL;
}

void process_accept(char *data) {

}

void process_message_from_server(char *data) {

}

void process_lobby(char *data) {
    
}

void process_game_ready(char *data) {

}

void process_game_state(char *data) {

}

void process_game_end(char *data) {

}




