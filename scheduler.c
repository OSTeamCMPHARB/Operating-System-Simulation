#include "headers.h"

int main(int argc, char *argv[])
{
    initClk();
 // create message queue to communicate process_generator.c with schduler.c
    int msgq_id_1 = msgget(Q1KEY, 0666 | IPC_CREAT);
    if (msgq_id_1 == -1)
    {
        perror("Error in create");
        exit(-1);
    }
 // create message queue to communicate schduler.c with process.c  
    int msgq_id_2 = msgget(Q2KEY, 0666 | IPC_CREAT);
    if (msgq_id_2 == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    int val;
    int x = getClk();
    int n = atoi(argv[1]);
    FILE *pFile;
    pFile = fopen("Scheduler.log", "w");
    fprintf(pFile, "#At time x process y state arr w total z remain y wait k \n");

// n = 1 means the chosen scheduler algo is FCFS
    if (n == 1)
    {
        msgbuff message;
        pbuff pmessage;
        queue readyq;
        initialize(&readyq);
        int flag = 1;

        process *run = NULL;
        process p;
        int f = 0;

        // while there are processes will come to the system OR there is processes not finshed yet 

        while ((flag == 1) || !isempty(&readyq) || (getClk() <= f))
        {
            x = getClk();
            printf("%d\n", x);
// if there is a message insert the arrived process in the queue
            val = msgrcv(msgq_id_1, &message, sizeof(message.sp), 0, IPC_NOWAIT);
            if (val != -1)
            {
                if (message.mtype == 2)
                {
                    flag = 0;
                }
                else
                {
                    enqueue(&readyq, message.sp);
                }
            }
// if there is no process running at that time and the readyqueue of procesess not empty
// pick one process and fork it 
            if (run == NULL && !isempty(&readyq))
            {
                p = dequeue(&readyq);
                run = &p;
                run->pid = fork();
                if (run->pid == 0)
                {
                    execl("/home/robert/Desktop/projects/OS_Scheduler/process.out", "process.out", NULL);
                }
                f = x + run->runtime;

                printf(" id :%d  start at :%d  end at %d:\n", run->id, x, f);
                run->wait = getClk() - run->arrival;
                fprintf(pFile, "At time %d process %d started arr %d total %d remain %d wait %d\n", x, run->id, run->arrival, run->runtime, run->remain, run->wait);
// send the remaining time of this process
                pmessage.mtype = run->pid % 100000;
                pmessage.r = run->remain;
                val = msgsnd(msgq_id_2, &pmessage, sizeof(pmessage.r), !IPC_NOWAIT);
            }
            else if (run != NULL)
            {
                // if there is process running decreamt remaing time then send it to this process
                run->remain--;

                pmessage.mtype = run->pid % 100000;
                pmessage.r = run->remain;
                val = msgsnd(msgq_id_2, &pmessage, sizeof(pmessage.r), !IPC_NOWAIT);
                if (run->remain == 0)
                {
                    // if the remaining time =0 set the run pointer to null to take anther process
                    printf(" run time %d \n", run->remain);
                    fprintf(pFile, "At time %d process %d finished arr %d total %d remain %d wait %d\n", x, run->id, run->arrival, run->runtime, run->remain, run->wait);

                    run = NULL;
                }
            }
            // while time dosent change dont do any any thing (busy wait)
            while (x == getClk())
                ;
        }
    }
    else if (n == 2)
    {
    }
    else if (n == 3)
    {
    }
    else if (n == 4)
    {
    }

    fclose(pFile);

    msgctl(msgq_id_2, IPC_RMID, (struct msqid_ds *)0);

    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.

    destroyClk(true);
}
