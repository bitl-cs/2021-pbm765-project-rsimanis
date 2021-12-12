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
server_shared_memory *get_server_shared_memory() {
    return mmap(NULL, sizeof(server_shared_memory), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
}

void start_network(char *port, server_shared_memory *sh_mem) {
    int server_socket;

    server_socket = -1;
    printf("Starting the network...");
    server_socket = get_server_socket(port);
    if (server_socket == -1) {
        printf("  Fatal ERROR: Could not open server socket - exiting...\n");
        exit(-1);
    }
    printf("  Starting accepting clients...\n");
    accept_clients(server_socket, sh_mem);
    printf(" Stopped accepting clients (Should not happen!) - server network stopped!\n");
}


/* game */
void gameloop(server_shared_memory *sh_mem) {
    printf("Starting game loop! (It will run forever - use Ctrl+C)\n");
    while (1) {
        // loop forever
    }
}


/* client processing */
void accept_clients(int server_socket, server_shared_memory *sh_mem) {
    int tid = 0;
    int client_socket = -1;
    char i;

    sh_mem->client_count = 0;
    for (i = 0; i < MAX_CLIENTS; i++)
        sh_mem->clients[i].id = -1;

    while (1) {
        client_socket = accept(server_socket, NULL, NULL);
        if (client_socket<0) {
            printf("  Soft ERROR accepting the client connection! ERRNO=%d Continuing...\n", errno);
            continue;
            /* WE CAN accept other clients if this fails to connect so continue! */
        }

        /* New client */
        client *new_client = init_client(sh_mem, client_socket);
        if (new_client == NULL) {
            printf("Limit for maximum clients is reached\n");
            close(client_socket);
            continue;
        }

        /* We have a client connection - doublefork&orphan to process it, main thread closes socket and listens for a new one */
        tid = fork();
        if (tid == 0){
            /* child - will double fork & orphan */
            close(server_socket);
            tid = fork();
            if (tid == 0){
                /* NOTE: ideally You would check if clients disconnect and reduce client_count to allow new connections, this is not yet implemented */
                process_client(new_client, sh_mem);
                exit(0);
            }
            else {
                /* orphaning */
                wait(NULL);
                printf("Successfully orphaned client %d\n", new_client->id);
                exit(0);
            }
        }
        else {
            /* parent - go to listen to the next client connection */
            close(client_socket);
        }
    }
}

void process_client(client *client, server_shared_memory* sh_mem) {
    printf("CLIENT connected (client_count=%d, client_id=%d, socket=%d)\n", sh_mem->client_count, client->id, client->socket);

    /* initialize thread arguments */
    server_recv_send_thread_args srsta;
    srsta.client = client;
    srsta.sh_mem = sh_mem;

    /* initialize packet receiving thread */
    pthread_t receiving_thread_id;
    if (pthread_create(&receiving_thread_id, NULL, receive_client_packets, (void *) &srsta) != 0)
        exit(-1);
    sleep(THREAD_INIT_WAIT_TIME); /* wait until thread's local variables from its argument (recv_thread_args) are initialized */

    /* initialize packet sending thread */
    pthread_t sending_thread_id;
    if (pthread_create(&sending_thread_id, NULL, send_server_packets, (void *) &srsta) != 0)
        exit(-1);
    sleep(THREAD_INIT_WAIT_TIME); /* wait until thread's local variables from its argument (recv_thread_args) are initialized */

    /* process already validated incoming packets */
    process_client_packets(client);
}


client *init_client(server_shared_memory *sh_mem, int socket) {
    char id;
    client *cl;

    if (sh_mem->client_count == MAX_CLIENTS)
        return NULL;

    for (id = 0; id < MAX_CLIENTS; id++) {
        cl = &sh_mem->clients[id];
        if (cl->id == -1) {
            sh_mem->client_count++;
            cl->id = id;
            cl->socket = socket;
            cl->state = PLAYER_STATE_JOIN;
            cl->lobby = NULL;
            cl->game_state = NULL;
            cl->recv_mem.packet_ready = PACKET_READY_FALSE;
            cl->send_mem.packet_ready = PACKET_READY_FALSE;
            return cl;
        }
    }
    return NULL;
}

void remove_client(client *client, server_shared_memory *sh_mem) {
    client->id = -1;
    sh_mem->client_count--;
    close(client->socket);
}

/* packet processing */
/* validate incoming packets */
void *receive_client_packets(void *arg) {
    /* process thread arguments */
    server_recv_send_thread_args *srsta = (server_recv_send_thread_args *) arg;
    client *client = srsta->client;
    server_shared_memory *sh_mem = srsta->sh_mem;

    /* initialize packet number counter for received packets */
    uint32_t recv_pn = 0;

    /* variables for code clarity */
    server_recv_memory *recv_mem = &client->recv_mem;
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
        if ((len = recv(client->socket, &c, 1, 0)) > 0) {
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
                        // print_bytes(packet, i);

                        /* verify packet */
                        *packet_ready = verify_packet(recv_pn, packet_buf, i);

                        recv_pn++;
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
            printf("Client disconnected (client_id=%d, socket=%d)\n", client->id, client->socket);
            remove_client(client, sh_mem);
            exit(0);
        }
        else {
            printf("Recv() error (errno=%d)\n", errno);
            printf("Removing client... (client_id=%d, socket=%d)\n", client->id, client->socket);
            remove_client(client, sh_mem);
            exit(-1);
        }
    }
    return NULL;
}

