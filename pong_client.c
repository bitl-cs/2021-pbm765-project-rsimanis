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
void get_client_shared_memory(client_shared_memory_config *sh_mem_cfg) {
    char *sh_mem_ptr;

    sh_mem_ptr = (char *) mmap(NULL, CLIENT_SHARED_MEMORY_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    sh_mem_cfg->shared_memory = sh_mem_ptr;
    get_recv_memory_config(sh_mem_ptr, CLIENT_RECV_MEMORY_SIZE, &(sh_mem_cfg->recv_mem_cfg));
    sh_mem_ptr += CLIENT_RECV_MEMORY_SIZE;
    get_send_memory_config(sh_mem_ptr, CLIENT_SEND_MEMORY_SIZE, &(sh_mem_cfg->send_mem_cfg));

    *(sh_mem_cfg->recv_mem_cfg.packet_ready) = PACKET_READY_FALSE;
    *(sh_mem_cfg->send_mem_cfg.packet_ready) = PACKET_READY_FALSE;
}


/* packet processing */
/* validate incoming packets */
void *receive_server_packets(void *arg) {
    /* process thread arguments */
    client_recv_thread_args *crta = (client_recv_thread_args *) arg;
    recv_memory_config *recv_mem_cfg = crta->recv_mem_cfg;
    int socket = crta->socket;

    /* initialize packet number counter for received packets */
    uint32_t recv_pn = 0;

    /* variables for code clarity */
    char *packet_ready = recv_mem_cfg->packet_ready;
    char *packet = recv_mem_cfg->packet;
    uint32_t *pn = recv_mem_cfg->pn;
    int32_t *psize = recv_mem_cfg->psize;

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
                    packet[i++] = '-';
                else {
                    sep_count++;
                    if (sep_count == PACKET_SEPARATOR_SIZE) {
                        /* convert packet header to host endianess */ 
                        *pn = big_endian_to_host_uint32_t(*pn);
                        *psize = big_endian_to_host_int32_t(*psize);

                        // printstr("packet received");
                        // print_bytes(packet, i);

                        /* verify packet */
                        *packet_ready = verify_packet(recv_pn, recv_mem_cfg, i - PACKET_HEADER_SIZE - PACKET_FOOTER_SIZE, packet[i - PACKET_CHECKSUM_SIZE]);

                        recv_pn++;
                        c = prevc = i = sep_count = 0;
                        continue;
                    }
                }
            }
            else if (c == '*')
                packet[i++] = (prevc == '?') ? '?' : '*';
            else {
                if (prevc == '?' || (sep_count > 0 && sep_count != PACKET_SEPARATOR_SIZE)) {
                    c = prevc = i = sep_count = 0;
                    continue;
                }
                if (c != '?')
                    packet[i++] = c;
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
    send_thread_args *sta = (send_thread_args *) arg;
    int socket = sta->socket;
    send_memory_config *send_mem_cfg = sta->send_mem_cfg;

    /* initialize packet number counter for sent packets */
    uint32_t send_pn = 0;

    /* allocate buffers for storing packets before they are sent (buffering) */
    char packet[PACKET_FROM_CLIENT_MAX_SIZE];
    char final_packet[2 * PACKET_FROM_CLIENT_MAX_SIZE]; /* assume that all characters in array "packet" could get encoded */ 

    while(1) {
        if (*(send_mem_cfg->packet_ready) == PACKET_READY_TRUE) {
            switch (*(send_mem_cfg->pid)) {
                case PACKET_JOIN_ID:
                    send_packet(send_pn++, PACKET_JOIN_DATA_SIZE, send_mem_cfg, packet, sizeof(packet), final_packet, sizeof(final_packet), socket);
                    break;
                case PACKET_MESSAGE_ID:
                    send_packet(send_pn++, PACKET_MESSAGE_DATA_SIZE, send_mem_cfg, packet, sizeof(packet), final_packet, sizeof(final_packet), socket);
                    break;
                case PACKET_PLAYER_READY_ID:
                    send_packet(send_pn++, PACKET_PLAYER_READY_DATA_SIZE, send_mem_cfg, packet, sizeof(packet), final_packet, sizeof(final_packet), socket);
                    break;
                case PACKET_PLAYER_INPUT_ID:
                    send_packet(send_pn++, PACKET_PLAYER_READY_DATA_SIZE, send_mem_cfg, packet, sizeof(packet), final_packet, sizeof(final_packet), socket);
                    break;
                case PACKET_CHECK_STATUS_ID:
                    send_packet(send_pn++, PACKET_CHECK_STATUS_DATA_SIZE, send_mem_cfg, packet, sizeof(packet), final_packet, sizeof(final_packet), socket);
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

/* process already validated packets */
void process_server_packets(recv_memory_config *recv_mem_cfg, send_memory_config *send_mem_cfg) {
    while(1) {
        if (*(recv_mem_cfg->packet_ready) == PACKET_READY_TRUE) {
            switch (*(recv_mem_cfg->pid)) {
                case PACKET_ACCEPT_ID:
                    process_accept(recv_mem_cfg->pdata, send_mem_cfg);
                    break;
                case PACKET_MESSAGE_ID:
                    process_message_from_server(recv_mem_cfg->pdata, send_mem_cfg);
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

void process_accept(char *data, send_memory_config *send_mem_cfg) {
    char status = *data;

    printf("RECEIVED ACCEPT, status=%d\n", status);
    send_message(1, 0, "Cau", send_mem_cfg);
}

void process_message_from_server(char *data, send_memory_config *send_mem_cfg) {
    char *data_ptr = data;
    char type = *data_ptr;
    data_ptr += 1;
    char source_id = *data_ptr;
    data_ptr += 1;
    char *message = data_ptr;

    printf("RECEIVED MESSAGE, type=%d, source_id=%d, message=%s\n", type, source_id, message);
    send_message(2, -1, "I received your message, mr. server", send_mem_cfg);
}

void process_lobby(char *data) {
    
}

void process_game_ready(char *data) {

}

void process_game_state(char *data) {

}

void process_game_end(char *data) {

}




