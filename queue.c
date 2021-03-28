#include "queue.h"

#include <pthread.h>
#include <stdlib.h>

/* Defines the struct for a node.
 * void *value is the value contained in the node.
 * node *next_ptr is the node next-in-line in the queue.
 */
typedef struct node {
    void *value;
    struct node *next_ptr;
} node_t;

/* Defines the struct for a queue.
 * node_t *head is the head of the queue.
 * node_t *tail is the tail of the queue.
 * pthread_mutex_t lock is the lock
 * pthread_cond_t cond is the cond
 */
typedef struct queue {
    node_t *head;
    node_t *tail;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} queue_t;

/* Initializes the queue. Sets the head and tail ptr to NULL.
 * Returns a pointer to the queue (its head)
 */
queue_t *queue_init(void) {
    queue_t *queue_ptr = (queue_t *) calloc(1, sizeof(queue_t));
    if (queue_ptr == NULL) {
        return NULL;
    }
    queue_ptr->head = NULL;
    queue_ptr->tail = NULL;
    pthread_mutex_init(&queue_ptr->lock, NULL);
    pthread_cond_init(&queue_ptr->cond, NULL);
    return queue_ptr;
}

/* Adds a node to the tail of the queue. */
void queue_enqueue(queue_t *queue, void *value) {
    pthread_mutex_lock(&queue->lock);
    node_t *new_node = (node_t *) malloc(sizeof(node_t));
    new_node->value = value;
    new_node->next_ptr = NULL;
    if ((queue->head == NULL) && (queue->tail == NULL)) {
        queue->head = new_node;
        queue->tail = new_node;
    }
    else {
        queue->tail->next_ptr = new_node;
        queue->tail = new_node;
    }
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->lock);
}

/* Removes a node from the queue. Frees that node.
 * Returns the value contained in that node. */
void *queue_dequeue(queue_t *queue) {
    if (queue == NULL) {
        return NULL;
    }
    pthread_mutex_lock(&queue->lock);
    while ((queue->head == NULL) && (queue->tail == NULL)) {
        pthread_cond_wait(&queue->cond, &queue->lock);
    }
    node_t *first_head = queue->head;
    void *value = first_head->value;

    if (queue->head == queue->tail) {
        free(queue->head);
        queue->head = NULL;
        queue->tail = NULL;
        pthread_mutex_unlock(&queue->lock);
        return value;
    }
    queue->head = queue->head->next_ptr;
    free(first_head);
    pthread_mutex_unlock(&queue->lock);
    return value;
}

/* Frees the queue. */
void queue_free(queue_t *queue) {
    free(queue);
}
