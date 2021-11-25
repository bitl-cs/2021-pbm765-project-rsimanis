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
#include "args.h"
#include "packets.h"

#define MAX_CLIENTS                         1

#define CLIENT_COUNT_SIZE                   sizeof(int)
#define VALIDITY_BYTE_SIZE                  sizeof(char)
#define SHARED_RECV_CLIENT_BUF_SIZE         CLIENT_PACKET_MAX_SIZE
#define SHARED_RECV_CLIENT_MEMORY_SIZE      (VALIDITY_BYTE_SIZE + SHARED_RECV_CLIENT_BUF_SIZE)
#define SHARED_CLIENT_MEMORY_SIZE           (sizeof(char) + MAX_NAME_SIZE + KEYBOARD_INPUT_SIZE + SHARED_RECV_CLIENT_MEMORY_SIZE)
#define SHARED_MEMORY_SIZE                  (CLIENT_COUNT_SIZE + MAX_CLIENTS * SHARED_CLIENT_MEMORY_SIZE) /* client_count + memory for clients + ... */

int PORT = 0;

char *shared_memory = NULL;
int *client_count = NULL;
char *shared_client_data = NULL; /* first int of shared_data is client_count */

void get_shared_memory();
void gameloop();
void start_network();
void process_client(int id, int socket);
void remove_client(int id, int socket);
int find_client_id();
char *get_client_memory(int id);
void print_shared_memory();
void *process_response();

typedef struct process_response_args {
    int id;
    int socket;
    int send_pn;
} process_response_args;

int main(int argc, char **argv) {
    int len;
    char buf[20];

    if (get_named_argument(1, argc, argv, buf) != -1 || get_unnamed_argument(1, argc, argv, buf) != -1) {
        printf("Only one argument \"-p\" (port) allowed\n");
        return -1;
    }
    if ((len = get_named_argument(0, argc, argv, buf)) == -1) {
        printf("Missing \"-p\" (port) argument\n");
        return -1;
    }

    char name[20], val[20];
    get_arg_name_and_value(buf, len, name, val);

    if (strcmp(name, "-p") != 0) {
        printf("Only \"-p\" (port) argument allowed\n");
        return -1;
    }

    PORT = atoi(val);
    if (PORT < 1 || PORT > 65535) {
        printf("Invalid port number (%d)\n", PORT);
        return -1;
    }

    int pid = 0;
    printf("SERVER started\n");
    get_shared_memory();

    pid = fork();
    if (pid == 0) {
        start_network();
    }
    else {
        /* vairaki threadi ar gameloop (katrai spēlei savs)*/
        /* checko queue */
        /* ja queue ir 2 speletaji */
        /* uztaisa game state kasti */
        /* uztaisa threadu kas laiz gameloop uz tas kastes */
        gameloop();
    }
    return 0;
}

void get_shared_memory() {
    shared_memory = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    client_count = (int *) shared_memory;
    shared_client_data = (char *) (shared_memory + CLIENT_COUNT_SIZE);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        *get_client_memory(i) = 0;
    }
}

void gameloop() {
    printf("Starting game loop! (It will run forever - use Ctrl+C)\n");
    while (1) {
        // loop forever
    }
}

