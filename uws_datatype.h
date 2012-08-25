#ifndef __UWS_DATATYPE_H__
#define __UWS_DATATYPE_H__
#include "uws.h"
typedef struct int_node{
    int element;
    struct int_node* next;
} int_node_t;
typedef struct int_queue{
    int length;
    int_node_t* head;
} int_queue_t;
int_queue_t* init_int_queue();
void push_int_queue(int_queue_t *queue, int data);
int pop_int_queue(int_queue_t *queue);
#endif
