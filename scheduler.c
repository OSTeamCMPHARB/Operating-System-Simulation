#include "headers.h"

int main(int argc, char *argv[])
{
    initClk();
    //create message queue to communicate process_generator.c with schduler.c
    int msgq_id_SPG = msgget(Q1KEY, 0666 | IPC_CREAT);
    if (msgq_id_SPG == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    //create message queue to communicate schduler.c with process.c
    int msgq_id_SP = msgget(Q2KEY, 0666 | IPC_CREAT);
    if (msgq_id_SP == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    int val;
    int currTime = getClk();
    int selectAlgo = atoi(argv[1]);
    FILE *pFile;
    pFile = fopen("Scheduler.log", "w");
    fprintf(pFile, "#At\ttime\tx\tprocess\ty\tstate\t\tarr\tw\ttotal\tz\tremain\ty\twait\tk \n");

    // for CPU utilization and Average WTA & Waiting time//
    FILE *perfFile;
    perfFile = fopen("Scheduler.perf", "w");
    float TotalWTA = 0;
    float TotalWait = 0;
    int NumberOfProcesses = 0;
    int usefulTime = 0;

    if (selectAlgo == 1)
    { //FCFS First Come First Serve
        msgbuff message;
        pbuff pmessage;
        queue readyQueue;
        initialize(&readyQueue);
        int flag = 1;

        int TA = 0;
        float WTA = 0;

        process *processRunning = NULL;
        process currProcess;
        int finishTime = 0;
        currTime = 1;
        // while there are processes will come to the system OR there is processes not finshed yet

        while ((flag == 1) || !isempty(&readyQueue) || (getClk() <= finishTime))
        {

            // if there is a message from process_generator insert the arrived process in the queue
            val = msgrcv(msgq_id_SPG, &message, sizeof(message.processObj), 0, IPC_NOWAIT);
            if (val != -1)
            {
                if (message.allProcessesGenerated == 2)
                {
                    flag = 0;
                }
                else
                {
                    enqueue(&readyQueue, message.processObj);
                    NumberOfProcesses++;
                }
            }
            // if there is no process running at that time and the readyqueue of procesess not empty
            // pick one process and fork it

            if (processRunning == NULL && !isempty(&readyQueue))
            {

                currProcess = dequeue(&readyQueue);
                processRunning = &currProcess;
                processRunning->pid = fork();
                if (processRunning->pid == 0)
                {
                    execl("/home/robert/Desktop/projects/test/OS_Scheduler/process.out", "process.out", NULL);
                }
                finishTime = getClk() + processRunning->runtime;

                printf(" id :%d  start at :%d  end at %d:\n", processRunning->id, getClk(), finishTime);
                processRunning->wait = getClk() - processRunning->arrival;
                fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tstarted\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                // send the remaining time of this process
                pmessage.mtype = processRunning->pid % 100000;
                pmessage.remainingTime = processRunning->remain;
                val = msgsnd(msgq_id_SP, &pmessage, sizeof(pmessage.remainingTime), !IPC_NOWAIT);
                continue;
            }

            if (currTime != getClk())
            {
                currTime = getClk();

                printf("%d\n", currTime);
                usefulTime++;
                if (processRunning != NULL)
                {
                    // if there is process running decreamt remaing time then send it to this process
                    processRunning->remain--;

                    pmessage.mtype = processRunning->pid % 100000;
                    pmessage.remainingTime = processRunning->remain;
                    val = msgsnd(msgq_id_SP, &pmessage, sizeof(pmessage.remainingTime), !IPC_NOWAIT);
                    if (processRunning->remain == 0)
                    {
                        // if the remaining time =0 set the run pointer to null to take anther process
                        printf(" run time %d \n", processRunning->remain);
                        TA = processRunning->wait + processRunning->runtime;
                        WTA = (float)TA / processRunning->runtime;
                        TotalWait += processRunning->wait;
                        TotalWTA += WTA;
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n", currTime, processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait, TA, WTA);
                        processRunning = NULL;
                    }
                }
                
            }
            // while time dosent change dont do any any thing (busy wait)
            //  while (currTime == getClk()){}
        }
    }
    else if (selectAlgo == 2)
    { //SJF Shortest Job First
    }
    else if (selectAlgo == 3)
    { //HPF Highest Priority First
    }
    else if (selectAlgo == 4)
    { //SRTN Shortest Remaining Time Next
    }
    else if (selectAlgo == 5)
    { //RR Round Robin
        msgbuff message;
        pbuff processMessage;
        queue readyQueue; //queue for ready processes
        initialize(&readyQueue);
        int flag = 1; //inticades that the schedular is not busy

        process *processRunning = NULL;
        process currProcess;
        int finishTime = 0;
        while (flag || !isempty(&readyQueue))
        {
            currTime = getClk();
            printf("%d\n", currTime);
            /*if there is a message from process_generator insert the arrived process in the queue*/
            val = msgrcv(msgq_id_SPG, &message, sizeof(message.processObj), 0, IPC_NOWAIT);
            if (val != -1)
            {
                if (message.allProcessesGenerated == 2)
                {
                    flag = 0;
                }
                else
                {
                    enqueue(&readyQueue, message.processObj);
                }
            }
            if (readyQueue.count > 0)
            { //I have only one process
                currProcess = dequeue(&readyQueue);
                /* To avoid multiple forking for the same process */
                if (!currProcess.forked)
                {
                    currProcess.forked = 1;
                    currProcess.pid = fork();
                    /*if this is the child process make it execute the currProcess*/
                    if (currProcess.pid == 0)
                    {
                        execl("/home/hazem/Desktop/OS_Scheduler-main/process.out", "process.out", NULL);
                    }
                }

                /*send the remaining time to the process.c to decrement it*/
                processMessage.mtype = currProcess.pid % 100000;
                processMessage.remainingTime = currProcess.remain;
                currProcess.remain = currProcess.remain - 1; /*------------------WARNING BAD PRACTICE CHANGE IT SOON-------------------*/
                val = msgsnd(msgq_id_SP, &processMessage, sizeof(processMessage.remainingTime), !IPC_NOWAIT);
                /*enqueuing the process again to the queue to take it's turn*/
                enqueue(&readyQueue, currProcess);
            }
            /* wait until the next time step comes */
            while (currTime == getClk())
            {
            }
        }
    }
    float CPU_utilization = ((float)usefulTime / (getClk()-1)) * 100;
    fprintf(perfFile, " CPU utilization = %.2f\n Avg WTA = %.2f\n Avg Waiting = %.2f", CPU_utilization, TotalWTA / NumberOfProcesses, TotalWait / NumberOfProcesses);
    fclose(perfFile);
    fclose(pFile);

    msgctl(msgq_id_SP, IPC_RMID, (struct msqid_ds *)0);

    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.

    destroyClk(true);
}
