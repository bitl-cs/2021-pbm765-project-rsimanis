#include "pong_networking.h"
#include "args.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>


/* initialization */
int get_port_parameter(int argc, char** argv, char* port){
    /* This function either leaves the port string as is (Should be set to DEFAULT_PORT), or changes it, if parameter -p=NEWPORT is set */
    int len, i, count;
    char buf[512], name[256], val[256];

    count = 0;
    for (i = 0; (len = get_named_argument(i, argc, argv, buf)) != -1; i++) {
        get_arg_name_and_value(buf, len, name, val);
        if (strcmp(name, "-p") == 0) {
            count++;
            if (count > 1) {
                printf("Too many \"-p\" arguments\n");
                return -1;
            }
            int portnr = atoi(val);
            if (portnr < 1 || portnr > 65535) {
                printf("Invalid port (%d)\n", portnr);
                return -1;
            }
            strcpy(port, val);
        }
        else {
            printf("Undefined argument (%s)\n", name);
            return -1;
        }
    }
    if (count == 0)
        return -1;
    return 0;
}

int get_host_parameter(int argc, char** argv, char* host){
    /* This function either leaves the host string as is (Should be set to DEFAULT_IP), or changes it, if parameter -h=NEWHOST is set */
    int len, i, count;
    char buf[30], name[30], val[30];

    count = 0;
    for (i = 0; (len = get_named_argument(i, argc, argv, buf)) != -1; i++) {
        get_arg_name_and_value(buf, len, name, val);
        if (strcmp(name, "-a") == 0) {
            count++;
            if (count > 1) {
                printf("Too many \"-a\" arguments\n");
                return -1;
            }
            if (!validate_ip(val)) {
                printf("Invalid ip (%s)\n", val);
                return -1;
            }
            strcpy(host, val);
        }
        else {
            printf("Undefined argument (%s)\n", name);
            return -1;
        }
    }
    if (count == 0)
        return -1;
    return 0;
}

int get_server_socket(char *port){
    int server_socket;
    int opt_value = 1;
    struct addrinfo hints, *list_of_addresses, *a;

    printf("Opening server socket on port %s\n", port);
    memset(&hints, 0, sizeof(struct addrinfo));  /* create empty hints structure */
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;  /* protocol independent stream */
    hints.ai_flags = AI_PASSIVE; /* passive sockets - server */

    if (getaddrinfo(NULL, port, &hints, &list_of_addresses) != 0) {
        printf("Port translating failed!\n");
        return -1;
    }
    if(list_of_addresses == NULL) 
        printf("NO valid addresses!\n");

    /* Iterate over all addresses available to the server and try to connect/bind */
    for(a = list_of_addresses; a != NULL; a = a->ai_next) {
        printf("... Creating socket ...\n");
        if((server_socket = socket(a->ai_family, a->ai_socktype, a->ai_protocol)) < 0)
            continue; /* failed on this address, try another one */

        /* get rid of "Address already in use" produced by previous bind() calls */
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const void *) &opt_value, sizeof(int));
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, (const void *) &opt_value, sizeof(int));
        printf("... Binding socket ...\n");
        if(bind(server_socket, a->ai_addr, a->ai_addrlen) == 0)
            break; /* successfully connected */
        printf("ERROR binding server! ERRNO=%d\n", errno);
        close(server_socket); /* could not connect to socket - close and try another one */
    }

    /* clean up */
    if (a == NULL) { /* All connections failed */
        printf("No connection was made - cleaning up...\n");
        freeaddrinfo(list_of_addresses);
        return -1;
    } 
    freeaddrinfo(list_of_addresses);

    /* start listening to the socket */
    printf("... Listening to socket ...\n");
    if (listen(server_socket, MAX_CLIENTS) < 0){
        close(server_socket);
        return -1;
    }
    printf("Server socket successfully opened - listening...\n");
    return server_socket; /* a connection succeeded, so return */
}

int get_client_socket(char *host, char *port){
    int client_socket = -1;
    struct addrinfo hints, *list_of_addresses, *a;

    printf("Opening client socket to %s:%s\n", host, port);
    memset(&hints, 0, sizeof(struct addrinfo));  /* create empty hints structure */
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;  /* protocol independent stream */

    if (getaddrinfo(host, port, &hints, &list_of_addresses) != 0) {
        printf("Host and port translating failed!\n");
        return -1;
    }
    if (list_of_addresses == NULL)
        printf("NO valid addresses!\n");

    /* Iterate over all addresses available to the server and try to connect/bind */
    for (a = list_of_addresses; a != NULL; a = a->ai_next) {
        printf("... Creating socket ...\n");
        if((client_socket = socket(a->ai_family, a->ai_socktype, a->ai_protocol))<0)
            continue; /* failed on this address, try another one */
        if(connect(client_socket, a->ai_addr, a->ai_addrlen)!=-1)
            break; /* successfully connected */
        printf("ERROR conneting to socket! ERRNO=%d\n", errno);
        close(client_socket); /* could not connect to socket - close and try another one */
    }
    /* clean up */
    if(a == NULL) { /* All connections failed */
        printf("No connection was made - cleaning up...\n");
        freeaddrinfo(list_of_addresses);
        return -1;
    } 
    freeaddrinfo(list_of_addresses);
    return client_socket;
}


