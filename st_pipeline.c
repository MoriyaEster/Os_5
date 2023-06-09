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
    if (num == 2)
    {
        printf("true\n");
        return 1;
    }
    if ((num % 2) == 0)
    {
        printf("false\n");
        return 0;
    }

    for (int i = 3; i <= sqrt(num); i += 2)
    {
        if ((num % i) == 0)
        {
            printf("false\n");
            return 0;
        }
    }
    printf("true\n");
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

    new_task->next = queue->head;
    queue->head = new_task;
    queue->size++;

    pthread_mutex_unlock(&queue->mutex);
    pthread_cond_signal(&queue->cond);
}


// Dequeue an task
void *dequeue(Pqueue queue)
{
    pthread_mutex_lock(&queue->mutex);
    while (queue->size <= 0)
    {
        pthread_mutex_unlock(&queue->mutex);
        pthread_cond_wait(&queue->cond, &queue->mutex);
        pthread_mutex_lock(&queue->mutex);
    }

    Pnode current = queue->head;
    Pnode previous = NULL;
    while (current->next != NULL)
    {
        previous = current;
        current = current->next;
    }

    void *task = current->task;
    if (previous != NULL)
    {
        previous->next = NULL;
    }
    else
    {
        queue->head = NULL;
    }
    queue->size--;

    free(current);
    pthread_mutex_unlock(&queue->mutex);
    return task;
}

void removeQueue(Pqueue queue)
{
    Pnode current = queue->head;
    while (current != NULL)
    {
        Pnode next = current->next;
        free(current);
        current = next;
    }
    free(queue);
}

// *********************************//
// PartC:
// *********************************//

typedef struct AO
{
    Pqueue queue;
    void (*func)(struct AO *, void *);
    pthread_t thread;
    struct AO *next;
} AO, *PAO;

static void *aoThread(void *arg)
{
    PAO ao = (PAO)arg;
    void *task;
    while ((task = dequeue(ao->queue)) != NULL)
    {
        ao->func(ao->next, task);
    }
    return task;
}


PAO CreateActiveObject(PAO next, void (*func)(PAO, void *))
{
    PAO ao = (PAO)malloc(sizeof(AO));
    if (ao == NULL)
    {
        perror("ao new");
        exit(-1);
    }

    ao->queue = initializeQueue();
    ao->next = next;
    ao->func = func;
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
void func1(PAO next, void *rand_seed)
{
    int num = *(int *)rand_seed;
    srand(num);
    int min = 100000;
    int max = 999999;
    int rand_num;
    rand_num = (rand() % (max - min + 1)) + min;
    void *task = &rand_num;
    usleep(1000);
    enqueue(getQueue(next), task);
}
// seconed AO:
void func2(PAO next, void *task)
{
    int num = *(int *)task;
    printf("%d\n", num);
    isPrime(num);
    num += 11;
    void *task2 = &num;
    enqueue(getQueue(next), task2);
}

// third AO:
void func3(PAO next, void *task)
{
    int num = *(int *)task;
    printf("%d\n", num);
    isPrime(num);
    num -= 13;
    void *task3 = &num;
    enqueue(getQueue(next), task3);
}

// fourth AO:
void func4(PAO next, void *task)
{
    int num = *(int *)task;
    printf("%d\n", num);
    num += 2;
    printf("%d\n", num);
}

int main(int argc, char *argv[])
{
    int rand_seed = 0;
    if (argc < 2 || argc > 3)
    {
        printf("usage: st_pipeline <N> <(optional)RAND> \n");
        return (1);
    }
    if (argc == 2)
    {
        srand(time(NULL));
        int min = 0;
        int max = 9;
        rand_seed = (rand() % (max - min + 1)) + min;
    }
    if (argc == 3)
    {

        rand_seed = atoi(argv[2]);
    }

    PAO fourthAO = CreateActiveObject(NULL, func4);
    PAO thirdAO = CreateActiveObject(fourthAO, func3);
    PAO secondAO = CreateActiveObject(thirdAO, func2);
    PAO firstAO = CreateActiveObject(secondAO, func1);

    void *task = &rand_seed;

    for (int i = 0; i < atoi(argv[1]); i++)
    {
        int rand_seed = *(int *)task;
        enqueue(getQueue(firstAO), &rand_seed);
    }

    while (firstAO->queue->size > 0)
    {
    }
    sleep(1);
    stop(firstAO);

    while (secondAO->queue->size > 0)
    {
    }
    stop(secondAO);

    while (thirdAO->queue->size > 0)
    {
    }
    stop(thirdAO);

    while (fourthAO->queue->size > 0)
    {
    }
    stop(fourthAO);
}
