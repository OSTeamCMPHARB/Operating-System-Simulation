#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char *argv[])
{
    initClk();
    int msgq_id_2 = msgget(Q2KEY, 0666 | IPC_CREAT);
    if (msgq_id_2 == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    int val;
    struct pbuff message;
    int type = getpid() % 100000;
    val = msgrcv(msgq_id_2, &message, sizeof(message.r), type, !IPC_NOWAIT);
    remainingtime = message.r;

printf ("remining time %d\n",remainingtime);
    //TODO The process needs to get the remaining time from somewhere
    //remainingtime = ??;
    while (remainingtime > 0)
    {
        // remainingtime = ??;
        val = msgrcv(msgq_id_2, &message, sizeof(message.r), type, IPC_NOWAIT);
        if (val != -1)
        {
            remainingtime = message.r;
            
        }

    }
    printf ("terminated at %d\n",getClk());

    destroyClk(false);

    return 0;
}