/* packet processing */
/* send generic packet (data must be in network endianess) */
void send_packet(uint32_t pn, unsigned char pid, int32_t psize, char *data, size_t datalen, char *buf, size_t buf_size, char *final_buf, size_t final_buf_size, int socket) {
    size_t offset = 0;

    /* add packet number */
    offset += insert_uint32_t_as_big_endian(pn, buf, buf_size, offset);
    // print_bytes(packet, offset);
    // printint("offset", offset);

    /* add packet id */
    offset += insert_char(pid, buf, buf_size, offset); 
    // print_bytes(packet, offset);
    // printint("offset", offset);

    /* add the size of data segment */
    offset += insert_int32_t_as_big_endian(psize, buf, buf_size, offset);
    // print_bytes(packet, offset);
    // printint("offset", offset);

    /* add data (assumes that data is in network byte order) */
    offset += insert_bytes(data, datalen, buf, buf_size, offset);
    // print_int("datalen", *(send_mem_cfg->datalen));
    // print_bytes(packet, offset);
    // printint("offset", offset);

    /* fill the unused data segment with null bytes */
    offset += insert_null_bytes(psize - datalen, buf, buf_size, offset);
    // print_int("rem datalen", psize - *(send_mem_cfg->datalen));
    // print_int("offset", offset);
    // print_bytes(packet, offset);
    // printint("offset", offset);

    /* add checksum */
    offset += insert_char(xor_checksum(buf, PACKET_HEADER_SIZE + datalen), buf, buf_size, offset);
    // printf("before encoding: \n");
    // print_bytes(buf, offset);
    // printint("offset", offset);

    /* encode */
    offset = encode(buf, offset, final_buf, final_buf_size - PACKET_SEPARATOR_SIZE);
    // printf("after encoding: \n");
    // print_bytes(final_packet, offset);

    /* add separator */
    offset += insert_separator(final_buf, final_buf_size, offset);
    // printf("packet send: \n");
    // print_bytes(final_buf, offset);

    /* send packet to socket */ 
    send(socket, final_buf, offset, 0);
}


/* debug */
void print_bytes(void *start, size_t len) {
    size_t i;
    char *s = (char *) start;

    for (i = 0; i < len; i++)
        printf("%02hhx ", s[i]);
    putchar('\n');
}

void print_bytes_full(void *start, size_t len) {
    size_t i;
    char *s = (char *) start;

    if (len > 999) {
        printf("Cannot print more than 999 bytes! You asked for %lu\n", len);
        return;
    }

    printf("Printing %lu bytes...\n", len);
    printf("[NPK] [C] [HEX] [DEC] [ BINARY ]\n");
    printf("================================\n");
    for (i = 0; i < len; i++) {
        printf(" %3lu | %c | %02X | %3d | %c%c%c%c%c%c%c%c\n", i, printable_char(s[i]), s[i], s[i],
            s[i] & 0x80 ? '1' : '0',
            s[i] & 0x40 ? '1' : '0',
            s[i] & 0x20 ? '1' : '0',
            s[i] & 0x10 ? '1' : '0',
            s[i] & 0x08 ? '1' : '0',
            s[i] & 0x04 ? '1' : '0',
            s[i] & 0x02 ? '1' : '0',
            s[i] & 0x01 ? '1' : '0'
        );
    }
}

void here() {
    printf("here\n");
}

void printint(char *tag, int x) {
    printf("%s: %d\n", tag, x);
}

void printfloat(char *tag, float x) {
    printf("%s: %f\n", tag, x);
}

void printstr(char *str) {
    printf("%s\n", str);
}

void printstrt(char *tag, char *str) {
    printf("%s: %s\n", tag, str);
}

/* utilities */
int encode(char *data, size_t datalen, char *buf, size_t buflen) {
    size_t i, buf_i;

    buf_i = 0;
    for (i = 0; i < datalen && buf_i < buflen; i++) {
        if (data[i] == '-') {
            buf[buf_i++] = '?';
            if (buf_i < buflen)
                buf[buf_i++] = '-';
            else
                break;
        }
        else if (data[i] == '?') {
            buf[buf_i++] = '?';
            if (buf_i < buflen)
                buf[buf_i++] = '*';
            else
                break;
        }
        else
            buf[buf_i++] = data[i];
    }
    return buf_i;
}

