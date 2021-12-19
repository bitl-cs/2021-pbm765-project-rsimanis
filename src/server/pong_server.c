#include "pong_server.h"


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
    printf("Initializing server...\n");
    init_server(sh_mem);
    printf("  Starting accepting clients...\n");
    accept_clients(server_socket, sh_mem);
    printf(" Stopped accepting clients (Should not happen!) - server network stopped!\n");
}

void init_server(server_shared_memory *sh_mem) {
    char i;

    /* reset clients */
    sh_mem->client_count = 0;
    for (i = 0; i < MAX_CLIENTS; i++)
        sh_mem->clients[i].id = CLIENT_ID_TAKEN_FALSE;

    /* init lobbys */
    init_lobby(&sh_mem->lobby_1v1, GAME_1V1_PLAYER_COUNT);
    init_lobby(&sh_mem->lobby_2v2, GAME_2V2_PLAYER_COUNT);

    /* reset game states */ 
    for (i = 0; i < MAX_GAME_STATES; i++)
        sh_mem->game_states[i].status = GAME_STATE_STATUS_FREE;
}

void init_lobby(lobby *lobby, int max_clients) {
    lobby->max_clients = max_clients;
    reset_lobby(lobby);
}

void reset_lobby(lobby *lobby) {
    char i;

    lobby->last_update = clock();
    lobby->client_count = 0;
    for (i = 0; i < lobby->max_clients; i++)
        lobby->clients[i] = NULL;
}

void init_teams(game_state *gs, int player_count) {
    team *left_team, *right_team;

    gs->team_count = TEAM_INITIAL_COUNT;

    /* initialize left team */
    left_team = &gs->teams[LEFT_TEAM_ID];
    init_team(left_team, LEFT_TEAM_ID, player_count, 
                LEFT_GOAL_LINE_UPPER_X, LEFT_GOAL_LINE_UPPER_Y,
                LEFT_GOAL_LINE_BOTTOM_X, LEFT_GOAL_LINE_BOTTOM_Y);

    /* initialize right team */
    right_team = &gs->teams[RIGHT_TEAM_ID];
    init_team(right_team, RIGHT_TEAM_ID, player_count, 
                RIGHT_GOAL_LINE_UPPER_X, RIGHT_GOAL_LINE_UPPER_Y,
                RIGHT_GOAL_LINE_BOTTOM_X, RIGHT_GOAL_LINE_BOTTOM_Y);
}

void init_back_players(game_state *gs, lobby *lobby) {
    player *p;
    client *c;

    /* initialize left player */
    p = &gs->players[LEFT_BACK_PLAYER_ID];
    c = lobby->clients[LEFT_BACK_PLAYER_ID];
    init_player(p, LEFT_BACK_PLAYER_ID, c->id, LEFT_TEAM_ID, c->name, LEFT_BACK_PLAYER_INITIAL_X, LEFT_BACK_PLAYER_INITIAL_Y);
    c->player = p;

    /* initialize right player */
    p = &gs->players[RIGHT_BACK_PLAYER_ID];
    c = lobby->clients[RIGHT_BACK_PLAYER_ID];
    init_player(p, RIGHT_BACK_PLAYER_ID, c->id, RIGHT_TEAM_ID, c->name, RIGHT_BACK_PLAYER_INITIAL_X, RIGHT_BACK_PLAYER_INITIAL_Y);
    c->player = p;
}

void init_front_players(game_state *gs, lobby *lobby) {
    player *p;
    client *c;

    /* initialize left player */
    p = &gs->players[LEFT_FRONT_PLAYER_ID];
    c = lobby->clients[LEFT_FRONT_PLAYER_ID];
    init_player(p, LEFT_FRONT_PLAYER_ID, c->id, LEFT_TEAM_ID, c->name, LEFT_FRONT_PLAYER_INITIAL_X, LEFT_FRONT_PLAYER_INITIAL_Y);
    c->player = p;

    /* initialize right player */
    p = &gs->players[RIGHT_FRONT_PLAYER_ID];
    c = lobby->clients[RIGHT_FRONT_PLAYER_ID];
    init_player(p, RIGHT_FRONT_PLAYER_ID, c->id, RIGHT_TEAM_ID, c->name, RIGHT_FRONT_PLAYER_INITIAL_X, RIGHT_FRONT_PLAYER_INITIAL_Y);
    c->player = p;
}




