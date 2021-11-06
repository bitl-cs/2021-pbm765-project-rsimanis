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

void process_input(); /* called by input processing thread */
void *process_response(void *vargp); /* called by response processing thread */
int validate_number(char *num);
int validate_ip(char *ip);

char HOST[30];
int PORT = 0;
int my_socket = -1;

int main(int argc, char **argv) {

    int len, i;
    int aCount = 0, pCount = 0;

    char buf[30], name[30], val[30];
    for (i = 0; (len = get_named_argument(i, argc, argv, buf)) != -1; i++) {
        get_arg_name_and_value(buf, len, name, val);
        if (strcmp(name, "-a") == 0) {
            aCount++;
            if (aCount > 1) {
                printf("Too many \"-a\" arguments\n");
                return -1;
            }
            if (!validate_ip(val)) {
                printf("Invalid ip (%s)\n", val);
                return -1;
            }
            strcpy(HOST, val);
        }
        else if (strcmp(name, "-p") == 0) {
            pCount++;
            if (pCount > 1) {
                printf("Too many \"-p\" arguments\n");
                return -1;
            }
            PORT = atoi(val);
            if (PORT < 1 || PORT > 65535) {
                printf("Invalid port number (%d)\n", PORT);
                return -1;
            }
        }
        else {
            printf("Undefined argument (%s)\n", name);
            return -1;
        }
    }
    if (i != 2) {
        printf("Some arguments are missing\n");
        return -1;
    }

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, process_response, NULL);

    process_input();

    return 0;
} 

void process_input() {
    char inputs[100];

    char c = 'z';
    send(my_socket, &c, 1, 0);
    while (1) {
        if (my_socket != -1) {
            scanf("%s", inputs);
            send(my_socket, inputs, strlen(inputs), 0);
        }
    }
}

void *process_response(void *vargp) {
    char *servername;
    struct sockaddr_in remote_address;

    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(PORT);
    servername = gethostbyname(HOST);
    inet_pton(AF_INET, servername, &remote_address.sin_addr);

    if ((my_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("SOCKET ERROR'n");
        exit(1);
    }
    if (connect(my_socket, (struct sockaddr *)&remote_address, sizeof(remote_address)) < 0) {
        printf("ERROR when making connection\n");
        exit(1);
    }
    printf("connected\n");

    char response[100];
    int len = -1;

    while (1) {
        if ((len = recv(my_socket, response, 100, 0)) > 0) {
            for (int i = 0; i < len; i++) {
                putchar(response[i]);
            }
        }
        else if (len < 0) {
            printf("reading error\n");
            exit(1);
        }
    }

    return NULL;
}

/* code from https://www.tutorialspoint.com/c-program-to-validate-an-ip-address */
/* assignment doc did not specify that we need to validate the ip address, but we decided to do it anyways. 
Therefore, we used code from tutorialspoint.com (refence above) */
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
    int i, num, dots = 0;
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