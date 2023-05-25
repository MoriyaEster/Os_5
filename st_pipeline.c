#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

// *********************************//
// PartA:
// *********************************//

int isPrime(unsigned int num)
{
    if ((num % 2) == 0)
        return 0;
    for (int i = 3; i < sqrt(num); i += 2)
    {
        if ((num % i) == 0)
            return 0;
    }
    return 1;
}
// *********************************//
// PartB:
// *********************************//

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

void removeQueue(Pqueue queue)
{
    Pnode current = queue->head;
    Pnode to_free = queue->head;
    while (current != NULL)
    {
        current = current->next;
        free(to_free);
        to_free = current;
    }
    free(queue);
}

// *********************************//
// PartC:
// *********************************//

typedef struct AO
{
    Pqueue queue;
    void (*func)(void *);
    pthread_t thread;
} AO, *PAO;

static void *aoThread(void *arg)
{
    PAO ao = (PAO)arg;
    void *task;
    while (task = dequeue(ao->queue))
    {
        ao->func(task);
    }
}

PAO CreateActiveObject()
{
    PAO ao = (PAO)malloc(sizeof(AO));
    if (ao == NULL)
    {
        perror("ao new");
        exit(-1);
    }

    ao->queue = initializeQueue();
    // function;
    pthread_create(&ao->thread, NULL, aoThread, ao);
    return ao;
}

Pqueue getQueue(PAO ao)
{
    return ao->queue;
}

void stop(PAO ao)
{
    pthread_cancel(ao->thread);
    removeQueue(ao->queue);
    free(ao);
}

// *********************************//
// PartD:
// *********************************//

// first AO:
int randNum(int N, int rand_seed)
{
    srand(rand_seed);
    int min = 100000;
    int max = 999999;
    int rand_num;
    for (int i = 0; i < N; i++)
    {
        rand_num = (rand() % (max - min + 1)) + min;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
    {
        printf("usage: st_pipeline <N> <(optional)RAND> \n");
        return (1);
    }
    if (argc == 2)
    {
        if (isdigit(argv[1]))
        {
            printf("usage: st_pipeline <N> <(optional)RAND> \n");
            return (1);
        }
        srand(time(NULL));
        int min = 0;
        int max = 9;
        int rand_seed = (rand() % (max - min + 1)) + min;
    }
    if (argc == 3)
    {
        if (isdigit(argv[1]) && isdigit(argv[2]))
        {
            printf("usage: st_pipeline <N> <(optional)RAND> \n");
            return (1);
        }

        int rand_seed = atoi(argv[2]);
    }
}