void *send_server_packets(void *arg) {
    /* process thread arguments */
    server_recv_send_thread_args *srsta = (server_recv_send_thread_args *) arg;
    client *client = srsta->client;
    server_shared_memory *sh_mem = srsta->sh_mem;

    /* initialize packet number counter for sent packets */
    uint32_t send_pn = 0;

    /* allocate buffers for storing packets before they are sent (buffering) */
    server_send_memory *send_mem = &client->send_mem;
    char buf[PACKET_HEADER_SIZE + PACKET_FROM_SERVER_MAX_DATA_SIZE];
    char final_buf[sizeof(buf) * 2 + PACKET_SEPARATOR_SIZE];

    while(1) {
        if (send_mem->packet_ready == PACKET_READY_TRUE) {
            if (send_mem->pid == PACKET_ACCEPT_ID)
                send_server_packet(send_pn++, PACKET_ACCEPT_DATA_SIZE, send_mem, buf, final_buf, client->socket);
            else if (send_mem->pid == PACKET_MESSAGE_ID)
                send_server_packet(send_pn++, PACKET_MESSAGE_DATA_SIZE, send_mem, buf, final_buf, client->socket);
            else if (send_mem->pid == PACKET_LOBBY_ID ||
                        send_mem->pid == PACKET_GAME_READY_ID ||
                        send_mem->pid == PACKET_GAME_STATE_ID ||
                        send_mem->pid == PACKET_GAME_END_ID)
                send_server_packet(send_pn++, send_mem->datalen, send_mem, buf, final_buf, client->socket);
            else
                printf("Invalid pid (%u)\n", (unsigned) send_mem->pid);
            send_mem->packet_ready = PACKET_READY_FALSE;
        }
        else
            sleep(PACKET_READY_WAIT_TIME);
    }

    return NULL;
}

void process_client_packets(client *client) {
    server_recv_memory *recv_mem = &(client->recv_mem);
    char *packet_ready = &recv_mem->packet_ready;
    unsigned char pid = *((unsigned char *) recv_mem->packet_buf + PACKET_NUMBER_SIZE);
    char *pdata = recv_mem->packet_buf + PACKET_HEADER_SIZE;

    while (1) {
        if (*packet_ready == PACKET_READY_TRUE) {
            switch (pid) {
                case PACKET_JOIN_ID:
                    process_join(pdata, client);
                    break;
                case PACKET_MESSAGE_ID:
                    process_message_from_client(pdata, client);
                    break;
                case PACKET_PLAYER_READY_ID:
                    process_player_ready(client);
                    break;
                case PACKET_PLAYER_INPUT_ID:
                    process_player_input(pdata, client);
                    break;
                case PACKET_CHECK_STATUS_ID:
                    process_check_status(client);
                    break;
                case PACKET_GAME_TYPE_ID:
                    process_game_type(pdata, client);
                    break;
                default:
                    printf("Invalid pid (%u)\n", pid);
            }
            *packet_ready = PACKET_READY_FALSE;
        }
        else
            sleep(PACKET_READY_WAIT_TIME);
    }
}

