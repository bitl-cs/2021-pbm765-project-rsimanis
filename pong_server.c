#include "pong_networking.h"
#include "pong_server.h"
#include "pong_game.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>


void get_shared_memory(shared_memory_config* sh_mem_cfg) {
    char *sh_mem_ptr;
    int id;

    sh_mem_ptr = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    sh_mem_cfg->shared_memory = sh_mem_ptr;
    sh_mem_cfg->client_count = (unsigned char *) sh_mem_ptr;
    sh_mem_ptr += SHARED_CLIENT_COUNT_SIZE;
    sh_mem_cfg->shared_client_data = sh_mem_ptr;
    sh_mem_ptr += SHARED_CLIENT_DATA_SIZE * MAX_CLIENTS;
    sh_mem_cfg->shared_gamestate_data = sh_mem_ptr;

    /* set all client slots as not-taken */
    for (id = 0; id < MAX_CLIENTS; id++)
        *(get_client_data_ptr(id, sh_mem_cfg)) = 0; /* first byte represents if the slot is taken */
}

void gameloop(shared_memory_config *sh_mem_cfg) {
    printf("Starting game loop! (It will run forever - use Ctrl+C)\n");
    while (1) {
        // loop forever
    }
}

void start_network(char *port, shared_memory_config *sh_mem_cfg) {
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

void accept_clients(int server_socket, shared_memory_config *sh_mem_cfg) {
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
        client_data cd = get_client_data(new_client_id, sh_mem_cfg);
        *(cd.taken) = 1;

        /* We have a client connection - doublefork&orphan to process it, main thread closes socket and listens for a new one */
        tid = fork();
        if(tid == 0){
            /* child - will double fork & orphan */
            close(server_socket);
            tid = fork();
            if(tid == 0){
                /* NOTE: ideally You would check if clients disconnect and reduce client_count to allow new connections, this is not yet implemented */
                process_client(new_client_id, client_socket, sh_mem_cfg, cd);
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

void process_client(int id, int socket, shared_memory_config* sh_mem_cfg, client_data client_data) {
    printf("Processing client id=%d, socket=%d\n", id, socket);
    // printf("CLIENT count %d\n", *(sh_mem_cfg->client_count));

    int32_t *pn = (int32_t *) (client_data.packet_buf);
    int32_t *pid = (int32_t *) (client_data.packet_buf + PACKET_NUMBER_SIZE);
    int32_t *psize = (int32_t *) (client_data.packet_buf + PACKET_NUMBER_SIZE + PACKET_ID_SIZE);

    char recv_buf[2 * CLIENT_PACKET_MAX_SIZE];
    int recv_pn = -1, send_pn = -1;

    char c = 0, prevc = 0;
    long encoded_size = 0, decoded_size = 0;
    int len, i = 0;

    *(client_data.packet_ready) = 0;

    /* create a thread for validated packet processing */
    proc_inc_packets_args pra;
    pra.id = id;
    pra.socket = socket;
    pra.send_pn = send_pn;
    pra.client_data = &client_data;
    pthread_t processing_thread_id;
    if (pthread_create(&processing_thread_id, NULL, process_incoming_packets, (void *) &pra) != 0) {
        printf("Error creating thread\n");
        remove_client(id, socket, sh_mem_cfg);
        exit(1);
    }

    /* receive and validate incoming packets */
    while (1) {
        if ((len = recv(socket, &c, 1, 0)) > 0) {
            if (c == '-') {
                if (prevc == '-') {
                    recv_pn++; /* TODO: need more checks if recv_pn overflows */

                    while (*(client_data.packet_ready) == 1) /* another packet is being processed */
                        sleep(1.0/100);

                    decoded_size = decode(recv_buf, encoded_size, client_data.packet_buf, CLIENT_PACKET_MAX_SIZE); /* decode packet, put the result in shared memory */

                    /* convert to host endianess */ 
                    *pn = network_to_host_int32_t(*pn);
                    *pid = network_to_host_int32_t(*pid);
                    *psize = network_to_host_int32_t(*psize);

                    *(client_data.packet_ready) = verify_packet(client_data.packet_buf, &recv_pn, decoded_size); /* verify packet */

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
            printf("Client id=%d disconnected\n", id);
            remove_client(id, socket, sh_mem_cfg);
            exit(0);
        }
        else {
            printf("Error receiving data from client id=%d", id);
            remove_client(id, socket, sh_mem_cfg);
            exit(-1);
        }
    }
}

void *process_incoming_packets(void *arg) {
    /* process arguments */
    proc_inc_packets_args *args = (proc_inc_packets_args *) arg;
    int id = args->id;
    int socket = args->socket;
    int send_pn = args->send_pn;
    client_data *client_data = args->client_data;

    int *pid = (int *) (client_data->packet_buf + PACKET_NUMBER_SIZE);   /* pointer to packet id */
    char *data = client_data->packet_buf + PACKET_HEADER_SIZE;           /* pointer to data segment */

    while(1) {
        if (*(client_data->packet_ready)) {
            switch (*pid) {
                case 1:
                    process_join(data);
                    // print_bytes(shared_recv_buf, JOIN_PACKET_SIZE);
                    send_lobby(0, NULL, ++send_pn, socket);
                    send_lobby(1, "bla bla", ++send_pn, socket);
                    // send_lobby(1, "super gars errrrrrors", ++send_pn, socket);
                    // send_lobby(1, "super gars errrrrrors", ++send_pn, socket);
                    // send_lobby(1, "super gars errrrrrors", ++send_pn, socket);
                    // send_lobby(1, "super gars errrrrrors", ++send_pn, socket);
                    // send_lobby(1, "super gars errrrrrors", ++send_pn, socket);
                    // send_lobby(1, "super gars errrrrrors", ++send_pn, socket);
                    // send_lobby(1, "super gars errrrrrors", ++send_pn, socket);
                    break;
                case 3:
                    // print_bytes(shared_recv_buf, GAME_TYPE_PACKET_SIZE);
                    process_game_type(data);
                    // print_bytes(shared_recv_buf, GAME_TYPE_PACKET_SIZE);
                    // send_player_queue(1, "XD", ++send_pn, socket);
                    break;
                case 6:
                    process_player_ready();
                    // print_bytes(shared_recv_buf, PLAYER_READY_PACKET_SIZE);
                    // send_game_ready(1, "ERROR: SLIKTI!", ++send_pn, socket);
                    break;
                case 8:
                    process_player_input(data);
                    // print_bytes(shared_recv_buf, PLAYER_INPUT_PACKET_SIZE);
                    break;
                case 9:
                    process_check_status();
                    break;
                default:
                    printf("Invalid pid (%d)\n", *pid);
            }
            *(client_data->packet_ready) = 0;
        }
        sleep(1.0/100);
    }
}

int find_free_client_id(shared_memory_config *sh_mem_cfg) {
    int id;

    for (id = 0; id < MAX_CLIENTS; id++) {
        if (*(get_client_data_ptr(id, sh_mem_cfg)) == 0) /* first byte represents if the slot is taken */ 
            return id;
    }
    return -1;
}

void remove_client(int id, int socket, shared_memory_config *sh_mem_cfg) {
    *(get_client_data_ptr(id, sh_mem_cfg)) = 0;
    (*(sh_mem_cfg->client_count))--;
    close(socket);
}

char *get_client_data_ptr(int id, shared_memory_config *sh_mem_cfg) {
    return sh_mem_cfg->shared_client_data + SHARED_CLIENT_DATA_SIZE * id;
}

client_data get_client_data(int id, shared_memory_config *sh_mem_cfg) {
    client_data rez;
    char *client_data_ptr;

    client_data_ptr = get_client_data_ptr(id, sh_mem_cfg);
    rez.client_data = client_data_ptr;
    rez.taken = (unsigned char *) client_data_ptr;
    client_data_ptr += SHARED_CLIENT_TAKEN_SIZE;
    rez.name = client_data_ptr;
    client_data_ptr += MAX_NAME_SIZE;
    rez.input = client_data_ptr; 
    client_data_ptr += INPUT_SIZE; 
    rez.packet_ready = client_data_ptr;
    client_data_ptr += SHARED_CLIENT_PACKET_READY_SIZE;
    rez.packet_buf = client_data_ptr;
    return rez;
}

void print_shared_memory(shared_memory_config* sh_mem_cfg) {
    int id;
    client_data cd;

    printf("Client count: ");
    print_bytes(sh_mem_cfg->client_count, SHARED_CLIENT_COUNT_SIZE);

    for (id = 0; id < MAX_CLIENTS; id++) {
        cd = get_client_data(id, sh_mem_cfg);
        printf("Taken: ");
        print_bytes(cd.taken, SHARED_CLIENT_TAKEN_SIZE);
        printf("Name: ");
        print_bytes(cd.name, MAX_NAME_SIZE);
        printf("Input: ");
        print_bytes(cd.input, INPUT_SIZE);
        printf("Packet ready: ");
        print_bytes(cd.packet_ready, SHARED_CLIENT_PACKET_READY_SIZE);
        printf("Packet buffer: ");
        print_bytes(cd.packet_buf, CLIENT_PACKET_MAX_SIZE);
    }
}