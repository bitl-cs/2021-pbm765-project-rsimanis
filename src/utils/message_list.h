#ifndef _LINKED_LIST_H
#define _LINKED_LIST_H

#include "../game/pong_game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _mnode {
    char message_type;
    char message[MAX_NAME_LENGTH + 2 + MAX_MESSAGE_LENGTH + 1];
    struct _mnode *next;
} mnode;

void init_list(char message_type, char *message, mnode **head);
void pop_front(mnode **head);
void push_back(char message_type, char *message, mnode **head);
void print_list(mnode *head);
void free_list(mnode **head);

#endif