void init_game_1v1(game_state *gs, lobby *lobby) {
    if (gs == NULL) {
        printf("Cannot initialize game with NULL game_state\n");
        return;
    }
    if (lobby->client_count != GAME_1V1_PLAYER_COUNT) {
        printf("Cannot initialize 1v1 game with %d clients in the lobby\n", lobby->client_count);
        return;
    }

    gs->game_type = GAME_TYPE_1V1;
    init_screen(gs);
    init_teams(gs, 1);
    gs->player_count = 2;
    init_back_players(gs, lobby); /* in 1v1 game mode, only back players are playing */
    init_balls(gs);
    init_power_ups(gs);

    gs->last_update = clock();
    gs->status = GAME_STATE_STATUS_LOADING;
}

void init_game_2v2(game_state *gs, lobby *lobby) {
    if (gs == NULL) {
        printf("Cannot initialize game with NULL game_state\n");
        return;
    }
    if (lobby->client_count != GAME_2V2_PLAYER_COUNT) {
        printf("Cannot initialize 1v1 game with %d clients in the lobby\n", lobby->client_count);
        return;
    }

    gs->game_type = GAME_TYPE_2V2;
    init_screen(gs);
    init_teams(gs, 2);
    gs->player_count = 4;
    init_back_players(gs, lobby);
    init_front_players(gs, lobby);
    init_balls(gs);
    init_power_ups(gs);

    gs->last_update = clock();
    gs->status = GAME_STATE_STATUS_LOADING;
}

/* game */
void update_lobby(lobby *lobby, server_shared_memory *sh_mem) {
    char i, gs_id;
    double diff;        /* time difference in seconds */
    game_state *gs;
    clock_t now;
    client *c;

    // if (lobby == NULL) {
    //     printf("update_lobby(lobby, sh_mem): lobby = NULL\n");
    //     return;
    // }
    // if (sh_mem == NULL) {
    //     printf("update_lobby(lobby, sh_mem): sh_mem = NULL\n");
    //     return;
    // }

    now = clock();
    diff = (double) (now - lobby->last_update) / CLOCKS_PER_SEC;
    if (diff >= LOBBY_UPDATE_INTERVAL) {
        lobby->last_update = now;
        if (lobby->client_count == lobby->max_clients) {
            /* send the lobby packet also to the last client who joined this lobby */
            send_lobby_to_all_clients_in_the_lobby(lobby, sh_mem);

            /* find free game state */
            gs_id = get_free_game_state(sh_mem);
            if (gs_id == -1) {
                printf("Currently no free game state available (should not happen)\n");
                return;
            }
            gs = &sh_mem->game_states[gs_id];

            /* update player info*/
            for (i = 0; i < lobby->client_count; i++) {
                c = lobby->clients[i];
                if (c != NULL) {
                    c->lobby = NULL;
                    c->game_state = gs;
                    c->state = CLIENT_STATE_LOADING;
                }
            }

            /* start game */
            if (lobby->max_clients == GAME_1V1_PLAYER_COUNT)
                init_game_1v1(gs, lobby);
            else if (lobby->max_clients == GAME_2V2_PLAYER_COUNT)
                init_game_2v2(gs, lobby);

            /* reset lobby */
            reset_lobby(lobby);
        }
        else {
            /* waiting for players => send current lobby state to client */
            send_lobby_to_all_clients_in_the_lobby(lobby, sh_mem);
        }
    }
}

void gameloop(server_shared_memory *sh_mem) {
    char i, j;
    game_state *gs;
    client *c;
    player *p;

    printf("Starting game loop! (It will run forever - use Ctrl+C)\n");
    while (1) {
        update_lobby(&sh_mem->lobby_1v1, sh_mem);
        update_lobby(&sh_mem->lobby_2v2, sh_mem);
        for (i = 0; i < MAX_GAME_STATES; i++)
            update_game(&sh_mem->game_states[i], sh_mem);
    }
}

