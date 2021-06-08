#include <stdio.h>
#include <stdlib.h>
typedef struct node node;
typedef struct process process ;
typedef struct queue queue;
struct process
{
    int id;
    int memsize;
    int address;
    int endAddress;
    int arrival;
    int runtime;
    int priority;
    int pid;
    int remain;
    int wait;
    int forked;
    int finishTime;
    int startTime;
    int real;
    int isblocked;
    int remainingTimeFreq;
};

struct node
{
    process data; // A data item
    node *next;
    int priority;
};

struct queue
{
    int count;
    node *front;
    node *rear;
};

void initialize(queue *q)
{
    q->count = 0;
    q->front =NULL;
    q->rear = NULL;
}
int isempty(queue *q)
{
    return (q->rear == NULL);
}
void enqueue(queue *q, process value)
{
    node *tmp;
    tmp =(node *)malloc((sizeof(node)));
    tmp->data = value;
    tmp->next = NULL;
    if(!isempty(q))
    {
        q->rear->next = tmp;
        q->rear = tmp;
    }
    else
    {
        q->front = q->rear = tmp;
    }
    q->count++;
}
process dequeue(queue *q)
{
    node *tmp;
    process n = q->front->data;
    tmp = q->front;
    q->front = q->front->next;
    q->count--;
    if(q->count==0)
        q->rear=NULL;
    free(tmp);
    return(n);
}