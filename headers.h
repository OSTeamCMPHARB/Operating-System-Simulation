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
typedef struct memoBuff memoBuff;
typedef struct memom memom;

#define true 1
#define false 0

#define SHKEY 300
#define Q1KEY 111
#define Q2KEY 222
#define Q3KEY 4443
#define Q4KEY 4444

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

int getID_SPG()
{
    int id = msgget(Q1KEY, 0666 | IPC_CREAT);
    if (id == -1)
    {
        perror("Error in create SPG");
        exit(-1);
    }
    return id;
}
int getID_SP()
{
    int id = msgget(Q2KEY, 0666 | IPC_CREAT);
    if (id == -1)
    {
        perror("Error in create SPG");
        exit(-1);
    }
    return id;
}
int getID_SM()
{
    int id = msgget(Q3KEY, 0666 | IPC_CREAT);
    if (id == -1)
    {
        perror("Error in create SPG");
        exit(-1);
    }
    return id;
}
int getID_MS()
{
    int id = msgget(Q4KEY, 0666 | IPC_CREAT);
    if (id == -1)
    {
        perror("Error in create SPG");
        exit(-1);
    }
    return id;
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
    int allProcessesGenerated; // 1 means not finished yet | 2 means finished generating
    struct process processObj;
    int moreProcess;
};

struct pbuff
{
    long mtype;
    int remainingTime;
};

struct memom
{
    int memorySize;
    int start; // start index
    int proccesID;
};

struct memoBuff
{
    long mtype; //1 means break (terminate memory.c)  2 means deallocate memory  3 means requset to allocate memory 
    memom m;
};
