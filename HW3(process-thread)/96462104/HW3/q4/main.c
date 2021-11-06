#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>


const int M = 10000;

struct task 
{
    int id;
    int value;
    int atime;
    int unit_count;
    int *unit_id;
    int done;
    struct timeval start;
};

typedef struct task task;

struct Queue 
{
    int front, rear, size;
    unsigned capacity;
    task *array;
};

struct Queue *createQueue(unsigned capacity) 
{
    struct Queue *queue = (struct Queue *) malloc(
            sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->array = (task *) malloc(
            queue->capacity * sizeof(task));
    return queue;
}

int isFull(struct Queue *queue) 
{
    return (queue->size == queue->capacity);
}

int isEmpty(struct Queue *queue) 
{
    return (queue->size == 0);
}

void enqueue(struct Queue *queue, task item) 
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}

task dequeue(struct Queue *queue) 
{
    if (isEmpty(queue)) {
        task *t = (task *) malloc(sizeof(task));
        return *t;
    }
    task item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

struct Queue *queue;
struct Queue *unit0;
struct Queue *unit1;
struct Queue *unit2;
struct Queue *unit3;
struct Queue *unit4;

int true = 1;


void *Unit0(void *arg) 
{
    while (true) 
    {
        if (!isEmpty(unit0)) 
        {
            printf("Run in Unit 0\n");
            task temp = dequeue(unit0);
            temp.done++;
            temp.value += 7;
            temp.value %= M;
        }
    }
}

void *Unit1(void *arg) 
{
    while (true) 
    {
        if (!isEmpty(unit1)) 
        {
            printf("Run in Unit 1\n");
            task temp = dequeue(unit1);
            temp.done++;
            temp.value *= 2;
            temp.value %= M;
            
        }
    }
}

void *Unit2(void *arg) 
{
    while (true) 
    {
        if (!isEmpty(unit3)) 
        {
            printf("Run in Unit 2\n");
            task temp = dequeue(unit3);
            temp.done++;
            temp.value *= temp.value * temp.value * temp.value * temp.value * temp.value;
            temp.value %= M;
        }
    }
}

task getCopy(task *t)
{
    task *new_t = malloc(sizeof(struct task));
    *new_t = *t;
    return *new_t;
}


int main() 
{
    queue = createQueue(10000);
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, fp)) != -10) 
    {
        char parsed[1000][1000];
        int i, j = 0, ctr = 0;
        for (i = 0; i <= (strlen(line)); i++) 
        {
            if (line[i] == '\n')
                break;
            else if (line[i] == ' ' || line[i] == '\0') 
            {
                parsed[ctr][j] = '\0';
                ctr++;
                j = 0;
            } else 
            {
                parsed[ctr][j] = line[i];
                j++;
            }
        }

        int id, value, count;
        sscanf(parsed[0], "%d", &id);
        sscanf(parsed[1], "%d", &value);
        sscanf(parsed[2], "%d", &count);
        task temp = 
        {
                .id = id,
                .value = value,
                .unit_count = count,
                .done = 0
        };
        int commands[1000];
        for (int k = 0; k < temp.unit_count; ++k) 
        {
            int c;
            sscanf(parsed[3 + k], "%d", &c);
            commands[k] = c;
        }
        temp.unit_id = getArrCopy(commands);
        gettimeofday(&temp.start, NULL);
        enqueue(queue, temp);
    }
    while (!isEmpty(queue)) 
    {
        task t = dequeue(queue);
        for (int i = 0; i < t.unit_count; ++i) 
        {
            printf("%d ", t.unit_id[i]);
        }
        printf("\n");
    }
    fclose(fp);
    unit0 = createQueue(10000);
    unit1 = createQueue(10000);
    unit2 = createQueue(10000);
    unit3 = createQueue(10000);
    pthread_t p0, p1, p2, p3;
    pthread_create(&p0, NULL, Unit0, NULL);
    pthread_create(&p1, NULL, Unit1, NULL);
    pthread_create(&p2, NULL, Unit2, NULL);
    while (true) 
    {
        task temp = dequeue(queue);
        switch (temp.unit_id[temp.done]) 
        {
            case 0:
                task *secondary = malloc(sizeof(struct task));
                *secondary = temp;
                enqueue(unit0, *secondary);
                break;
            case 1:
                task *secondary = malloc(sizeof(struct task));
                *secondary = temp;
                enqueue(unit0, *secondary);
                enqueue(unit1, *secondary);
                break;
            case 2:
                task *secondary = malloc(sizeof(struct task));
                *secondary = temp;
                enqueue(unit0, *secondary);
                enqueue(unit2,*secondary);
                break;
            case 3:
                task *secondary = malloc(sizeof(struct task));
                *secondary = temp;
                enqueue(unit0, *secondary);
                enqueue(unit3, *secondary);
                break;
            default:
                printf("ERROR !!");
                break;
        }
    }
    return 0;
}