#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include "args.h"

#define MAX_CLIENTS         10
#define SHARED_MEMORY_SIZE  1024

int PORT = 0;

char *shared_memory = NULL;
int *client_count = NULL;
int *shared_data = NULL; /* first int of shared_data is client_count */

void get_shared_memory();
void gameloop();
void start_network();
void process_client(int id, int socket);
void remove_client(int id, int socket);
int find_client_id();

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
        gameloop();
    }
    return 0;
}

void get_shared_memory() {
    shared_memory = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    client_count = (int*) shared_memory;
    shared_data = (int*) (shared_memory + sizeof(int));
    for (int i = 0; i < MAX_CLIENTS; i++) {
        shared_data[i] = -1;
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

    if (bind(main_socket,(struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
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

        client_socket = accept(main_socket, (struct sockaddr*) &client_address, (socklen_t *) &client_address_size);
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
        shared_data[new_client_id] = 0;
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
    int N = 0, len;
    char inputs[100], temp[1000];
    printf("Processing client id=%d, socket=%d\n", id, socket);
    printf("CLIENT count %d\n", *client_count);

    while (1) {
        if ((len = recv(socket, inputs, 1, 0)) > 0) {
            for (int i = 0; i < len; i++) {
                char c = inputs[i];
                if (c == '\n') {
                    printf("Received \\n from client id=%d\n", id);
                    send(socket, &c, 1, 0);
                    printf("Char \\n sent to client id=%d\n", id);
                }
                else {
                    printf("Received %c from client id=%d\n", c, id);
                    N++;
                    shared_data[id] = N;
                    /* Buffering, since system calls are expensive.
                    Note that we can run out of buffer space, so it should be adjusted carefully */
                    for (int i = 0; i < N; i++) {
                        temp[i] = c;
                    }
                    send(socket, temp, N, 0);
                    printf("Char %c sent to client id=%d (%d times) \n", c, id, N);
                }
            }
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

int find_client_id() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (shared_data[i] == -1) {
            return i;
        }
    }
    return -1;
}

void remove_client(int id, int socket) {
    shared_data[id] = -1;
    (*client_count)--;
    close(socket);
}