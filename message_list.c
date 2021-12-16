#include "message_list.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void init_list(char message_type, char *message, lnode **head) {
    lnode *h;

    h = *head = (lnode *) malloc(sizeof(lnode));
    h->message_type = message_type;
    strcpy(h->message, message);
    h->next = NULL;
}

void pop_front(lnode **head) {
    lnode *h;
    
    h = *head;
    if (h->next != NULL)
        *head = h->next;
    else
        *head = NULL;
    free(h);
}

void push_back(char message_type, char *message, lnode **head) {
    lnode *t;

    if (*head == NULL) {
        init_list(message_type, message, head);
        return;
    }

    t = *head;
    while (t->next != NULL)
        t = t->next;

    t->next = (lnode *) malloc(sizeof(lnode));
    t->next->message_type = message_type;
    strcpy(t->next->message, message);
    t->next->next = NULL;
}

void print_list(lnode *head) {
    while (head != NULL) {
        printf("%s\n", head->message);
        head = head->next;
    }
}

void free_list(lnode **head) {
    lnode *node, *temp;

    node = *head;
    while (node != NULL) {
        temp = node->next;
        free(node);
        node = temp;
    }
    *head = NULL;
}