#ifndef _LINKED_LIST_H
#define _LINKED_LIST_H

#include "../game/pong_game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _lnode {
    char message_type;
    char message[MAX_MESSAGE_SIZE];
    struct _lnode *next;
} lnode;

void init_list(char message_type, char *message, lnode **head);
void pop_front(lnode **head);
void push_back(char message_type, char *message, lnode **head);
void print_list(lnode *head);
void free_list(lnode **head);

#endif