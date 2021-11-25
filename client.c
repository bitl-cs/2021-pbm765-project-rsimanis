#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <ctype.h>
#include <pthread.h>
#include "args.h"
#include "packets.h"

#define VALIDITY_BYTE_SIZE      1
#define SHARED_RECV_BUF_SIZE    SERVER_PACKET_MAX_SIZE
#define SHARED_MEMORY_SIZE      (VALIDITY_BYTE_SIZE + SHARED_RECV_BUF_SIZE)

void process_input(); /* called by input processing thread */
void *receive_response(); /* called by response receiving thread */
void write_circular(char *data, size_t count, char *buf, size_t buflen, size_t offset);
int index_difference(int receiver_index, int processer_index, int max);
int increment_index(int index, int increment, int max);
int min(int x1, int x2);

int validate_number(char *num);
int validate_ip(char *ip);

char host[30];
int port = 0;
int my_socket = -1;
int recv_pn = -1;
int send_pn = -1;

int main(int argc, char **argv) {
    int len, i;
    int a_count = 0, p_count = 0;

    char buf[30], name[30], val[30];
    for (i = 0; (len = get_named_argument(i, argc, argv, buf)) != -1; i++) {
        get_arg_name_and_value(buf, len, name, val);
        if (strcmp(name, "-a") == 0) {
            a_count++;
            if (a_count > 1) {
                printf("Too many \"-a\" arguments\n");
                return -1;
            }
            if (!validate_ip(val)) {
                printf("Invalid ip (%s)\n", val);
                return -1;
            }
            strcpy(host, val);
        }
        else if (strcmp(name, "-p") == 0) {
            p_count++;
            if (p_count > 1) {
                printf("Too many \"-p\" arguments\n");
                return -1;
            }
            port = atoi(val);
            if (port < 1 || port > 65535) {
                printf("Invalid port number (%d)\n", port);
                return -1;
            }
        }
        else {
            printf("Undefined argument (%s)\n", name);
            return -1;
        }
    }
    if (a_count == 0) {
        printf("Need to specify client's IP address\n");
        return -1;
    }
    if (p_count == 0) {
        printf("Need to specify client's port number\n");
        return -1;
    }

    char *servername;
    struct sockaddr_in remote_address;

    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(port);
    servername = (char *) gethostbyname(host);
    inet_pton(AF_INET, servername, &remote_address.sin_addr);

    if ((my_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("SOCKET ERROR\n");
        exit(1);
    }
    if (connect(my_socket, (struct sockaddr *)&remote_address, sizeof(remote_address)) < 0) {
        printf("ERROR when making connection\n");
        exit(1);
    }
    printf("connected\n");

    char *shared_memory = (char *) malloc(SHARED_MEMORY_SIZE); /* one extra for validity byte */
    char *validity_byte = shared_memory;
    char *shared_recv_buf = shared_memory + 1;
    *validity_byte = 0;

    pthread_t receiving_thread_id;
    if (pthread_create(&receiving_thread_id, NULL, receive_response, (void *) shared_memory) != 0) {
        printf("Error creating thread\n");
        return -1;
    }

    // process input and send it to the server
    int *pid = (int *) (shared_recv_buf + PACKET_NUMBER_SIZE);
    char *data = shared_recv_buf + PACKET_HEADER_SIZE;
    send_join("Mr. Rozkalns", ++send_pn, my_socket);

    while(1) {
        if (*validity_byte == 1) {
            switch (*pid) {
                case 2:
                    process_lobby(data);
                    // print_bytes(shared_recv_buf, LOBBY_PACKET_SIZE);
                    send_game_type(1, ++send_pn, my_socket);
                    send_game_type(2, ++send_pn, my_socket);
                    send_game_type(3, ++send_pn, my_socket);
                    send_game_type(4, ++send_pn, my_socket);
                    break;
                case 4:
                    process_player_queue(data);
                    // print_bytes(shared_recv_buf, PLAYER_QUEUE_PACKET_SIZE);
                    // send_player_ready(++send_pn, my_socket);
                    break;
                case 5:
                    process_game_ready(data);
                    // print_bytes(shared_recv_buf, GAME_READY_PACKET_SIZE);
                    // send_player_input('a', ++send_pn, my_socket);
                    break;
                case 7:
                    process_game_state(data);
                    break;
                case 10:
                    process_game_end(data);
                    break;
                default:
                    printf("Invalid pid (%d)\n", *pid);
            }
            *validity_byte = 0;
        }
        sleep(1.0/100);
    }
    return 0;
} 


void *receive_response(void *arg) {
    char *validity_byte = (char *) arg;
    char *shared_recv_buf = ((char *) arg) + 1;
    char recv_buf[2 * CLIENT_PACKET_MAX_SIZE];
    int32_t *recv_packet_number = (int32_t *) (shared_recv_buf);
    int32_t *recv_pid = (int32_t *) (shared_recv_buf + PACKET_NUMBER_SIZE);
    int64_t *recv_size = (int64_t *) (shared_recv_buf + PACKET_NUMBER_SIZE + PACKET_ID_SIZE);
    char c = 0, prevc = 0;
    long encoded_size = 0, decoded_size = 0;
    int i = 0;
    int len;

    while (1) {
        if ((len = recv(my_socket, &c, sizeof(c), 0)) > 0) {
            if (c == '-') {
                if (prevc == '-') {
                    recv_pn++;
                    // printf("encoded_size: %ld\n", encoded_size);
                    while (*validity_byte == 1)
                        sleep(1.0/100);
                    decoded_size = decode(recv_buf, encoded_size, shared_recv_buf, SHARED_RECV_BUF_SIZE);
                    /* fix endianess for header*/ 
                    *recv_packet_number = network_to_host_int32_t(*recv_packet_number);
                    *recv_pid = network_to_host_int32_t(*recv_pid);
                    *recv_size = network_to_host_int64_t(*recv_size);

                    // printf("decoded_size: %ld\n", decoded_size);
                    *validity_byte = verify_packet(shared_recv_buf, &recv_pn, decoded_size);
                    // printf("validity_byte: %d\n", *validity_byte);
                    // fflush(stdout);
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


/* --- EXTRA UTILITIES --- */
/* code from https://www.tutorialspoint.com/c-program-to-validate-an-ip-address */
/* Assignment document did not specify that we need to validate the ip address, but we decided to do it anyway. 
However, since it was not specified, we did not code this ourselves, but rather took the code from tutorialspoint.com (refence above) */
int validate_number(char *str) {
    while (*str) {
        if(!isdigit(*str)){ //if the character is not a number, return false
            return 0;
        }
        str++; //point to next character
    }
    return 1;
}

/* code from https://www.tutorialspoint.com/c-program-to-validate-an-ip-address */
int validate_ip(char *ip) { //check whether the IP is valid or not
    int num, dots = 0;
    char copy[30];

    if (ip == NULL)
        return 0;

    strcpy(copy, ip);
    char *ptr;
    ptr = strtok(copy, "."); //cut the string using dor delimiter
    if (ptr == NULL)
        return 0;
    while (ptr) {
        if (!validate_number(ptr)) //check whether the sub string is
            return 0;
        num = atoi(ptr); //convert substring to number
        if (num >= 0 && num <= 255) {
            ptr = strtok(NULL, "."); //cut the next part of the string
            if (ptr != NULL)
            dots++; //increase the dot count
        } else
            return 0;
        }
        if (dots != 3) { //if the number of dots are not 3, return false
            return 0;
        }
    return 1;
}