void update_game(game_state *gs, server_shared_memory *sh_mem) {
    clock_t now;

    now = clock();
    if (time_diff_in_seconds(gs->last_update, now) >= GAME_STATE_UPDATE_INTERVAL) {
        gs->last_update = now;
        if (gs->status >= GAME_STATE_STATUS_SUCCESS) {
            send_game_statistics_to_all_players(gs, sh_mem);
            gs->status = GAME_STATE_STATUS_STATISTICS;
        }
        else if (gs->status == GAME_STATE_STATUS_LOADING) {
            if (all_players_ready(gs)) {
                set_all_player_client_status_to_game(gs, sh_mem); 
                gs->status = GAME_STATE_STATUS_IN_PROGRESS;
            }
            else
                send_game_ready_to_all_players(gs, sh_mem);
        }
        else if (gs->status == GAME_STATE_STATUS_IN_PROGRESS) {
            update_game_state(gs);
            send_game_state_to_all_players(gs, sh_mem);
        }
        else if (gs->status == GAME_STATE_STATUS_STATISTICS) {
            if (all_players_disconnected(gs))
                gs->status = GAME_STATE_STATUS_FREE; /* other values inside the game_state will be changed when it will be initialized for the next game */
            else if (time_diff_in_seconds(gs->end_time, now) >= STATISTICS_DURATION) {
                send_return_to_menu_to_all_players(gs, sh_mem);
                gs->status = GAME_STATE_STATUS_FREE;
            }
        }
    }
}

void set_all_player_client_status_to_game(game_state *gs, server_shared_memory *sh_mem) {
    int i;

    for (i = 0; i < gs->player_count; i++)
        sh_mem->clients[gs->players[i].client_id].state = CLIENT_STATE_GAME;
}

void send_lobby_to_all_clients_in_the_lobby(lobby *lobby, server_shared_memory *sh_mem) {
    int i;

    for (i = 0; i < lobby->max_clients; i++) {
        if (lobby->clients[i] != NULL)
            send_lobby(lobby->clients[i]);
    }
}

void send_game_ready_to_all_players(game_state *gs, server_shared_memory *sh_mem) {
    char i;

    for (i = 0; i < gs->player_count; i++)
        send_game_ready(&sh_mem->clients[gs->players[i].client_id]);
}

void send_game_state_to_all_players(game_state *gs, server_shared_memory *sh_mem) {
    char i;
    player *p;

    for (i = 0; i < gs->player_count; i++) {
        p = &gs->players[i];
        if (p->client_id != -1)
            send_game_state(&sh_mem->clients[p->client_id]);
    }
}

void send_game_statistics_to_all_players(game_state *gs, server_shared_memory *sh_mem) {
    char i;
    player *p;
    client *c;
    char *error_msg;

    // if (gs == NULL || sh_mem == NULL)
    //     return;

    // for all players
    for (i = 0; i < gs->player_count; i++) {
        p = &gs->players[i];
        if (p == NULL || p->client_id == PLAYER_DISCONNECTED_CLIENT_ID)
            continue;
        c = &sh_mem->clients[p->client_id];
        // if (c->id == -1)
        //     continue;
        c->state = CLIENT_STATE_STATISTICS;

        // send game_end
        send_game_end(c);
        if (gs->status > 0) {
            switch (gs->status) {
                case GAME_STATE_STATUS_CLIENT_FAILED_TO_LOAD:
                    error_msg = "Client disconnected before the game finished loading";
                    break;
                case GAME_STATE_STATUS_ALL_TEAM_LEFT:
                    error_msg = "A team left the game";
                    break;
                default:
                    error_msg = "Something went wrong";
            }
            send_message_from_server(PACKET_MESSAGE_TYPE_ERROR, PACKET_MESSAGE_SOURCE_SERVER, error_msg, c);
        }
    }
}