void start_network() {
    int main_socket;
    struct sockaddr_in server_address;
    int client_socket;
    struct sockaddr_in client_address;
    int client_address_size = sizeof(client_address);

    main_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (main_socket < 0) {
        printf("Error opening main server socket!\n");
        exit(1);
    }
    printf("Main socket created!\n");

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(main_socket,(struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        printf("Error binding the main server socket!\n");
        exit(1);
    }
    printf("Main socket binded!\n");

    if (listen(main_socket, MAX_CLIENTS) < 0) {
        printf("Error listening to socket!\n");
        exit(1);
    }
    printf("Main socket is listening!\n");

    while (1) {
        int new_client_id = 0;
        int cpid = 0;

        client_socket = accept(main_socket, (struct sockaddr *) &client_address, (socklen_t *) &client_address_size);
        if (client_socket < 0) {
            printf("Error accepting client connection! ERRNO=%d\n", errno);
            continue;
        }

        new_client_id = find_client_id();
        if (new_client_id == -1) {
            printf("Limit for maximum clients is reached\n");
            close(client_socket);
            continue;
        }
        *client_count += 1;
        *get_client_memory(new_client_id) = 1;
        cpid = fork();

        if (cpid == 0) {
            close(main_socket);
            cpid = fork();
            if (cpid == 0) {
                process_client(new_client_id, client_socket);
                exit(0);
            }
            else {
                /* orphaning */
                wait(NULL);
                printf("Successfully orphaned client id=%d\n", new_client_id);
                exit(0);
            }
        }
        else {
            close(client_socket);
        }
    }
}

void process_client(int id, int socket) {
    printf("Processing client id=%d, socket=%d\n", id, socket);
    printf("CLIENT count %d\n", *client_count);

    char *client_data = get_client_memory(id);
    char *validity_byte = client_data + sizeof(char) + MAX_NAME_SIZE + KEYBOARD_INPUT_SIZE;
    char *shared_recv_buf = validity_byte + 1;
    *validity_byte = 0;

    char recv_buf[2 * CLIENT_PACKET_MAX_SIZE];

    int32_t *recv_packet_number = (int32_t *) (shared_recv_buf);
    int32_t *recv_pid = (int32_t *) (shared_recv_buf + PACKET_NUMBER_SIZE);
    int64_t *recv_size = (int64_t *) (shared_recv_buf + PACKET_NUMBER_SIZE + PACKET_ID_SIZE);

    int recv_pn = -1, send_pn = -1;

    char c = 0, prevc = 0;
    long encoded_size = 0, decoded_size = 0;
    int len, i = 0;

    process_response_args pra;
    pra.id = id;
    pra.socket = socket;
    pra.send_pn = send_pn;
    pthread_t processing_thread_id;
    if (pthread_create(&processing_thread_id, NULL, process_response, (void *) &pra) != 0) {
        printf("Error creating thread\n");
        remove_client(id, socket);
        exit(1);
    }

    while (1) {
        if ((len = recv(socket, &c, 1, 0)) > 0) {
            if (c == '-') {
                if (prevc == '-') {
                    recv_pn++;
                    // printf("encoded_size: %ld\n", encoded_size);
                    while (*validity_byte == 1)
                        sleep(1.0/100);
                    decoded_size = decode(recv_buf, encoded_size, shared_recv_buf, SHARED_RECV_CLIENT_BUF_SIZE);

                    /* fix endianess for header*/ 
                    *recv_packet_number = network_to_host_int32_t(*recv_packet_number);
                    *recv_pid = network_to_host_int32_t(*recv_pid);
                    *recv_size = network_to_host_int64_t(*recv_size);

                    // printf("decoded_size: %ld\n", decoded_size);
                    *validity_byte = verify_packet(shared_recv_buf, &recv_pn, decoded_size);
                    // printf("validity_byte: %d\n", *validity_byte);
                    encoded_size = 0;
                    i = 0;
                    prevc = 0;
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
            remove_client(id, socket);
            exit(0);
        }
        else {
            printf("Error receiving data from client id=%d", id);
            remove_client(id, socket);
            exit(1);
        }
    }
}

void *process_response(void *arg) {
    int id = ((process_response_args *)arg)->id;
    int socket = ((process_response_args *)arg)->socket;
    int send_pn = ((process_response_args *)arg)->send_pn;
    char *client_data = get_client_memory(id);
    char *validity_byte = client_data + sizeof(char) + MAX_NAME_SIZE + KEYBOARD_INPUT_SIZE;
    char *shared_recv_buf = validity_byte + 1;
    int *pid = (int *) (shared_recv_buf + PACKET_NUMBER_SIZE);
    char *data = shared_recv_buf + PACKET_HEADER_SIZE;

    while(1) {
        if (*validity_byte == 1) {
            switch (*pid) {
                case 1:
                    process_join(data);
                    // print_bytes(shared_recv_buf, JOIN_PACKET_SIZE);
                    send_lobby(0, NULL, ++send_pn, socket);
                    send_lobby(1, "bla bla", ++send_pn, socket);
                    send_lobby(1, "super gars errrrrrors", ++send_pn, socket);
                    send_lobby(1, "super gars errrrrrors", ++send_pn, socket);
                    send_lobby(1, "super gars errrrrrors", ++send_pn, socket);
                    send_lobby(1, "super gars errrrrrors", ++send_pn, socket);
                    send_lobby(1, "super gars errrrrrors", ++send_pn, socket);
                    send_lobby(1, "super gars errrrrrors", ++send_pn, socket);
                    send_lobby(1, "super gars errrrrrors", ++send_pn, socket);
                    break;
                case 3:
                    // print_bytes(shared_recv_buf, GAME_TYPE_PACKET_SIZE);
                    process_game_type(data);
                    // print_bytes(shared_recv_buf, GAME_TYPE_PACKET_SIZE);
                    // send_player_queue(1, "ZBAL", ++send_pn, socket);
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
            *validity_byte = 0;
        }
        sleep(1.0/100);
    }
}

int find_client_id() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (*get_client_memory(i) == 0) {
            return i;
        }
    }
    return -1;
}

void remove_client(int id, int socket) {
    *get_client_memory(id) = 0;
    (*client_count)--;
    close(socket);
}

char *get_client_memory(int id) {
    return shared_client_data + SHARED_CLIENT_MEMORY_SIZE * id;
}

void print_shared_memory() {
    print_bytes(shared_memory, CLIENT_COUNT_SIZE);
    int id;
    for (id = 0; id < MAX_CLIENTS; id++)
        print_bytes(get_client_memory(id), SHARED_CLIENT_MEMORY_SIZE);
}