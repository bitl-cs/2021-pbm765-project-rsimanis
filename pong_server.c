#include "pong_server.h"
#include "pong_game.h"
#include "pong_networking.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/mman.h>


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
    client_data cl_data;
    get_client_data(id, sh_mem_cfg, &cl_data);

    /* initialize packet receiving thread */
    if (init_recv_thread(socket, &(cl_data.recv_mem_cfg)) < 0) {
        printf(" Fatal ERROR: Could not create packet receiving thread (id=%d) - exiting...\n", id);
        exit(-1);
    }

    /* initialize packet sending thread */
    if (init_send_thread(socket, &(cl_data.send_mem_cfg), send_server_packets) < 0) {
        printf(" Fatal ERROR: Could not create packet sending thread (id=%d)- exiting...\n", id);
        exit(-1);
    }

    /* process already validated incoming packets */
    process_client_packets(&(cl_data.recv_mem_cfg));
}


int find_free_client_id(server_shared_memory_config *sh_mem_cfg) {
    int id;

    for (id = 0; id < MAX_CLIENTS; id++) {
        if (*(get_client_data_ptr(id, sh_mem_cfg)) == 0) /* first byte represents if the slot is taken */ 
            return id;
    }
    return -1;
}

void remove_client(int id, int socket, server_shared_memory_config *sh_mem_cfg) {
    *(get_client_data_ptr(id, sh_mem_cfg)) = 0;
    (*(sh_mem_cfg->client_count))--;
    close(socket);
}

char *get_client_data_ptr(int id, server_shared_memory_config *sh_mem_cfg) {
    return sh_mem_cfg->shared_client_data + SERVER_SHARED_CLIENT_DATA_SIZE * id;
}

void get_client_data(int id, server_shared_memory_config *sh_mem_cfg, client_data *cd) {
    char *cd_ptr;

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
void *send_server_packets(void *arg) {
    /* process thread arguments */
    send_thread_args *sta = (send_thread_args *) arg;
    int socket = sta->socket;
    send_memory_config *send_mem_cfg = sta->send_mem_cfg;

    /* initialize packet number counter for sent packets */
    uint32_t send_pn = 0;

    *(send_mem_cfg->packet_ready) = PACKET_READY_FALSE;
    while(1) {
        if (*(send_mem_cfg->packet_ready) == PACKET_READY_TRUE) {
            if (*(send_mem_cfg->pid) == PACKET_ACCEPT_ID)
                send_packet(send_pn++, PACKET_ACCEPT_DATA_SIZE, send_mem_cfg, socket);
            else if (*(send_mem_cfg->pid) == PACKET_MESSAGE_ID)
                send_packet(send_pn++, PACKET_MESSAGE_DATA_SIZE, send_mem_cfg, socket);
            else if (*(send_mem_cfg->pid) == PACKET_LOBBY_ID ||
                        *(send_mem_cfg->pid) == PACKET_GAME_READY_ID ||
                        *(send_mem_cfg->pid) == PACKET_GAME_STATE_ID ||
                        *(send_mem_cfg->pid) == PACKET_GAME_END_ID)
                send_packet(send_pn++, *(send_mem_cfg->datalen), send_mem_cfg, socket);
            else
                printf("Invalid pid (%u)\n", (unsigned) *(send_mem_cfg->pid));
            *(send_mem_cfg->packet_ready) = PACKET_READY_FALSE;
        }
        else
            sleep(PACKET_READY_WAIT_TIME);
    }

    return NULL;
}

void process_client_packets(recv_memory_config *recv_mem_cfg) {
    while (1) {
        if (*(recv_mem_cfg->packet_ready) == PACKET_READY_TRUE) {
            switch (*(recv_mem_cfg->pid)) {
                case PACKET_JOIN_ID:
                    process_join(recv_mem_cfg->pdata);
                    break;
                case PACKET_MESSAGE_ID:
                    process_message_from_client(recv_mem_cfg->pdata);
                    break;
                case PACKET_PLAYER_READY_ID:
                    process_player_ready();
                    break;
                case PACKET_PLAYER_INPUT_ID:
                    process_player_input(recv_mem_cfg->pdata);
                    break;
                case PACKET_CHECK_STATUS_ID:
                    process_check_status();
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

void process_join(char *data) {
    char *name = data;
    printf("RECEIVED JOIN, NAME = %s", name);
    /* if state == 0 */
        /* add username to client_data */ 
        /* change state */
        /* send accept packet as response */
}

void process_message_from_client(char *data) {

}

void process_player_ready(void) {

}

void process_player_input(char *data) {

}

void process_check_status(void) {

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