void send_return_to_menu_to_all_players(game_state *gs, server_shared_memory *sh_mem) {
    int i;

    for (i = 0; i < MAX_PLAYER_COUNT; i++)
        send_return_to_menu_to_player(&gs->players[i], sh_mem);
}

void send_return_to_menu_to_player(player *p, server_shared_memory *sh_mem) {
    client *c;

    if (p->client_id == PLAYER_DISCONNECTED_CLIENT_ID)
        return;

    // update clients state
    c = &sh_mem->clients[p->client_id];
    c->game_state = NULL; 
    c->player = NULL;
    c->state = CLIENT_STATE_MENU;

    // show that player disconnected from the game
    p->client_id = PLAYER_DISCONNECTED_CLIENT_ID;

    // send return_to_menu
    send_return_to_menu(c);
}


/* client processing */
void accept_clients(int server_socket, server_shared_memory *sh_mem) {
    int tid, client_socket, client_id;

    tid = 0;
    client_socket = -1;
    while (1) {
        client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            printf("  Soft ERROR accepting the client connection! ERRNO=%d Continuing...\n", errno);
            continue;
        }

        /* New client */
        client_id = get_free_client(client_socket, sh_mem);
        if (client_id == -1) {
            printf("Limit for maximum clients is reached\n");
            close(client_socket);
            continue;
        }

        tid = fork();
        if (tid == 0){
            close(server_socket);
            tid = fork();
            if (tid == 0){
                process_client(&sh_mem->clients[client_id], sh_mem);
                exit(0);
            }
            else {
                wait(NULL); /* blocks parent process until any of its children has finished */
                exit(0);
            }
        }
        else
            close(client_socket);
    }
}

void process_client(client *client, server_shared_memory* sh_mem) {
    printf("CLIENT connected (client_count=%d, client_id=%d, socket=%d)\n", sh_mem->client_count, client->id, client->socket);

    /* initialize packet sending thread */
    server_send_thread_args sta;
    sta.client = client;
    sta.sh_mem = sh_mem;
    pthread_t sending_thread_id;
    if (pthread_create(&sending_thread_id, NULL, send_server_packets, (void *) &sta) != 0)
        exit(-1);
    sleep(THREAD_INIT_WAIT_TIME); /* wait until thread's local variables from its argument (recv_thread_args) are initialized */

    /* validate and process incoming server packets */
    receive_client_packets(client, sh_mem);
}


char get_free_client(int socket, server_shared_memory *sh_mem) {
    char id;
    client *c;

    if (sh_mem->client_count == MAX_CLIENTS)
        return -1;

    for (id = 0; id < MAX_CLIENTS; id++) {
        c = &sh_mem->clients[id];
        if (c->id == CLIENT_ID_TAKEN_FALSE) {
            sh_mem->client_count++;
            init_client(id, socket, c);
            return id;
        }
    }
    return -1;
}

void init_client(char id, int socket, client *c) {
    c->id = id;
    c->socket = socket;
    c->state = CLIENT_STATE_JOIN;
    c->lobby = NULL;
    c->game_state = NULL;
    c->player = NULL;
    c->send_mem.packet_ready = PACKET_READY_FALSE;

}

void remove_client(client *client, server_shared_memory *sh_mem) {
    reset_client(client);
    close(client->socket);
    client->socket = -1;
    sh_mem->client_count--;
    client->id = CLIENT_ID_TAKEN_FALSE;
}

void remove_client_from_lobby(client *client) {
    int i;
    lobby *lobby;

    if (client == NULL) {
        printf("remove_client_from_lobby(client *client), client=NULL\n");
        return;
    }

    lobby = client->lobby;
    if (lobby == NULL) {
        printf("remove_client_from_lobby(client *client), client->lobby=NULL\n");
        return;
    }

    for (i = 0; i < lobby->max_clients; i++) {
        if (lobby->clients[i] == client) {
            lobby->client_count--;
            lobby->clients[i] = NULL;
            break;
        }
    }
    client->lobby = NULL;
}