void process_join(char *data, client *client) {
    char *name;
    size_t namelen;
    // printf("RECEIVED JOIN, NAME = %s\n", name);

    if (client->state == PLAYER_STATE_JOIN) {
        name = data;
        namelen = strlen(name);
        if (namelen > MAX_NAME_SIZE - 1) {
            // name too long
            send_accept(PACKET_ACCEPT_STATUS_ERROR, client);
            send_message_from_server(PACKET_MESSAGE_TYPE_ERROR, PACKET_MESSAGE_SOURCE_SERVER, "Name must not be more than 19 characters long!", client);
            printf("Client (id=%d) tried to join with too long name\n", client->id);
        }
        else if (!is_alphanum(name, namelen)) {
            // must contain only alphanumerical characters
            send_accept(PACKET_ACCEPT_STATUS_ERROR, client);
            send_message_from_server(PACKET_MESSAGE_TYPE_ERROR, PACKET_MESSAGE_SOURCE_SERVER, "Name must contain only alphanumerical characters!", client);
            printf("Client (id=%d) tried to join with name that does not contain only alphanumerical characters\n", client->id);
        }
        else {
            // everything ok
            insert_str(name, namelen, client->name, MAX_NAME_SIZE, 0);
            client->state = PLAYER_STATE_MENU;
            send_accept(client->id, client);
        }
    }
    else {
            printf("Client (id=%d) send JOIN packet while being in wrong state (state=%d)\n", client->id, client->state);
    }
}

void process_message_from_client(char *data, client *client) {
    char target_id, source_id, *message, i;
    size_t mlen;
    // printf("Received MESSAGE from client (id=%d, socket=%d), type=%d, source_id=%d, message=%s\n", cd->id, cd->socket, type, source_id, message);

    if (client->state == PLAYER_STATE_LOBBY) {
        target_id = *data;
        source_id = *(data + 1);
        message = data + 2;
        mlen = strlen(message);

        if (mlen > MAX_MESSAGE_SIZE - 1)
            printf("Client (id=%d) tried to send message that is too long\n", client->id);
        else if (target_id == PACKET_MESSAGE_TARGET_ALL) {
            // send message to all players in the same lobby
            for (i = 0; i < client->lobby->player_count; i++) {
                send_message_from_server(PACKET_MESSAGE_TYPE_CHAT, source_id, message, client->lobby->players[i]);
            }
        }
        else {
            send_message_from_server(PACKET_MESSAGE_TYPE_CHAT, source_id, message, client);
        }
    }
    else {
        printf("Client (id=%d) send MESSAGE packet while being in wrong state (state=%d) \n", client->id, client->state);
    }
}

void process_player_ready(client *client) {
    // TODO: start game
}

void process_player_input(char *data, client *client) {
    char input;
    char exit, down, up;

    input = *data;
    exit = input & 1;
    down = (input >> 1) & 1;
    up = (input >> 2) & 1;

    if (exit) {
        if (client->state == PLAYER_STATE_STATISTICS) {
            // TODO: move player to main menu
        }
        else {
            // TODO: move player to join
        }
    }

    if (down && client->state == PLAYER_STATE_GAME) {
        // TODO: set (positive?) player acceleration
    }

    if (up && client->state == PLAYER_STATE_GAME) {
        // TODO: set (negative?) player acceleration
    }
}

void process_check_status(client *client) {
    // do nothing
}

void process_game_type(char *data, client *client) {
    char type;

    if (client->state == PLAYER_STATE_MENU) {
        type = *data;
        if (type == PACKET_GAME_TYPE_TYPE_1V1) {
            // TODO: add player to 1v1 lobby
        }
        else if (type == PACKET_GAME_TYPE_TYPE_2V2) {
            // TODO: add player to 2v2 lobby
        }
        else {
            printf("Client (id=%d) send invalid game type (type=%d) \n", client->id, type);
        }
    }
    else
        printf("Client (id=%d) send GAME_TYPE packet while being in wrong state (state=%d) \n", client->id, client->state);
}


void send_server_packet(uint32_t pn, int32_t psize, server_send_memory *send_mem, char *buf, char *final_buf, int socket) {
    send_packet(pn, send_mem->pid, psize, send_mem->pdata, send_mem->datalen, 
                    buf, PACKET_HEADER_SIZE + PACKET_FROM_SERVER_MAX_DATA_SIZE, 
                    final_buf, (PACKET_HEADER_SIZE + PACKET_FROM_SERVER_MAX_DATA_SIZE) * 2 + PACKET_SEPARATOR_SIZE, 
                    socket);
}

void send_accept(char player_id, client *client) {
    server_send_memory *send_mem;

    send_mem = &client->send_mem;
    while (send_mem->packet_ready == PACKET_READY_TRUE)
        sleep(PACKET_READY_WAIT_TIME);

    send_mem->pid = PACKET_ACCEPT_ID;
    send_mem->datalen = insert_char(player_id, send_mem->pdata, sizeof(send_mem->pdata), 0);
    send_mem->packet_ready = PACKET_READY_TRUE;
}

