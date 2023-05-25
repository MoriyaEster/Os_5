#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct Node
{
    void *task;
    struct Node *next;
} Node, *Pnode;

// Define the queue structure
typedef struct Queue
{
    Pnode head;
    int size;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Queue, *Pqueue;

Pnode new_node(void *task)
{
    Pnode new_node = (Pnode)malloc(sizeof(Node));
    if (new_node == NULL)
    {
        perror("Pnode new");
        exit(-1);
    }
    new_node->task = task;
    new_node->next = NULL;
    return new_node;
}

// Initialize the queue
Pqueue initializeQueue()
{
    Pqueue new_queue = (Pqueue)malloc(sizeof(Queue));
    if (new_queue == NULL)
    {
        perror("Pqueue new");
        exit(-1);
    }
    new_queue->head = NULL;
    new_queue->size = 0;
    pthread_mutex_init(&new_queue->mutex, NULL);
    pthread_cond_init(&new_queue->cond, NULL);

    return new_queue;
}

// Enqueue an task
void enqueue(Pqueue queue, void *task)
{
    pthread_mutex_lock(&queue->mutex);
    Pnode new_task = new_node(task);

    if (queue->size == 0)
    {
        queue->head = new_task;
        queue->size = 1;
    }
    else
    {
        new_task->next = queue->head;
        queue->head = new_task;
        queue->size++;
    }
    pthread_mutex_unlock(&queue->mutex);
    pthread_cond_signal(&queue->cond);
}

// Dequeue an task
void *dequeue(Pqueue queue)
{
    pthread_mutex_lock(&queue->mutex);
    while (queue->size <= 0)
    {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }
    Pnode current = queue->head;

    while (current->next != NULL)
    {
        if (current->next->next == NULL)
        {
            break;
        }
        current = current->next;
    }
    void *task = current->next->task;
    Pnode to_free = current->next;
    current->next = NULL;
    queue->size--;
    free(to_free);

    pthread_mutex_unlock(&queue->mutex);
    return task;
}