void remove_client_from_game_state(client *client) {
    game_state *gs;
    player *p;
    ball *b;
    int i;

    p = client->player;
    gs = client->game_state;

    if (client->game_state == NULL || p == NULL)
        return;

    p->client_id = PLAYER_DISCONNECTED_CLIENT_ID;
    gs->teams[p->team_id].active_player_count--;
    for (i = 0; i < gs->ball_count; i++) {
        b = &gs->balls[i]; 
        if (b->last_touched_id == p->id)
            b->last_touched_id = BALL_INITIAL_LAST_TOUCHED_ID;
    }

    client->player = NULL;
    client->game_state = NULL;

    printstr("CLIENT FROM GAME STATE REMOVED");
}

void reset_client(client *client) {
    if (client->lobby != NULL) {
        remove_client_from_lobby(client);
        client->lobby = NULL;
    }
    if (client->game_state != NULL) {
        remove_client_from_game_state(client);
        client->player = NULL;
        client->game_state = NULL;
    }
    client->state = CLIENT_STATE_JOIN;
}

/* packet processing */
void receive_client_packets(client *client , server_shared_memory *sh_mem) {
    /* initialize packet number counter for received packets */
    uint32_t recv_pn = 0;

    /* variables for code clarity */
    server_recv_memory *recv_mem = &client->recv_mem;
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
                            process_client_packets(client, sh_mem);
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
            printf("Client (id=%d) disconnected\n", client->id);
            remove_client(client, sh_mem);
            exit(0);
        }
        else {
            printf("Client (id=%d) recv() error (errno=%d). Removing...\n", client->id, errno);
            remove_client(client, sh_mem);
            exit(-1);
        }
    }
}

/* process already validated packets */
void process_client_packets(client *client, server_shared_memory *sh_mem) {
    unsigned char *pid = (unsigned char *) (client->recv_mem.packet_buf + PACKET_NUMBER_SIZE);
    char *pdata = client->recv_mem.packet_buf + PACKET_HEADER_SIZE;

    switch (*pid) {
        case PACKET_JOIN_ID:
            process_join(pdata, client, sh_mem);
            break;
        case PACKET_MESSAGE_ID:
            process_message_from_client(pdata, client, sh_mem);
            break;
        case PACKET_PLAYER_READY_ID:
            process_player_ready(client, sh_mem);
            break;
        case PACKET_PLAYER_INPUT_ID:
            process_player_input(pdata, client, sh_mem);
            break;
        case PACKET_CHECK_STATUS_ID:
            process_check_status(client);
            break;
        case PACKET_GAME_TYPE_ID:
            process_game_type(pdata, client, sh_mem);
            break;
        default:
            printf("Invalid pid (%u)\n", *pid);
    }
}

void process_join(char *data, client *client, server_shared_memory *sh_mem) {
    char *name;
    size_t namelen;
    printf("Received JOIN from client (id=%d)\n", client->id);

    if (client->state == CLIENT_STATE_JOIN) {
        name = data;
        namelen = strlen(name);
        if (namelen < MIN_NAME_LENGTH || namelen > MAX_NAME_LENGTH) {
            // name too long
            send_accept(PACKET_ACCEPT_STATUS_ERROR, client);
            send_message_from_server(PACKET_MESSAGE_TYPE_ERROR, PACKET_MESSAGE_SOURCE_SERVER, "Name must be between 3 and 19 characters long!", client);
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
            insert_str(name, namelen, client->name, sizeof(client->name), 0);
            client->state = CLIENT_STATE_MENU;
            send_accept(client->id, client);
        }
    }
    else {
        printf("Client (id=%d) send JOIN packet while being in wrong state (state=%d)\n", client->id, client->state);
    }
    // print_shared_memory(sh_mem);
}