int decode(char *data, size_t datalen, char *buf, size_t buflen) {
    size_t i, buf_i;
    char next;

    buf_i = 0;
    for (i = 0; i < datalen && buf_i < buflen; i++) {
        if (data[i] == '?') {
            if (i < datalen - 1) {
                next = data[++i];
                if (next == '-')
                    buf[buf_i++] = '-';
                else if (next == '*')
                    buf[buf_i++] = '?';
                else 
                    break;
            }
            else
                break;
        }
        else
            buf[buf_i++] = data[i];
    }
    return buf_i;
}

char xor_checksum(char *data, size_t len) {
    char rez;
    size_t i;

    if (len == 0)
        return 0;

    rez = data[0];
    for (i = 1; i < len; i++)
        rez ^= data[i];
    return rez;
}

char verify_packet(uint32_t exp_pn, char *packet, int32_t decoded_size) {
    uint32_t pn = big_endian_to_host_uint32_t(*((uint32_t *) packet));
    int32_t psize = decoded_size - PACKET_HEADER_SIZE - PACKET_FOOTER_SIZE;
    int32_t exp_psize = big_endian_to_host_uint32_t(*((int32_t *) (packet + PACKET_NUMBER_SIZE + PACKET_ID_SIZE)));
    char checksum = packet[decoded_size - PACKET_CHECKSUM_SIZE];

    // printf("%d %d\n", pn, exp_pn);
    // printf("%d %d\n", psize, exp_psize);
    // printf("%d %d\n", checksum, xor_checksum(packet, PACKET_HEADER_SIZE + psize));


    return ((pn >= exp_pn) || ((exp_pn >= UINT32_MAX - 50) && (pn >= 0))) &&
            (psize == exp_psize) &&
            (checksum == xor_checksum(packet, PACKET_HEADER_SIZE + psize));
}

size_t insert_bytes(char *data, size_t datalen, char *buf, size_t buflen, size_t offset) {
    size_t i;
    for (i = 0; i < datalen && i + offset < buflen; i++)
        buf[i + offset] = data[i];
    return i;
}

size_t insert_int32_t(int32_t x, char *buf, size_t buflen, size_t offset) {
    return insert_bytes((char *) &x, sizeof(int32_t), buf, buflen, offset);
}

size_t insert_int64_t(int64_t x, char *buf, size_t buflen, size_t offset) {
    return insert_bytes((char *) &x, sizeof(int64_t), buf, buflen, offset);
}

size_t insert_char(char x, char *buf, size_t buflen, size_t offset) {
    return insert_bytes(&x, sizeof(char), buf, buflen, offset);
}

size_t insert_str(char *str, size_t strlen, char *buf, size_t buflen, size_t offset) {
    size_t off;

    off = insert_bytes(str, strlen, buf, buflen, offset);
    off += insert_null_bytes(1, buf, buflen, offset + off);
    return off;
}

size_t insert_null_bytes(int count, char *buf, size_t buflen, size_t offset) {
    int i;
    size_t rez = 0;

    for (i = 0; i < count && i + offset < buflen; i++) {
        buf[i + offset] = '\0';
        rez++;
    }
    return rez;
}

size_t insert_separator(char *buf, size_t buflen, size_t offset) {
    return insert_bytes(PACKET_SEPARATOR, PACKET_SEPARATOR_SIZE, buf, buflen, offset);
}


size_t insert_bytes_as_big_endian(char *data, size_t datalen, char *buf, size_t buflen, size_t offset) {
    size_t i;

    if (is_little_endian_system()) {
        /* reverse */
        for (i = 0; i < datalen && i + offset < buflen; i++)
            buf[i + offset] = data[datalen - i - 1];
    }
    else {
        /* store it as it is */
        for (i = 0; i < datalen && i + offset < buflen; i++)
            buf[i + offset] = data[i];
    }
    return i;
}

size_t insert_short_as_big_endian(short x, char *buf, size_t buflen, size_t offset) {
    return insert_bytes_as_big_endian((char *) &x, sizeof(x), buf, buflen, offset);
}

size_t insert_int_as_big_endian(int x, char *buf, size_t buflen, size_t offset) {
    return insert_bytes_as_big_endian((char *) &x, sizeof(x), buf, buflen, offset);
}

size_t insert_long_as_big_endian(long x, char *buf, size_t buflen, size_t offset) {
    return insert_bytes_as_big_endian((char *) &x, sizeof(x), buf, buflen, offset);
}

size_t insert_float_as_big_endian(float x, char *buf, size_t buflen, size_t offset) {
    return insert_bytes_as_big_endian((char *) &x, sizeof(x), buf, buflen, offset);
}

