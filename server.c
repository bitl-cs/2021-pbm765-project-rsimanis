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

int main(int argc, char **argv) {
    int len;
    char buf[20];

    if ((len = get_named_argument(0, argc, argv, buf)) < 0) {
        printf("Must specify valid port number!\n");
        return -1;
    }

    char name[20], val[20];
    get_arg_name_and_value(buf, len, name, val);
    PORT = atoi(val);
    if (PORT < 1 || PORT > 65535) {
        printf("Invalid port number (%d)\n", PORT);
        return -1;
    }

    int pid = 0;
    int i;
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
}

void gameloop() {
    printf("Starting game loop! (It will run forever - use Ctrl+C)\n");
    while (1) {

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

        client_socket = accept(main_socket, (struct sockaddr*) &client_address, &client_address_size);
        if (client_socket < 0) {
            printf("Error accepting client connection! ERRNO=%d\n", errno);
            continue;
        }

        new_client_id = *client_count;
        *client_count += 1;
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
                printf("Successfully orphaned client %d\n", new_client_id);
                exit(0);
            }
        }
        else {
            close(client_socket);
        }
    }
}

void process_client(int id, int socket) {
    int N = 0;
    char c;
    printf("Processing client id=%d, socket=%d\n", id, socket);
    printf("CLIENT count %d\n", *client_count);

    while (1) {
        if (read(socket, &c, 1) == 1 && c != '\n') {
            printf("received %c from client %d\n", c, id);
            N++;
            shared_data[id] = N;
            for (int i = 0; i < N; i++) {
                if (write(socket, &c, 1) != 1) {
                    printf("sending failed\n");
                    exit(1);
                }
                printf("char %c sent\n", c);
            }
        }
        else if (c == '\n') {
            write(socket, &c, 1);
        }
    }
}