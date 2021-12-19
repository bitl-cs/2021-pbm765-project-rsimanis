#include "pong_message_list.h"


void init_list(char message_type, char *message, mnode **head) {
    mnode *h;

    h = *head = (mnode *) malloc(sizeof(mnode));
    h->message_type = message_type;
    strcpy(h->message, message);
    h->next = NULL;
}

void pop_front(mnode **head) {
    mnode *h;
    
    h = *head;
    if (h->next != NULL)
        *head = h->next;
    else
        *head = NULL;
    free(h);
}

void push_back(char message_type, char *message, mnode **head) {
    mnode *t;

    if (*head == NULL) {
        init_list(message_type, message, head);
        return;
    }

    t = *head;
    while (t->next != NULL)
        t = t->next;

    t->next = (mnode *) malloc(sizeof(mnode));
    t->next->message_type = message_type;
    strcpy(t->next->message, message);
    t->next->next = NULL;
}

void print_list(mnode *head) {
    while (head != NULL) {
        printf("%s\n", head->message);
        head = head->next;
    }
}

void free_list(mnode **head) {
    mnode *node, *temp;

    node = *head;
    while (node != NULL) {
        temp = node->next;
        free(node);
        node = temp;
    }
    *head = NULL;
}