void send_message_from_server(char type, char source_id, char *message, client *client) {
    size_t mlen;
    size_t offset;
    server_send_memory *send_mem;

    send_mem = &client->send_mem;
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

void send_lobby(client *client) {
    char i;
    size_t offset;
    server_send_memory *send_mem;

    send_mem = &client->send_mem;
    while (send_mem->packet_ready == PACKET_READY_TRUE)
        sleep(PACKET_READY_WAIT_TIME);

    send_mem->pid = PACKET_LOBBY_ID;
    offset = insert_char(client->lobby->player_count, send_mem->pdata, sizeof(send_mem->pdata), 0);
    for (i = 0; i < client->lobby->player_count; i++) {
        offset += insert_char(client->lobby->players[i]->id, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_bytes(client->lobby->players[i]->name, MAX_NAME_SIZE, send_mem->pdata, sizeof(send_mem->pdata), offset);
    }    
    send_mem->datalen = offset;
    send_mem->packet_ready = PACKET_READY_TRUE;
}

void send_game_ready(client *client) {
    char i;
    size_t offset;
    game_state *gs;
    server_send_memory *send_mem;

    gs = client->game_state;
    send_mem = &client->send_mem;
    while (send_mem->packet_ready == PACKET_READY_TRUE)
        sleep(PACKET_READY_WAIT_TIME);

    send_mem->pid = PACKET_GAME_READY_ID;
    offset = insert_int32_t_as_big_endian(gs->window_width, send_mem->pdata, sizeof(send_mem->pdata), 0);
    offset += insert_int32_t_as_big_endian(gs->window_height, send_mem->pdata, sizeof(send_mem->pdata), offset);

    offset += insert_char(gs->team_count, send_mem->pdata, sizeof(send_mem->pdata), offset);
    for (i = 0; i < gs->team_count; i++) {
        offset += insert_char(gs->teams[i].id, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->teams[i].goal1.x, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->teams[i].goal1.y, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->teams[i].goal2.x, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->teams[i].goal2.y, send_mem->pdata, sizeof(send_mem->pdata), offset);
    }

    offset += insert_char(gs->player_count, send_mem->pdata, sizeof(send_mem->pdata), offset);
    for (i = 0; i < gs->player_count; i++) {
        offset += insert_char(gs->players[i].id, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_char(gs->players[i].ready, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_char(gs->players[i].team_id, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_bytes(gs->players[i].name, MAX_NAME_SIZE,send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->players[i].pos.x, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->players[i].pos.y, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->players[i].width, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->players[i].height, send_mem->pdata, sizeof(send_mem->pdata), offset);
    }

    send_mem->datalen = offset;
    send_mem->packet_ready = PACKET_READY_TRUE;
}

void send_game_state(client *client) {
    char i;
    size_t offset;
    game_state *gs;
    server_send_memory *send_mem;

    gs = client->game_state;
    send_mem = &client->send_mem;
    while (send_mem->packet_ready == PACKET_READY_TRUE)
        sleep(PACKET_READY_WAIT_TIME);

    send_mem->pid = PACKET_GAME_STATE_ID;
    offset = insert_int32_t_as_big_endian(gs->window_width, send_mem->pdata, sizeof(send_mem->pdata), 0);
    offset += insert_int32_t_as_big_endian(gs->window_height, send_mem->pdata, sizeof(send_mem->pdata), offset);

    offset += insert_char(gs->team_count, send_mem->pdata, sizeof(send_mem->pdata), offset);
    for (i = 0; i < gs->team_count; i++) {
        offset += insert_char(gs->teams[i].id, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_int32_t_as_big_endian(gs->teams[i].score, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->teams[i].goal1.x, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->teams[i].goal1.y, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->teams[i].goal2.x, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->teams[i].goal2.y, send_mem->pdata, sizeof(send_mem->pdata), offset);
    }

    offset += insert_char(gs->player_count, send_mem->pdata, sizeof(send_mem->pdata), offset);
    for (i = 0; i < gs->player_count; i++) {
        offset += insert_char(gs->players[i].id, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_char(gs->players[i].team_id, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->players[i].pos.x, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->players[i].pos.y, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->players[i].width, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->players[i].height, send_mem->pdata, sizeof(send_mem->pdata), offset);
    }

    offset += insert_char(gs->ball_count, send_mem->pdata, sizeof(send_mem->pdata), offset);
    for (i = 0; i < gs->ball_count; i++) {
        offset += insert_float_as_big_endian(gs->balls[i].pos.x, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->balls[i].pos.y, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->balls[i].radius, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_char(gs->balls[i].type, send_mem->pdata, sizeof(send_mem->pdata), offset);
    }

    offset += insert_char(gs->power_up_count, send_mem->pdata, sizeof(send_mem->pdata), offset);
    for (i = 0; i < gs->power_up_count; i++) {
        offset += insert_char(gs->power_ups[i].type, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->power_ups[i].pos.x, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->power_ups[i].pos.y, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->power_ups[i].width, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_float_as_big_endian(gs->power_ups[i].height, send_mem->pdata, sizeof(send_mem->pdata), offset);
    }

    send_mem->datalen = offset;
    send_mem->packet_ready = PACKET_READY_TRUE;
}

void send_game_end(client *client) {
    char i;
    size_t offset;
    game_state *gs;
    server_send_memory *send_mem;

    gs = client->game_state;
    send_mem = &client->send_mem;
    while (send_mem->packet_ready == PACKET_READY_TRUE)
        sleep(PACKET_READY_WAIT_TIME);

    send_mem->pid = PACKET_GAME_END_ID;

    offset = insert_char(gs->end_status, send_mem->pdata, sizeof(send_mem->pdata), 0);
    offset += insert_int32_t_as_big_endian(find_team_score(client), send_mem->pdata, sizeof(send_mem->pdata), offset);
    offset += insert_int32_t_as_big_endian(gs->end_time - gs->start_time, send_mem->pdata, sizeof(send_mem->pdata), offset);

    offset += insert_char(gs->team_count, send_mem->pdata, sizeof(send_mem->pdata), offset);
    for (i = 0; i < gs->team_count; i++) {
        offset += insert_char(gs->teams[i].id, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_int32_t_as_big_endian(gs->teams[i].score, send_mem->pdata, sizeof(send_mem->pdata), offset);
    }

    offset += insert_char(gs->player_count, send_mem->pdata, sizeof(send_mem->pdata), offset);
    for (i = 0; i < gs->player_count; i++) {
        offset += insert_char(gs->players[i].id, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_char(gs->players[i].team_id, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_int32_t_as_big_endian(gs->players[i].score, send_mem->pdata, sizeof(send_mem->pdata), offset);
        offset += insert_bytes(gs->players[i].name, MAX_NAME_SIZE, send_mem->pdata, sizeof(send_mem->pdata), offset);
    }

    send_mem->datalen = offset;
    send_mem->packet_ready = PACKET_READY_TRUE;
}

/* helpers */
int find_team_score(client *client) {
    char i, team_id;
    game_state *gs;

    gs = client->game_state;
    if (gs == NULL)
        return -1;

    team_id = -1;
    for (i = 0; i < gs->player_count; i++) {
        if (gs->players[i].id == client->id)
            team_id = gs->players[i].team_id;
    }
    if (team_id == -1)
        return -1;

    for (i = 0; i < gs->team_count; i++) {
        if (gs->teams[i].id == team_id)
            return gs->teams[i].score;
    }
    return -1;
}

int is_alphanum(char *data, size_t datalen) {
    size_t i;

    for (i = 0; i < datalen; i++) {
        if (!((data[i] >= '0' && data[i] <= '9') || (data[i] >= 'a' && data[i] <= 'z') || (data[i] >= 'A' && data[i] <= 'Z')))
            return 0;
    }
    return 1;
}


/* debug */
void print_client_data(client *client) {
    printf("ID: %d\n", client->id);
    printf("SOCKET: %d\n", client->socket);
    printf("NAME %s\n", client->name);
    printf("STATE: %d\n", client->state);
    // TODO: print lobby
    // TODO: print game_state
    // print_recv_memory(&(cd->recv_mem_cfg));
    // print_send_memory(&(cd->send_mem_cfg));
}

void print_shared_memory(server_shared_memory* sh_mem) {
    // char id;
    // client_data cd;

    // /* print client count */
    // printf("Client count: ");
    // print_bytes(sh_mem->client_count, 1);
    // putchar('\n');

    // /* print client datas */
    // for (id = 0; id < MAX_CLIENTS; id++) {
    //     print_client_data(&cd);
    //     putchar('\n');
    // }
    // putchar('\n');

    /* TODO: print game states */
}