void process_message_from_client(char *data, client *client, server_shared_memory *sh_mem) {
    char target_id, source_id, *message, i;
    size_t mlen;
    printf("Received MESSAGE from client (id=%d)\n", client->id);

    target_id = *data;
    source_id = *(data + 1);
    message = data + 2;
    mlen = strlen(message);

    if (mlen > MAX_MESSAGE_LENGTH)
        printf("Client (id=%d) tried to send message that is too long\n", client->id);
    else if (target_id == PACKET_MESSAGE_TARGET_ALL) {
        // send message to all players in the same lobby
        for (i = 0; i < client->lobby->max_clients; i++) {
            if (client->lobby->clients[i] != NULL)
                send_message_from_server(PACKET_MESSAGE_TYPE_CHAT, source_id, message, client->lobby->clients[i]);
        }
    }
    else {
        for (i = 0; i < client->lobby->max_clients; i++) {
            if (client->lobby->clients[i] != NULL && client->lobby->clients[i]->id == target_id)
                send_message_from_server(PACKET_MESSAGE_TYPE_CHAT, source_id, message, client->lobby->clients[i]);
        }
    }
}

void process_player_ready(client *client, server_shared_memory *sh_mem) {
    printf("Received PLAYER_READY from client (id=%d)\n", client->id);
    if (client->state == CLIENT_STATE_LOADING && client->player != NULL)
        client->player->ready = PLAYER_READY_TRUE;
}

void process_player_input(char *data, client *client, server_shared_memory *sh_mem) {
    // printf("Received PLAYER_INPUT from client (id=%d)\n", client->id);
    char input;
    char exit, down, up;

    input = *data;
    exit = input & 1;
    down = (input >> 1) & 1;
    up = (input >> 2) & 1;

    if (exit) {
        if (client->state == CLIENT_STATE_STATISTICS)
            send_return_to_menu_to_player(client->player, sh_mem);
        else {
            reset_client(client);
            send_return_to_join(client);
        }
        return;
    }

    if (client->state == CLIENT_STATE_GAME && client->player != NULL) {
        if (up && !down) {
            // client->player->should_decelerate = 0;
            client->player->a.y = -PLAYER_ACCELERATION_MOD;
        }
        else if (down && !up) {
            // client->player->should_decelerate = 0;
            client->player->a.y = PLAYER_ACCELERATION_MOD;
        }
        else {
            client->player->a.y = 0;
            client->player->v.y *= PLAYER_FRICTION;
        }
    }
}

void process_check_status(client *client) {
    printf("Received CHECK_STATUS from client (id=%d)\n", client->id);
    // do nothing
}

void process_game_type(char *data, client *client, server_shared_memory *sh_mem) {
    printf("Received GAME_TYPE from client (id=%d)\n", client->id);
    char type;

    if (client->state == CLIENT_STATE_MENU) {
        type = *data;
        if (type == PACKET_GAME_TYPE_TYPE_1V1)
            add_client_to_lobby(client, &sh_mem->lobby_1v1);
        else if (type == PACKET_GAME_TYPE_TYPE_2V2)
            add_client_to_lobby(client, &sh_mem->lobby_2v2);
        else
            printf("Client (id=%d) send invalid game type (type=%d) \n", client->id, type);
    }
    else
        printf("Client (id=%d) send GAME_TYPE packet while being in wrong state (state=%d) \n", client->id, client->state);
    // print_shared_memory(sh_mem);
}


