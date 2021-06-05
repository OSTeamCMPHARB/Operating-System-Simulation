#include <stdio.h>
#include <stdlib.h>

typedef struct nodepriority nodepriority;
typedef struct priorityqueue priorityqueue;

struct nodepriority
{
    process data; // A data item
    int priority;
    nodepriority *next;
};

struct priorityqueue
{
    int count;
    nodepriority *front;
    nodepriority *rear;
};

void initializepriority(priorityqueue *q)
{
    q->count = 0;
    q->front = NULL;
    q->rear = NULL;
}
int isemptypriority(priorityqueue *q)
{
    return (q->rear == NULL);
}
void enqueuepriority(priorityqueue *q, process value, int priority)
{
    nodepriority *tmp;
    tmp = (nodepriority *)malloc((sizeof(nodepriority)));
    tmp->data = value;
    tmp->priority = priority;
    tmp->next = NULL;
    if (!isemptypriority(q))
    {
        if (tmp->priority < q->front->priority)
        {
            tmp->next = q->front;
            q->front = tmp;
            q->count++;
            return;
        }
        nodepriority *currentNode;
        currentNode = q->front;
        while (currentNode != NULL)
        {
            //printf("testing \n");
            if (currentNode->next != NULL && tmp->priority < currentNode->next->priority)
            {
                tmp->next = currentNode->next;
                currentNode->next = tmp;
                q->count++;
                return;
            }
            else
            {
                currentNode = currentNode->next;
            }
        }
        q->rear->next = tmp;
        q->rear = tmp;
    }
    else
    {
        q->front = q->rear = tmp;
    }
    q->count++;
}
process dequeuepriority(priorityqueue *q)
{
    nodepriority *tmp;
    process n = q->front->data;
    tmp = q->front;
    q->front = q->front->next;
    q->count--;
    if (q->count == 0)
        q->rear = NULL;
    free(tmp);
    return (n);
}
process mdequeuepriority(priorityqueue *q, int i)
{
    if ( i == 0)
    {
        return dequeuepriority(q);
    }
    nodepriority *tmp;
    tmp = q->front;
    i--;
    while (i)
    {
        tmp = tmp->next;
        i--;
    }

    nodepriority *deleted = tmp->next;
    process n = deleted->data;
    tmp->next = deleted->next;
    q->count--;
    if (q->count == 0)
    {
        q->front = NULL;
        q->rear = NULL;
    }
    free(deleted);
    return (n);
}
process *beek(priorityqueue *q)
{
    return &(q->front->data);
}
int mbeek(priorityqueue *q, int i)
{
    nodepriority *tmp;
    tmp = q->front;
    while (i)
    {
        if(tmp==NULL){
            return 1000;
        }
        tmp = tmp->next;
        i--;
    }
    return tmp->data.priority;
}
