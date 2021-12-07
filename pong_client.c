#include "pong_client.h"
#include "pong_networking.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>


/* allocate shared memory for client */
void get_client_shared_memory(client_shared_memory_config *sh_mem_cfg) {
    char *sh_mem_ptr;
    
    sh_mem_ptr = (char *) malloc(SHARED_MEMORY_SIZE);
    sh_mem_cfg->shared_memory = sh_mem_ptr;
    sh_mem_cfg->recv_packet_ready = sh_mem_ptr;
    sh_mem_ptr += SHARED_PACKET_READY_SIZE;
    sh_mem_cfg->recv_packet_buf = sh_mem_ptr;
    sh_mem_ptr += SERVER_PACKET_MAX_SIZE;
    sh_mem_cfg->send_packet_ready = sh_mem_ptr;
    sh_mem_ptr += SHARED_PACKET_READY_SIZE;
    sh_mem_cfg->send_packet_id = (unsigned char *) sh_mem_ptr;
    sh_mem_ptr += PACKET_ID_SIZE;
    sh_mem_cfg->send_packet_data = sh_mem_ptr;
    get_packet_info(sh_mem_cfg->recv_packet_buf, &sh_mem_cfg->recv_packet_info);
}

/* receive and validate packets */
void *receive_server_packets(void *arg) {
    client_thread_args *cta = (client_thread_args *) arg;
    client_shared_memory_config *sh_mem_cfg = cta->sh_mem_cfg;
    int client_socket = cta->client_socket;

    char recv_buf[2 * SERVER_PACKET_MAX_SIZE];
    uint32_t recv_pn = -1;

    char c = 0, prevc = 0;
    int encoded_size = 0, decoded_size = 0;
    int len, i = 0;

    while (1) {
        if ((len = recv(client_socket, &c, sizeof(c), 0)) > 0) {
            if (c == '-') {
                if (prevc == '-') {
                    recv_pn++; /* TODO: need more checks if recv_pn overflows */

                    /* wait until another packet is processed */
                    while (*(sh_mem_cfg->recv_packet_ready)) 
                        sleep(1.0/100);

                    /* decode packet, put the result in shared memory */
                    decoded_size = decode(recv_buf, encoded_size, (sh_mem_cfg->recv_packet_buf), CLIENT_PACKET_MAX_SIZE);

                    /* convert to host endianess */ 
                    *(sh_mem_cfg->recv_packet_info.number) = network_to_host_uint32_t(*(sh_mem_cfg->recv_packet_info.number));
                    *(sh_mem_cfg->recv_packet_info.size) = network_to_host_uint32_t(*(sh_mem_cfg->recv_packet_info.size));

                    /* verify packet */
                    *((sh_mem_cfg->recv_packet_ready)) = verify_packet((sh_mem_cfg->recv_packet_buf), recv_pn, decoded_size); 

                    encoded_size = i = prevc = 0;
                    continue;
                }
            }
            else {
                if (prevc == '-') {
                    recv_buf[i++] = prevc;
                    encoded_size++;
                }
                recv_buf[i++] = c;
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
            exit(1);
        }
    }
    return NULL;
}

/* process already validated packets */
void process_server_packets(client_shared_memory_config *sh_mem_cfg) {
    while(1) {
        if (*(sh_mem_cfg->recv_packet_ready)) {
            switch (*(sh_mem_cfg->recv_packet_info.id)) {
                case 2:
                    process_lobby(sh_mem_cfg->recv_packet_info.data);
                    break;
                case 4:
                    process_player_queue(sh_mem_cfg->recv_packet_info.data);
                    break;
                case 5:
                    process_game_ready(sh_mem_cfg->recv_packet_info.data);
                    break;
                case 7:
                    process_game_state(sh_mem_cfg->recv_packet_info.data);
                    break;
                case 10:
                    process_game_end(sh_mem_cfg->recv_packet_info.data);
                    break;
                default:
                    printf("Invalid pid (%d)\n", *(sh_mem_cfg->recv_packet_info.id));
            }
            *(sh_mem_cfg->recv_packet_ready) = 0;
        }
        else
            sleep(1.0/100);
    }
}

/* send packets to server */
void *send_server_packets(void *arg) {
    client_thread_args *cta = (client_thread_args *) arg;
    client_shared_memory_config *sh_mem_cfg = cta->sh_mem_cfg;
    int client_socket = cta->client_socket;

    char send_buf[2 * CLIENT_PACKET_MAX_SIZE + PACKET_SEPARATOR_SIZE];
    uint32_t send_pn = 0;

    while(1) {
        if (*(sh_mem_cfg->send_packet_ready)) {
            switch (*(sh_mem_cfg->send_packet_id)) {
                case 1:
                    /* send join */
                    /* send_join(char *data, int socket); */
                    /* data is in correct order, but NOT in network byte order */
                    /* send_join() function MUST ALSO CONVERT DATA TO NETWORK BYTE ORDER */
                    /* send_join(sh_mem_cfg->send_packet_data, client_socket); */
                    break;
                case 3:
                    break;
                case 6:
                    break;
                case 8:
                    break;
                case 9:
                    break;
                default:
                    printf("Invalid pid (%d)\n", *(sh_mem_cfg->send_packet_id));
            }
            *(sh_mem_cfg->send_packet_ready) = 0;
        }
        else
            sleep(1.0/100);
    }

    return NULL;
}