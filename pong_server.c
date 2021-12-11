#include "pong_server.h"
#include "pong_game.h"
#include "pong_networking.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/mman.h>
#include <pthread.h>


/* init */
void get_shared_memory(server_shared_memory_config* sh_mem_cfg) {
    char *sh_mem_ptr;
    int id;

    sh_mem_ptr = (char *) mmap(NULL, SERVER_SHARED_MEMORY_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    sh_mem_cfg->shared_memory = sh_mem_ptr;
    sh_mem_cfg->client_count = (unsigned char *) sh_mem_ptr;
    sh_mem_ptr += 1;
    sh_mem_cfg->shared_client_data = sh_mem_ptr;
    sh_mem_ptr += SERVER_SHARED_CLIENT_DATA_SIZE * MAX_CLIENTS;
    sh_mem_cfg->shared_gamestate_data = sh_mem_ptr;

    /* set all client slots as not-taken */
    for (id = 0; id < MAX_CLIENTS; id++)
        *(get_client_data_ptr(id, sh_mem_cfg)) = 0; /* first byte represents if the slot is taken */
}

void start_network(char *port, server_shared_memory_config *sh_mem_cfg) {
    int server_socket;

    server_socket = -1;
    printf("Starting the network...");
    server_socket = get_server_socket(port);
    if (server_socket == -1) {
        printf("  Fatal ERROR: Could not open server socket - exiting...\n");
        exit(-1);
    }
    printf("  Starting accepting clients...\n");
    accept_clients(server_socket, sh_mem_cfg);
    printf(" Stopped accepting clients (Should not happen!) - server network stopped!\n");
}


/* game */
void gameloop(server_shared_memory_config *sh_mem_cfg) {
    printf("Starting game loop! (It will run forever - use Ctrl+C)\n");
    while (1) {
        // loop forever
    }
}


/* client processing */
void accept_clients(int server_socket, server_shared_memory_config *sh_mem_cfg) {
    int new_client_id = 0;
    int tid = 0;
    int client_socket = -1;
    *(sh_mem_cfg->client_count) = 0;

    while(1){
        client_socket = accept(server_socket, NULL, NULL);
        if(client_socket<0){
            printf("  Soft ERROR accepting the client connection! ERRNO=%d Continuing...\n", errno);
            continue;
            /* WE CAN accept other clients if this fails to connect so continue! */
        }

        /* New client */
        new_client_id = find_free_client_id(sh_mem_cfg);
        if (new_client_id == -1) {
            printf("Limit for maximum clients is reached\n");
            close(client_socket);
            continue;
        }
        *(sh_mem_cfg->client_count) += 1;
        *get_client_data_ptr(new_client_id, sh_mem_cfg) = 1;

        /* We have a client connection - doublefork&orphan to process it, main thread closes socket and listens for a new one */
        tid = fork();
        if(tid == 0){
            /* child - will double fork & orphan */
            close(server_socket);
            tid = fork();
            if(tid == 0){
                /* NOTE: ideally You would check if clients disconnect and reduce client_count to allow new connections, this is not yet implemented */
                process_client(new_client_id, client_socket, sh_mem_cfg);
                exit(0);
            }
            else {
                /* orphaning */
                wait(NULL);
                printf("Successfully orphaned client %d\n", new_client_id);
                exit(0);
            }
        }
        else {
            /* parent - go to listen to the next client connection */
            close(client_socket);
        }
    }
}

void process_client(int id, int socket, server_shared_memory_config* sh_mem_cfg) {
    printf("CLIENT connected (client_count=%d, client_id=%d, socket=%d)\n", *(sh_mem_cfg->client_count), id, socket);
    client_data cd;
    get_client_data(id, socket, sh_mem_cfg, &cd);

    /* set packet_ready for both buffers to false */
    *(cd.recv_mem_cfg.packet_ready) = PACKET_READY_FALSE;
    *(cd.send_mem_cfg.packet_ready) = PACKET_READY_FALSE;

    /* initialize packet receiving thread */
    pthread_t receiving_thread_id;
    server_recv_thread_args srta;
    srta.cd = &cd;
    srta.sh_mem_cfg = sh_mem_cfg;
    if (pthread_create(&receiving_thread_id, NULL, receive_client_packets, (void *) &srta) != 0)
        exit(-1);
    sleep(THREAD_INIT_WAIT_TIME); /* wait until thread's local variables from its argument (recv_thread_args) are initialized */

    /* initialize packet sending thread */
    pthread_t sending_thread_id;
    send_thread_args sta;
    sta.socket = socket;
    sta.send_mem_cfg = &cd.send_mem_cfg;
    if (pthread_create(&sending_thread_id, NULL, send_server_packets, (void *) &sta) != 0)
        exit(-1);
    sleep(THREAD_INIT_WAIT_TIME); /* wait until thread's local variables from its argument (recv_thread_args) are initialized */

    /* process already validated incoming packets */
    process_client_packets(&cd);
}


int find_free_client_id(server_shared_memory_config *sh_mem_cfg) {
    int id;

    for (id = 0; id < MAX_CLIENTS; id++) {
        if (*(get_client_data_ptr(id, sh_mem_cfg)) == 0) /* first byte represents if the slot is taken */ 
            return id;
    }
    return -1;
}

void remove_client(client_data *cd, server_shared_memory_config *sh_mem_cfg) {
    *(cd->taken) = 0;
    (*(sh_mem_cfg->client_count))--;
    close(cd->socket);
}

char *get_client_data_ptr(int id, server_shared_memory_config *sh_mem_cfg) {
    return sh_mem_cfg->shared_client_data + SERVER_SHARED_CLIENT_DATA_SIZE * id;
}

void get_client_data(int id, int socket, server_shared_memory_config *sh_mem_cfg, client_data *cd) {
    char *cd_ptr;

    cd->id = id;
    cd->socket = socket;
    cd_ptr = get_client_data_ptr(id, sh_mem_cfg);
    cd->client_data = cd_ptr;
    cd->taken = (unsigned char *) cd_ptr;
    cd_ptr += CLIENT_TAKEN_SIZE;
    cd->name = cd_ptr;
    cd_ptr += MAX_NAME_SIZE;
    cd->input = cd_ptr; 
    cd_ptr += INPUT_SIZE; 
    cd->state = cd_ptr;
    cd_ptr += CLIENT_STATE_SIZE;
    get_recv_memory_config(cd_ptr, SERVER_RECV_MEMORY_SIZE, &(cd->recv_mem_cfg));
    cd_ptr += SERVER_RECV_MEMORY_SIZE;
    get_send_memory_config(cd_ptr, SERVER_SEND_MEMORY_SIZE, &(cd->send_mem_cfg));
}

/* packet processing */
/* validate incoming packets */
void *receive_client_packets(void *arg) {
    /* process thread arguments */
    server_recv_thread_args *srta = (server_recv_thread_args *) arg;
    client_data *cd = srta->cd;
    server_shared_memory_config *sh_mem_cfg = srta->sh_mem_cfg;
    recv_memory_config *recv_mem_cfg = &(cd->recv_mem_cfg);

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
        if ((len = recv(cd->socket, &c, 1, 0)) > 0) {
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
            printf("Client disconnected (client_id=%d, socket=%d)\n", cd->id, cd->socket);
            remove_client(cd, sh_mem_cfg);
            exit(0);
        }
        else {
            printf("Recv() error (errno=%d)\n", errno);
            printf("Removing client... (client_id=%d, socket=%d)\n", cd->id, cd->socket);
            remove_client(cd, sh_mem_cfg);
            exit(-1);
        }
    }
    return NULL;
}