size_t insert_double_as_big_endian(double x, char *buf, size_t buflen, size_t offset) {
    return insert_bytes_as_big_endian((char *) &x, sizeof(x), buf, buflen, offset);
}

size_t insert_int32_t_as_big_endian(int32_t x, char *buf, size_t buflen, size_t offset) {
    return insert_bytes_as_big_endian((char *) &x, sizeof(x), buf, buflen, offset);
}

size_t insert_uint32_t_as_big_endian(uint32_t x, char *buf, size_t buflen, size_t offset) {
    return insert_bytes_as_big_endian((char *) &x, sizeof(x), buf, buflen, offset);
}

size_t insert_int64_t_as_big_endian(int64_t x, char *buf, size_t buflen, size_t offset) {
    return insert_bytes_as_big_endian((char *) &x, sizeof(x), buf, buflen, offset);
}

size_t insert_uint64_t_as_big_endian(uint64_t x, char *buf, size_t buflen, size_t offset) {
    return insert_bytes_as_big_endian((char *) &x, sizeof(x), buf, buflen, offset);
}


int is_little_endian_system() {
    volatile uint32_t i = 0x01234567;
    return (*((uint8_t *) (&i))) == 0x67;
}

/* if needed reverses "len" bytes starting at "start", so that the order is BIG ENDIAN */
void host_to_big_endian_bytes(char *start, size_t len) {
    size_t i;
    char temp;

    /* if already running on big endian machine */
    if (!is_little_endian_system())
        return;

    /* reverse byte order */
    for (i = 0; i < len / 2; i++) {
        temp = start[i];
        start[i] = start[len - i - 1];
        start[len - i - 1] = temp;
    }
}

short host_to_big_endian_short(short x) {
    host_to_big_endian_bytes((char *) &x, sizeof(x));
    return x;
}

int host_to_big_endian_int(int x) {
    host_to_big_endian_bytes((char *) &x, sizeof(x));
    return x;
}

long host_to_big_endian_long(long x) {
    host_to_big_endian_bytes((char *) &x, sizeof(x));
    return x;
}

float host_to_big_endian_float(float x) {
    host_to_big_endian_bytes((char *) &x, sizeof(x));
    return x;
}

double host_to_big_endian_double(double x) {
    host_to_big_endian_bytes((char *) &x, sizeof(x));
    return x;
}

int32_t host_to_big_endian_int32_t(int32_t x) {
    host_to_big_endian_bytes((char *) &x, sizeof(x));
    return x;
}

uint32_t host_to_big_endian_uint32_t(uint32_t x) {
    host_to_big_endian_bytes((char *) &x, sizeof(x));
    return x;
}

int64_t host_to_big_endian_int64_t(int64_t x) {
    host_to_big_endian_bytes((char *) &x, sizeof(x));
    return x;
}

uint64_t host_to_big_endian_uint64_t(uint64_t x) {
    host_to_big_endian_bytes((char *) &x, sizeof(x));
    return x;
}

/* if needed reverses "len" bytes starting at "start", so that the order is LITTLE ENDIAN */
void big_endian_to_host_bytes(char *start, size_t len) {
    size_t i;
    char temp;

    /* if already running on big endian machine */
    if (!is_little_endian_system())
        return;

    /* reverse byte order */
    for (i = 0; i < len / 2; i++) {
        temp = start[i];
        start[i] = start[len - i - 1];
        start[len - i - 1] = temp;
    }
}

short big_endian_to_host_short(short x) {
    big_endian_to_host_bytes((char *) &x, sizeof(x));
    return x;
}

int big_endian_to_host_int(int x) {
    big_endian_to_host_bytes((char *) &x, sizeof(x));
    return x;
}

long big_endian_to_host_long(long x) {
    big_endian_to_host_bytes((char *) &x, sizeof(x));
    return x;
}

float big_endian_to_host_float(float x) {
    big_endian_to_host_bytes((char *) &x, sizeof(x));
    return x;
}

double big_endian_to_host_double(double x) {
    big_endian_to_host_bytes((char *) &x, sizeof(x));
    return x;
}

int32_t big_endian_to_host_int32_t(int32_t x) {
    big_endian_to_host_bytes((char *) &x, sizeof(x));
    return x;
}

uint32_t big_endian_to_host_uint32_t(uint32_t x) {
    big_endian_to_host_bytes((char *) &x, sizeof(x));
    return x;
}

int64_t big_endian_to_host_int64_t(int64_t x) {
    big_endian_to_host_bytes((char *) &x, sizeof(x));
    return x;
}

uint64_t big_endian_to_host_uint64_t(uint64_t x) {
    big_endian_to_host_bytes((char *) &x, sizeof(x));
    return x;
}


char printable_char(char c) {
    if (isprint(c) != 0)
        return c;
    return ' ';
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