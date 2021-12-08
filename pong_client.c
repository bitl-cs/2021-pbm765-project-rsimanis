#include "pong_client.h"
#include "pong_networking.h"
#include "pong_game.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>


/* allocate shared memory for client */
void get_client_shared_memory(client_shared_memory_config *sh_mem_cfg) {
    char *sh_mem_ptr;
    
    sh_mem_ptr = (char *) malloc(CLIENT_SHARED_MEMORY_SIZE);
    sh_mem_cfg->shared_memory = sh_mem_ptr;
    get_recv_memory_config(sh_mem_ptr, &(sh_mem_cfg->recv_mem_cfg));
    sh_mem_ptr += CLIENT_RECV_MEMORY_SIZE;
    get_send_memory_config(sh_mem_ptr, &(sh_mem_cfg->send_mem_cfg));
}

/* validate incoming packets */
void *receive_server_packets(void *arg) {
    /* process thread arguments */
    client_thread_args *cta = (client_thread_args *) arg;
    client_shared_memory_config *sh_mem_cfg = cta->sh_mem_cfg;
    int client_socket = cta->client_socket;

    /* initialize packet number counter for received packets */
    uint32_t recv_pn = 0;

    /* variables for code clarity */
    recv_memory_config *recv_mem_cfg = &(sh_mem_cfg->recv_mem_cfg);
    packet_info *packet_info = &(recv_mem_cfg->packet_info);
    char *packet_ready = recv_mem_cfg->packet_ready;
    char *packet = packet_info->packet;
    uint32_t *pn = packet_info->pn;
    unsigned char *pid = packet_info->pid;
    int32_t *psize = packet_info->psize;
    char *pdata = packet_info->pdata;

    /* variables for underlying algorithm */
    char c = 0, prevc = 0;
    int encoded_size = 0, decoded_size = 0;
    char encoded_buf[2 * PACKET_FROM_SERVER_MAX_SIZE];
    char decoded_checksum;
    int len, i = 0;

    *packet_ready = 0;
    while (1) {
        if ((len = recv(client_socket, &c, sizeof(c), 0)) > 0) {
            if (c == '-') {
                if (prevc == '-') {
                    /* wait until another packet is processed */
                    while (*packet_ready) 
                        sleep(1.0/100);

                    /* decode packet, put the result in shared memory */
                    decoded_size = decode(encoded_buf, encoded_size, packet, PACKET_FROM_SERVER_MAX_SIZE);

                    /* extract checksum from packet's footer */
                    decoded_checksum = packet[decoded_size - PACKET_CHECKSUM_SIZE];

                    /* convert to host endianess */ 
                    *pn = big_endian_to_host_uint32_t(*pn);
                    *psize = big_endian_to_host_int32_t(*psize);

                    /* verify packet */
                    *packet_ready = verify_packet(recv_pn, packet_info, decoded_size - PACKET_HEADER_SIZE - PACKET_FOOTER_SIZE, decoded_checksum);

                    recv_pn++; /* TODO: need more checks if recv_pn overflows */
                    encoded_size = i = prevc = 0;
                    continue;
                }
            }
            else {
                if (prevc == '-') {
                    encoded_buf[i++] = prevc;
                    encoded_size++;
                }
                encoded_buf[i++] = c;
                encoded_size++;
            }
            prevc = c;
        }
        else if (len == 0) {
            printf("Server disconnected\n");
            exit(0);
        }
        else {
            printf("reading error\n");
            exit(-1);
        }
    }
    return NULL;
}

/* process already validated packets */
void process_server_packets(recv_memory_config *recv_mem_cfg) {
    /* variables for code clarity */
    packet_info *packet_info = &(recv_mem_cfg->packet_info);
    char *packet_ready = recv_mem_cfg->packet_ready;
    char *packet = packet_info->packet;
    uint32_t *pn = packet_info->pn;
    unsigned char *pid = packet_info->pid;
    int32_t *psize = packet_info->psize;
    char *pdata = packet_info->pdata;

    while(1) {
        if (*packet_ready) {
            switch (*pid) {
                case 2:
                    process_accept(pdata);
                    break;
                case 3:
                    process_message(pdata);
                    break;
                case 4:
                    process_lobby(pdata);
                    break;
                case 5:
                    process_game_ready(pdata);
                    break;
                case 7:
                    process_game_state(pdata);
                    break;
                case 10:
                    process_game_end(pdata);
                    break;
                default:
                    printf("Invalid pid (%u)\n", (unsigned) *pid);
            }
            *packet_ready = 0;
        }
        else
            sleep(1.0/100);
    }
}

/* send packets to server */
void *send_server_packets(void *arg) {
    /* process thread arguments */
    client_thread_args *cta = (client_thread_args *) arg;
    client_shared_memory_config *sh_mem_cfg = cta->sh_mem_cfg;
    int client_socket = cta->client_socket;

    /* initialize packet number counter for sent packets */
    uint32_t send_pn = 0;

    /* variables for code clarity */
    send_memory_config *send_mem_cfg = &(sh_mem_cfg->send_mem_cfg);
    char *packet_ready = send_mem_cfg->packet_ready;
    unsigned char *pid = send_mem_cfg->pid;
    int32_t *packet_data_size = send_mem_cfg->packet_data_size;
    char *pdata = send_mem_cfg->pdata;

    *packet_ready = 0;
    while(1) {
        if (*packet_ready) {
            switch (*pid) {
                case 1:
                    send_packet(send_pn++, *pid, PACKET_JOIN_DATA_SIZE, pdata, *packet_data_size, client_socket);
                    break;
                case 3:
                    send_packet(send_pn++, *pid, PACKET_MESSAGE_DATA_SIZE, pdata, *packet_data_size, client_socket);
                    break;
                case 6:
                    send_packet(send_pn++, *pid, PACKET_PLAYER_READY_DATA_SIZE, pdata, *packet_data_size, client_socket);
                    break;
                case 8:
                    send_packet(send_pn++, *pid, PACKET_PLAYER_INPUT_DATA_SIZE, pdata, *packet_data_size, client_socket);
                    break;
                case 9:
                    send_packet(send_pn++, *pid, PACKET_CHECK_STATUS_DATA_SIZE, pdata, *packet_data_size, client_socket);
                    break;
                default:
                    printf("Invalid pid (%u)\n", (unsigned) *pid);
            }
            *packet_ready = 0;
        }
        else
            sleep(1.0/100);
    }

    return NULL;
}