void *send_server_packets(void *arg) {
    /* process thread arguments */
    send_thread_args *sta = (send_thread_args *) arg;
    int socket = sta->socket;
    send_memory_config *send_mem_cfg = sta->send_mem_cfg;

    /* initialize packet number counter for sent packets */
    uint32_t send_pn = 0;

    /* allocate buffers for storing packets before they are sent (buffering) */
    char packet[PACKET_FROM_SERVER_MAX_SIZE];
    char final_packet[2 * PACKET_FROM_SERVER_MAX_SIZE]; /* assume that all characters in array "packet" could get encoded */ 

    while(1) {
        if (*(send_mem_cfg->packet_ready) == PACKET_READY_TRUE) {
            if (*(send_mem_cfg->pid) == PACKET_ACCEPT_ID)
                send_packet(send_pn++, PACKET_ACCEPT_DATA_SIZE, send_mem_cfg, packet, sizeof(packet), final_packet, sizeof(final_packet), socket);
            else if (*(send_mem_cfg->pid) == PACKET_MESSAGE_ID)
                send_packet(send_pn++, PACKET_MESSAGE_DATA_SIZE, send_mem_cfg, packet, sizeof(packet), final_packet, sizeof(final_packet), socket);
            else if (*(send_mem_cfg->pid) == PACKET_LOBBY_ID ||
                        *(send_mem_cfg->pid) == PACKET_GAME_READY_ID ||
                        *(send_mem_cfg->pid) == PACKET_GAME_STATE_ID ||
                        *(send_mem_cfg->pid) == PACKET_GAME_END_ID)
                send_packet(send_pn++, *(send_mem_cfg->datalen), send_mem_cfg, packet, sizeof(packet), final_packet, sizeof(final_packet), socket);
            else
                printf("Invalid pid (%u)\n", (unsigned) *(send_mem_cfg->pid));
            *(send_mem_cfg->packet_ready) = PACKET_READY_FALSE;
        }
        else
            sleep(PACKET_READY_WAIT_TIME);
    }

    return NULL;
}