void *send_server_packets(void *arg) {
    /* process thread arguments */
    server_send_thread_args *sta = (server_send_thread_args *) arg;
    client *client = sta->client;
    server_shared_memory *sh_mem = sta->sh_mem;

    /* initialize packet number counter for sent packets */
    uint32_t send_pn = 0;

    /* allocate buffers for storing packets before they are sent (buffering) */
    server_send_memory *send_mem = &client->send_mem;
    char buf[PACKET_HEADER_SIZE + PACKET_FROM_SERVER_MAX_DATA_SIZE];
    char final_buf[sizeof(buf) * 2 + PACKET_SEPARATOR_SIZE];

    while(1) {
        if (send_mem->packet_ready == PACKET_READY_TRUE) {
            if (send_mem->pid == PACKET_ACCEPT_ID) {
                send_server_packet(send_pn++, PACKET_ACCEPT_DATA_SIZE, send_mem, buf, final_buf, client->socket);
            }
            else if (send_mem->pid == PACKET_MESSAGE_ID)
                send_server_packet(send_pn++, PACKET_MESSAGE_DATA_SIZE, send_mem, buf, final_buf, client->socket);
            else if (send_mem->pid == PACKET_RETURN_TO_MENU_ID)
                send_server_packet(send_pn++, PACKET_RETURN_TO_MENU_DATA_SIZE, send_mem, buf, final_buf, client->socket);
            else if (send_mem->pid == PACKET_RETURN_TO_JOIN_ID)
                send_server_packet(send_pn++, PACKET_RETURN_TO_JOIN_DATA_SIZE, send_mem, buf, final_buf, client->socket);
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

void send_server_packet(uint32_t pn, int32_t psize, server_send_memory *send_mem, char *buf, char *final_buf, int socket) {
    send_packet(pn, send_mem->pid, psize, send_mem->pdata, send_mem->datalen, 
                    buf, PACKET_FROM_SERVER_MAX_SIZE, 
                    final_buf, PACKET_FROM_SERVER_MAX_SIZE * 2 + PACKET_SEPARATOR_SIZE, 
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
    size_t mlen, offset;
    server_send_memory *send_mem;

    send_mem = &client->send_mem;
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

void send_lobby(client *client) {
    char i;
    size_t offset;
    server_send_memory *send_mem;

    if (client == NULL || client->lobby == NULL)
        return;

    send_mem = &client->send_mem;
    while (send_mem->packet_ready == PACKET_READY_TRUE)
        sleep(PACKET_READY_WAIT_TIME);

    send_mem->pid = PACKET_LOBBY_ID;
    offset = insert_char(client->lobby->client_count, send_mem->pdata, sizeof(send_mem->pdata), 0);
    for (i = 0; i < client->lobby->max_clients; i++) {
        if (client->lobby->clients[i] != NULL) {
            offset += insert_char(client->lobby->clients[i]->id, send_mem->pdata, sizeof(send_mem->pdata), offset);
            offset += insert_bytes(client->lobby->clients[i]->name, MAX_NAME_LENGTH + 1, send_mem->pdata, sizeof(send_mem->pdata), offset);
        }
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
        offset += insert_bytes(gs->players[i].name, MAX_NAME_LENGTH + 1,send_mem->pdata, sizeof(send_mem->pdata), offset);
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
    char i, status;
    size_t offset;
    game_state *gs;
    server_send_memory *send_mem;

    gs = client->game_state;
    send_mem = &client->send_mem;
    while (send_mem->packet_ready == PACKET_READY_TRUE)
        sleep(PACKET_READY_WAIT_TIME);

    send_mem->pid = PACKET_GAME_END_ID;

    status = (gs->status == 0) ? PACKET_GAME_END_STATUS_SUCCESS : PACKET_GAME_END_STATUS_ERROR;
    offset = insert_char(status, send_mem->pdata, sizeof(send_mem->pdata), 0);
    offset += insert_int32_t_as_big_endian(gs->teams[client->player->team_id].score, send_mem->pdata, sizeof(send_mem->pdata), offset);
    offset += insert_int32_t_as_big_endian((gs->end_time - gs->start_time) / CLOCKS_PER_SEC, send_mem->pdata, sizeof(send_mem->pdata), offset);

    offset += insert_char(gs->team_count, send_mem->pdata, sizeof(send_mem->pdata), offset);
    if (status != PACKET_GAME_END_STATUS_ERROR) {
        for (i = 0; i < gs->team_count; i++) {
            offset += insert_char(gs->teams[i].id, send_mem->pdata, sizeof(send_mem->pdata), offset);
            offset += insert_int32_t_as_big_endian(gs->teams[i].score, send_mem->pdata, sizeof(send_mem->pdata), offset);
        }
    }

    offset += insert_char(gs->player_count, send_mem->pdata, sizeof(send_mem->pdata), offset);
    if (status != PACKET_GAME_END_STATUS_ERROR) {
        for (i = 0; i < gs->player_count; i++) {
            offset += insert_char(gs->players[i].id, send_mem->pdata, sizeof(send_mem->pdata), offset);
            offset += insert_char(gs->players[i].team_id, send_mem->pdata, sizeof(send_mem->pdata), offset);
            offset += insert_int32_t_as_big_endian(gs->players[i].score, send_mem->pdata, sizeof(send_mem->pdata), offset);
            offset += insert_bytes(gs->players[i].name, MAX_NAME_LENGTH + 1, send_mem->pdata, sizeof(send_mem->pdata), offset);
        }
    }

    send_mem->datalen = offset;
    send_mem->packet_ready = PACKET_READY_TRUE;
}

void send_return_to_menu(client *client) {
    server_send_memory *send_mem;

    send_mem = &client->send_mem;
    while (send_mem->packet_ready == PACKET_READY_TRUE)
        sleep(PACKET_READY_WAIT_TIME);

    send_mem->pid = PACKET_RETURN_TO_MENU_ID;
    send_mem->datalen = PACKET_RETURN_TO_MENU_DATA_SIZE;
    send_mem->packet_ready = PACKET_READY_TRUE;
}

void send_return_to_join(client *client) {
    server_send_memory *send_mem;

    send_mem = &client->send_mem;
    while (send_mem->packet_ready == PACKET_READY_TRUE)
        sleep(PACKET_READY_WAIT_TIME);

    send_mem->pid = PACKET_RETURN_TO_JOIN_ID;
    send_mem->datalen = PACKET_RETURN_TO_JOIN_DATA_SIZE;
    send_mem->packet_ready = PACKET_READY_TRUE;
}


/* helpers */
char get_free_game_state(server_shared_memory *sh_mem) {
    char i;
    game_state *gs;

    for (i = 0; i < MAX_GAME_STATES; i++) {
        gs = &sh_mem->game_states[i];
        if (gs->status == GAME_STATE_STATUS_FREE) {
            gs->status = GAME_STATE_STATUS_TAKEN;
            return i;
        }
    }
    return -1;
}

void add_client_to_lobby(client *client, lobby *lobby) {
    int i;

    while (lobby->max_clients == lobby->client_count)
        sleep(1/50.0);

    client->lobby = lobby;
    client->state = CLIENT_STATE_LOBBY;
    for (i = 0; i < lobby->max_clients; i++) {
        if (lobby->clients[i] == NULL) {
            lobby->clients[i] = client;
            lobby->client_count += 1;
            break;
        }
    }
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
void print_client(client *client) {
    printf("id: %d\n", client->id);
    printf("socket: %d\n", client->socket);
    printf("name %s\n", client->name);
    printf("state: %d\n", client->state);
}

void print_lobby(lobby *lobby) {
    char i;
    client *c;

    printf("last_update: %lu\n", lobby->last_update);
    printf("client_count: %d\n", lobby->client_count);
    printf("max_clients: %d\n", lobby->max_clients);
    printf("client_ids: ");
    for (i = 0; i < lobby->max_clients; i++) {
        if (lobby->clients[i] != NULL) {
            c = lobby->clients[i];
            if (c == NULL)
                printf("NULL ");
            else
                printf("%d ", c->id);
        }
    }
    putchar('\n');
}

void print_shared_memory(server_shared_memory* sh_mem) {
    char i;

    /* print client count */
    printf("CLIENT COUNT: %u\n", sh_mem->client_count);

    /* print clients */
    putchar('\n');
    printf("====== CLIENTS ======\n");
    for (i = 0; i < sh_mem->client_count; i++) {
        print_client(&sh_mem->clients[i]);
        putchar ('\n');
    }

    /* print lobbys */ 
    putchar ('\n');
    printf("====== LOBBY_1V1 ======\n");
    print_lobby(&sh_mem->lobby_1v1);

    putchar ('\n');
    printf("====== LOBBY_2V2 ======\n");
    print_lobby(&sh_mem->lobby_2v2);

    /* print game_states */
    putchar ('\n');
    putchar ('\n');
    printf("====== GAME_STATES ======\n");
    for (i = 0; i < MAX_GAME_STATES; i++) {
        print_game_state(&sh_mem->game_states[i]);
        printf("----------------------------------------------\n");
    }
}