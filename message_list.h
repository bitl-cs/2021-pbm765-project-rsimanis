#ifndef _LINKED_LIST_H
#define _LINKED_LIST_H

#include "pong_game.h"

typedef struct _lnode {
    char message[MAX_MESSAGE_SIZE];
    struct _lnode *next;
} lnode;

void init_list(char *message, lnode **head);
void pop_front(lnode **head);
void push_back(char *message, lnode **head);
void print_list(lnode *head);
void free_list(lnode **head);

#endif