void process_client_packets(client_data *cd) {
    recv_memory_config *recv_mem_cfg = &(cd->recv_mem_cfg);

    while (1) {
        if (*(recv_mem_cfg->packet_ready) == PACKET_READY_TRUE) {
            switch (*(recv_mem_cfg->pid)) {
                case PACKET_JOIN_ID:
                    process_join(recv_mem_cfg->pdata, cd);
                    break;
                case PACKET_MESSAGE_ID:
                    process_message_from_client(recv_mem_cfg->pdata, cd);
                    break;
                case PACKET_PLAYER_READY_ID:
                    process_player_ready(cd);
                    break;
                case PACKET_PLAYER_INPUT_ID:
                    process_player_input(recv_mem_cfg->pdata, cd);
                    break;
                case PACKET_CHECK_STATUS_ID:
                    process_check_status(cd);
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

void process_join(char *data, client_data *cd) {
    char *name = data;

    printf("RECEIVED JOIN, NAME = %s\n", name);
    send_accept(1, &cd->send_mem_cfg);
    /* if state == 0 */
        /* add username to client_data */ 
        /* change state */
        /* send accept packet as response */
}

void process_message_from_client(char *data, client_data *cd) {
    char *data_ptr = data;
    char type = *data_ptr;
    data_ptr += 1;
    char source_id = *data_ptr;
    data_ptr += 1;
    char *message = data_ptr;

    printf("Received MESSAGE from client (id=%d, socket=%d), type=%d, source_id=%d, message=%s\n", cd->id, cd->socket, type, source_id, message);
    send_message(0, -1, "to all", &cd->send_mem_cfg);
}

void process_player_ready(client_data *cd) {

}

void process_player_input(char *data, client_data *cd) {

}

void process_check_status(client_data *cd) {

}

/* debug */
void print_client_data(client_data *cd) {
    printf("TAKEN: %c\n", *(cd->taken));
    printf("NAME %s\n", cd->name);
    printf("INPUT: %c\n", *(cd->input));
    printf("STATE: %c\n", *(cd->state));
    print_recv_memory(&(cd->recv_mem_cfg));
    print_send_memory(&(cd->send_mem_cfg));
}

void print_shared_memory(server_shared_memory_config* sh_mem_cfg) {
    int id;
    client_data cd;

    /* print client count */
    printf("Client count: ");
    print_bytes(sh_mem_cfg->client_count, 1);
    putchar('\n');

    /* print client datas */
    for (id = 0; id < MAX_CLIENTS; id++) {
        print_client_data(&cd);
        putchar('\n');
    }
    putchar('\n');

    /* TODO: print game states */
}