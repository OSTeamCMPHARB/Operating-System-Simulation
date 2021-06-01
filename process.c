#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char *argv[])
{
    initClk();
    int msgq_id_SP = getID_SP();
    int val;
    struct pbuff message;
    int type = getpid() % 100000;
    val = msgrcv(msgq_id_SP, &message, sizeof(message.remainingTime), type, !IPC_NOWAIT);
    remainingtime = message.remainingTime;

    //printf ("remining time %d\n",remainingtime);
    //TODO The process needs to get the remaining time from somewhere
    //remainingtime = ??;
    while (remainingtime > 0)
    {
        // remainingtime = ??;
        val = msgrcv(msgq_id_SP, &message, sizeof(message.remainingTime), type, IPC_NOWAIT);
        if (val != -1)
        {
            remainingtime = message.remainingTime;
        }

    }
    printf ("process terminated at %d\n",getClk());

    destroyClk(false);

    return 0;
}
