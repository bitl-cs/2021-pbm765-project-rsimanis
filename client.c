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
void *process_response(); /* called by response processing thread */
int validate_number(char *num);
int validate_ip(char *ip);

char host[30];
int port = 0;
int my_socket = -1;

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


    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, process_response, NULL) != 0) {
        printf("Error creating thread\n");
        return -1;
    }

    // process input and send it to the server
    char inputs[100];
    while (1) {
        if (my_socket != -1) {
            fgets(inputs, 100, stdin);
            send(my_socket, inputs, strlen(inputs), 0);
        }
    }

    return 0;
} 

void *process_response() {
    char *servername;
    struct sockaddr_in remote_address;

    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(port);
    servername = (char *) gethostbyname(host);
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
            fwrite(response, len, 1, stdout);
            fflush(stdout);
        }
        else if (len == 0) {
            printf("Server disconnected");
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