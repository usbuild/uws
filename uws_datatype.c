#include "uws_datatype.h"
int_queue_t* init_int_queue() {
    int_queue_t* new_queue = (int_queue_t*) uws_malloc(sizeof(int_queue_t));
    new_queue->length = 0;
    new_queue->head = NULL;
    return new_queue;
}
void push_int_queue(int_queue_t *queue, int data) {
    int_node_t** next = &queue->head;
    while(*next != NULL) {
        next = &((*next)->next);
    }
    int_node_t* new_node = (int_node_t*) uws_malloc(sizeof(int_node_t));
    new_node->element = data;
    new_node->next = NULL;
    *next = new_node;
    queue->length++;
}
int pop_int_queue(int_queue_t *queue) {
    int_node_t* tmp = queue->head;
    int ret_val = queue->head->element;
    queue->head =  queue->head->next;
    uws_free(tmp);
    queue->length--;
    return ret_val;
}
