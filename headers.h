#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "queue.h"
#include "priorityqueue.h"
#include "string.h"

typedef short bool;
typedef struct msgbuff msgbuff;
typedef struct pbuff pbuff;


#define true 1
#define false 0

#define SHKEY 300
#define Q1KEY 111
#define Q2KEY 222
#define Q3KEY 333

///==============================
//don't mess with this variable//
int *shmaddr; //
//===============================

int getClk()
{
    return *shmaddr;
}

/*
 * All processes call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All processes call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}
struct msgbuff
{
    int allProcessesGenerated;// 1 means not finished yet | 2 means finished generating
    struct process processObj;
    int moreProcess;
};

struct pbuff
{
    long mtype;
    int remainingTime;
};

struct mbuff
{
    long algorithmNum;
    int memorySize;
    int start;
    int finished;/* 0= program isn't finished         1= program finished          2= deallocate this memory    from scheduler to memory
                    0= NO enough space were found           1= YES enough memory were found                     from memory to scheduler  */
};
