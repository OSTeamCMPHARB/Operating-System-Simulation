#include "headers.h"

int main(int argc, char *argv[])
{
    initClk();

    //create message queue to communicate process_generator.c with schduler.c
    int msgq_id_SPG = getID_SPG();
    //create message queue to communicate schduler.c with process.c
    int msgq_id_SP = getID_SP();

    int val;
    int currTime = getClk();
    int selectAlgo = atoi(argv[1]);
    int freqTime = atoi(argv[2]);
    printf("The freq time is %i , The scheduler algo is %i \n", freqTime, selectAlgo);

    //writing output in scheduler.log
    FILE *pFile;
    pFile = fopen("Scheduler.log", "w");
    fprintf(pFile, "#At\ttime\tx\tprocess\ty\tstate\t\tarr\tw\ttotal\tz\tremain\ty\twait\tk \n");
    //writing output in memory.log

    // for CPU utilization and Average WTA & Waiting time//
    FILE *perfFile;
    perfFile = fopen("Scheduler.perf", "w");
    float TotalWTA = 0;
    float TotalWait = 0;
    int NumberOfProcesses = 0;
    int usefulTime = 0;

    char path[256];
    getcwd(path, sizeof(path));
    strcat(path, "/process.out");

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

        while ((flag == 1) || !isempty(&readyQueue) || processRunning != NULL)
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
                    while (message.moreProcess)
                    {
                        val = msgrcv(msgq_id_SPG, &message, sizeof(message.processObj), 0, !IPC_NOWAIT);
                        enqueue(&readyQueue, message.processObj);
                        NumberOfProcesses++;
                    }
                    if (message.allProcessesGenerated == 2)
                    {
                        flag = 0;
                    }
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
                    execl(path, "process.out", NULL);
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

                if (processRunning != NULL)
                {
                    // if there is process running decreamt remaing time then send it to this process
                    processRunning->remain--;
                    usefulTime++;

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
        }
    }
    else if (selectAlgo == 2)
    { //SJF Shortest Job First
        msgbuff message;
        pbuff pmessage;
        priorityqueue readyQueue;
        initializepriority(&readyQueue);

        int flag = 1;
        int TA = 0;
        float WTA = 0;

        process *processRunning = NULL;
        process currProcess;
        int finishTime = 0;
        currTime = 1;
        // while there are processes will come to the system OR there is processes not finshed yet

        while ((flag == 1) || !isemptypriority(&readyQueue) || (processRunning != NULL))
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
                    enqueuepriority(&readyQueue, message.processObj, message.processObj.runtime);
                    NumberOfProcesses++;
                    while (message.moreProcess)
                    {
                        val = msgrcv(msgq_id_SPG, &message, sizeof(message.processObj), 0, !IPC_NOWAIT);
                        enqueuepriority(&readyQueue, message.processObj, message.processObj.runtime);
                        NumberOfProcesses++;
                    }
                    if (message.allProcessesGenerated == 2)
                    {
                        flag = 0;
                    }
                }
            }
            // if there is no process running at that time and the readyqueue of procesess not empty
            // pick one process and fork it

            if (processRunning == NULL && !isemptypriority(&readyQueue))
            {

                currProcess = dequeuepriority(&readyQueue);
                processRunning = &currProcess;
                processRunning->pid = fork();
                if (processRunning->pid == 0)
                {
                    execl(path, "process.out", NULL);
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
                if (processRunning != NULL)
                {
                    usefulTime++;

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
    else if (selectAlgo == 3)
    { //HPF Highest Priority First
        msgbuff message;
        pbuff pmessage;
        priorityqueue readyQueue;
        initializepriority(&readyQueue);
        int flag = 1;
        int TA = 0;
        float WTA = 0;
        process *processRunning = NULL;
        process currProcess;
        int finishTime = 0;
        currTime = 1;
        while ((flag == 1) || !isemptypriority(&readyQueue) || processRunning != NULL)
        {
            val = msgrcv(msgq_id_SPG, &message, sizeof(message.processObj), 0, IPC_NOWAIT);
            if (val != -1)
            {
                if (message.allProcessesGenerated == 2)
                {
                    flag = 0;
                }
                else
                {
                    enqueuepriority(&readyQueue, message.processObj, message.processObj.priority);
                    NumberOfProcesses++;
                    while (message.moreProcess)
                    {
                        val = msgrcv(msgq_id_SPG, &message, sizeof(message.processObj), 0, !IPC_NOWAIT);
                        enqueuepriority(&readyQueue, message.processObj, message.processObj.priority);
                        NumberOfProcesses++;
                    }
                    if (message.allProcessesGenerated == 2)
                    {
                        flag = 0;
                    }
                }
            }
            ///////////////////////////////////////////////////start trial
            if (!isemptypriority(&readyQueue))
            {
                if (processRunning == NULL)
                {
                    currProcess = dequeuepriority(&readyQueue);
                    processRunning = &currProcess;
                    if (processRunning->isblocked == 1)
                    {
                        kill(processRunning->pid, SIGCONT);
                        finishTime = getClk() + processRunning->remain;
                        processRunning->wait = currTime - processRunning->arrival - processRunning->effectinetime;
                        printf("process with PID = %d resumed at time step = %d  the remaining time = %d the finish time = %d \n", processRunning->id, getClk(), processRunning->remain, finishTime);
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tresumed\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                    }
                    else
                    {
                        processRunning->pid = fork();
                        if (processRunning->pid == 0)
                        {
                            execl(path, "process.out", NULL);
                        }
                        processRunning->wait = getClk() - processRunning->arrival;
                        finishTime = getClk() + processRunning->remain;
                        printf(" id :%d  start at :%d  end at %d:\n", processRunning->id, getClk(), finishTime);
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tstarted\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                    }
                    pmessage.mtype = processRunning->pid % 100000;
                    pmessage.remainingTime = processRunning->remain;
                    val = msgsnd(msgq_id_SP, &pmessage, sizeof(pmessage.remainingTime), !IPC_NOWAIT);
                }
                else
                {
                    if (processRunning->priority > beek(&readyQueue)->priority) //change
                    {
                        kill(processRunning->pid, SIGSTOP);
                        processRunning->wait = currTime - processRunning->arrival - processRunning->effectinetime;
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tstopped\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                        processRunning->isblocked = 1;
                        enqueuepriority(&readyQueue, currProcess, processRunning->priority); //change
                        currProcess = dequeuepriority(&readyQueue);
                        processRunning = &currProcess;
                        if (processRunning->isblocked == 1)
                        {
                            kill(processRunning->pid, SIGCONT);
                            finishTime = currTime + processRunning->remain;
                            processRunning->wait = currTime - processRunning->arrival - processRunning->effectinetime;
                            printf("process with PID = %d continue at time step = %d the remaining time = %d the finish time = %d \n", processRunning->id, getClk(), processRunning->remain, finishTime);
                            fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tresumed\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                        }
                        else
                        {
                            processRunning->pid = fork();
                            if (processRunning->pid == 0)
                            {
                                execl(path, "process.out", NULL);
                            }
                            processRunning->wait = getClk() - processRunning->arrival;
                            finishTime = getClk() + processRunning->remain;
                            printf(" id :%d  start at :%d  end at %d:\n", processRunning->id, getClk(), finishTime);
                            fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tstarted\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                        }
                        pmessage.mtype = processRunning->pid % 100000;
                        pmessage.remainingTime = processRunning->remain;
                        val = msgsnd(msgq_id_SP, &pmessage, sizeof(pmessage.remainingTime), !IPC_NOWAIT);
                    }
                }
            }
            if (currTime != getClk())
            {
                currTime = getClk();
                printf("%d\n", currTime);
                if (processRunning != NULL)
                {
                    usefulTime++;

                    // if there is process running decreamt remaing time then send it to this process
                    processRunning->effectinetime++;
                    processRunning->remain--;
                    pmessage.mtype = processRunning->pid % 100000;
                    pmessage.remainingTime = processRunning->remain;
                    val = msgsnd(msgq_id_SP, &pmessage, sizeof(pmessage.remainingTime), !IPC_NOWAIT);
                    if (processRunning->remain == 0)
                    {
                        processRunning->wait = currTime - processRunning->arrival - processRunning->runtime;

                        // if the remaining time =0 set the run pointer to null to take anther process
                        printf(" run time %d \n", processRunning->remain);
                        TA = processRunning->wait + processRunning->runtime;
                        WTA = (float)TA / processRunning->runtime;
                        TotalWait += processRunning->wait;
                        TotalWTA += WTA;
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tfinished\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n", currTime, processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait, TA, WTA);
                        processRunning = NULL;
                    }
                }
            }
            //////////////////////////////////////////////////end trial
        }
    }
    else if (selectAlgo == 4)
    { //SRTN Shortest Remaining Time Next
        priorityqueue temp;
        initializepriority(&temp);

        msgbuff message;
        pbuff pmessage;
        priorityqueue readyQueue;
        initializepriority(&readyQueue);
        int flag = 1;
        int TA = 0;
        float WTA = 0;
        process *processRunning = NULL;
        process currProcess;
        int finishTime = 0;
        currTime = 1;
        while ((flag == 1) || !isemptypriority(&readyQueue) || processRunning != NULL)
        {
            val = msgrcv(msgq_id_SPG, &message, sizeof(message.processObj), 0, IPC_NOWAIT);
            if (val != -1)
            {
                if (message.allProcessesGenerated == 2)
                {
                    flag = 0;
                }
                else
                {
                    enqueuepriority(&readyQueue, message.processObj, message.processObj.runtime);
                    NumberOfProcesses++;
                    while (message.moreProcess)
                    {
                        val = msgrcv(msgq_id_SPG, &message, sizeof(message.processObj), 0, !IPC_NOWAIT);
                        enqueuepriority(&readyQueue, message.processObj, message.processObj.runtime);
                        NumberOfProcesses++;
                    }
                    if (message.allProcessesGenerated == 2)
                    {
                        flag = 0;
                    }
                }
            }
            ///////////////////////////////////////////////////start trial
            if (!isemptypriority(&readyQueue))
            {
                if (processRunning == NULL)
                {
                    currProcess = dequeuepriority(&readyQueue);
                    processRunning = &currProcess;
                    if (processRunning->isblocked == 1)
                    {
                        kill(processRunning->pid, SIGCONT);
                        finishTime = getClk() + processRunning->remain;
                        processRunning->wait = currTime - processRunning->arrival - processRunning->effectinetime;
                        printf("process with PID = %d resumed at time step = %d  the remaining time = %d the finish time = %d \n", processRunning->id, getClk(), processRunning->remain, finishTime);
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tresumed\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                    }
                    else
                    {
                        processRunning->pid = fork();
                        if (processRunning->pid == 0)
                        {
                            execl(path, "process.out", NULL);
                        }
                        processRunning->wait = getClk() - processRunning->arrival;
                        finishTime = getClk() + processRunning->remain;
                       
                        printf(" id :%d  start at :%d  end at %d:\n", processRunning->id, getClk(), finishTime);
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tstarted\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                    }
                    pmessage.mtype = processRunning->pid % 100000;
                    pmessage.remainingTime = processRunning->remain;
                    val = msgsnd(msgq_id_SP, &pmessage, sizeof(pmessage.remainingTime), !IPC_NOWAIT);
                }
                else
                {
                    if (processRunning->remain > beek(&readyQueue)->remain) //change
                    {
                        kill(processRunning->pid, SIGSTOP);
                        processRunning->wait = currTime - processRunning->arrival - processRunning->effectinetime;
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tstopped\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                        processRunning->isblocked = 1;
                        enqueuepriority(&readyQueue, currProcess, processRunning->remain); //change
                        currProcess = dequeuepriority(&readyQueue);
                        processRunning = &currProcess;
                        if (processRunning->isblocked == 1)
                        {
                            kill(processRunning->pid, SIGCONT);
                            finishTime = currTime + processRunning->remain;
                            processRunning->wait = currTime - processRunning->arrival - processRunning->effectinetime;
                            printf("process with PID = %d continue at time step = %d the remaining time = %d the finish time = %d \n", processRunning->id, getClk(), processRunning->remain, finishTime);
                            fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tresumed\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                        }
                        else
                        {
                            processRunning->pid = fork();
                            if (processRunning->pid == 0)
                            {
                                execl(path, "process.out", NULL);
                            }
                            processRunning->wait = getClk() - processRunning->arrival;
                            finishTime = getClk() + processRunning->remain;
                            
                            printf(" id :%d  start at :%d  end at %d:\n", processRunning->id, getClk(), finishTime);
                            fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tstarted\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                        }
                        pmessage.mtype = processRunning->pid % 100000;
                        pmessage.remainingTime = processRunning->remain;
                        val = msgsnd(msgq_id_SP, &pmessage, sizeof(pmessage.remainingTime), !IPC_NOWAIT);
                    }
                }
            }
            if (currTime != getClk())
            {

                currTime = getClk();
                printf("%d\n", currTime);
                if (processRunning != NULL)
                {
                    usefulTime++;
                        // if there is process running decreamt remaing time then send it to this process
                    processRunning->effectinetime++;

                        processRunning->remain--;
                    pmessage.mtype = processRunning->pid % 100000;
                    pmessage.remainingTime = processRunning->remain;
                    val = msgsnd(msgq_id_SP, &pmessage, sizeof(pmessage.remainingTime), !IPC_NOWAIT);
                    if (processRunning->remain == 0)
                    { //
                        processRunning->wait = currTime - processRunning->arrival - processRunning->runtime;
                        
                        // if the remaining time =0 set the run pointer to null to take anther process
                        printf(" run time %d \n", processRunning->remain);
                        TA = processRunning->wait + processRunning->runtime;
                        WTA = (float)TA / processRunning->runtime;
                        TotalWait += processRunning->wait;
                        TotalWTA += WTA;
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tfinished\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n", currTime, processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait, TA, WTA);
                        processRunning = NULL;
                    }
                }
            }
            //////////////////////////////////////////////////end trial
        }
    }
    else if (selectAlgo == 5)
    { //RR Round Robin
        msgbuff message;
        pbuff processMessage;
        queue readyQueue;    //queue for ready processes
        queue finishedQueue; //queue for finished processes
        initialize(&readyQueue);
        initialize(&finishedQueue);

        int flag = 1; //inticades that the schedular is not busy

        int flagForFreq = 0;
        int freqTimeRemain = 0;

        process *processRunning = NULL;
        process currProcess;
        currProcess.id = -1;
        currProcess.remain=0;
        int finishTime = 0;
        currTime = -1;
        int printClk = -1;
        int TA = 0;
        float WTA = 0;
        int stoppedProc=0;

        while (flag || !isempty(&readyQueue) || !isempty(&finishedQueue) || flagForFreq || stoppedProc)
        {
            int Clk = getClk();
            if (printClk != getClk())
            {
                printClk = getClk();
                printf("Current Time %d\n", getClk());
            }
            /*if there is a message from process_generator insert the arrived process in the queue*/
            val = msgrcv(msgq_id_SPG, &message, sizeof(message.processObj), 0, IPC_NOWAIT);

            if (val != -1) //checking for new Processes from process generator
            {
                if (message.allProcessesGenerated == 2)
                {
                    flag = 0;
                }
                else
                {
                    message.processObj.runtime = 0;   //Didn't run before so intialize it to zero
                    message.processObj.isblocked = 0; //Wasn't forked before
                    enqueue(&readyQueue, message.processObj);
                    NumberOfProcesses++;
                    while (message.moreProcess)
                    {
                        val = msgrcv(msgq_id_SPG, &message, sizeof(message.processObj), 0, !IPC_NOWAIT);
                        message.processObj.runtime = 0;   //Didn't run before so intialize it to zero
                        message.processObj.isblocked = 0; //Wasn't forked before
                        enqueue(&readyQueue, message.processObj);
                        NumberOfProcesses++;
                    }
                    if (message.allProcessesGenerated == 2)
                    {
                        flag = 0;
                    }
                }
            }
            if (freqTime == 1)
            {
                if ((readyQueue.count > 0 || finishedQueue.count > 0 || currProcess.remain != 0) && currTime != Clk)
                { //if it's a new time step run process with the currTurn
                    //printf("here \n");
                    stoppedProc=0;
                    currTime = Clk;
                    if (currProcess.id != -1 && currProcess.remain!=0 && isempty(&finishedQueue))
                    {
                        currProcess.wait = currTime - currProcess.arrival - currProcess.runtime;
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tstopped\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", currTime, currProcess.id, currProcess.arrival, currProcess.runtime, currProcess.remain, currProcess.wait);
                        enqueue(&readyQueue, currProcess);
                    }

                    if (!isempty(&finishedQueue)) //checking if there is a finished processes
                    {                             //If process is finished
                        currProcess = dequeue(&finishedQueue);

                        currProcess.finishTime = Clk;
                        currProcess.wait = currProcess.finishTime - currProcess.arrival - currProcess.runtime;
                        TA = currProcess.wait + currProcess.runtime; //total turn around , waiting + running
                        WTA = (float)TA / currProcess.runtime;       //total turn around over runtime
                        TotalWait += currProcess.wait;
                        TotalWTA += WTA;
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n", currTime, currProcess.id, currProcess.arrival, currProcess.runtime, currProcess.remain, currProcess.wait, TA, WTA);
                    }

                    if (!isempty(&readyQueue))
                    {
                        currProcess = dequeue(&readyQueue); //sending signal continue to process
                        if (currProcess.isblocked == 1)
                        {
                            //stoppedProc=1;
                            currProcess.wait = currTime - currProcess.arrival - currProcess.runtime;
                            fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tresumed\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", currTime, currProcess.id, currProcess.arrival, currProcess.runtime, currProcess.remain, currProcess.wait);
                            kill(SIGCONT, currProcess.pid);
                        }
                        else /* To avoid multiple forking for the same process */
                        {

                            currProcess.pid = fork();
                            currProcess.isblocked = 1;
                            currProcess.startTime = Clk;
                            currProcess.wait = currTime - currProcess.arrival;

                            /*if this is the child process make it execute the currProcess*/
                            if (currProcess.pid == 0)
                            {
                                execl(path, "process.out", NULL);
                            }
                            printf("just forked process details : pid=%d  forked=%d  arrival= %d     remain=%d      runtime=%d\n", currProcess.id, currProcess.isblocked, currProcess.arrival, currProcess.remain, currProcess.runtime);
                            fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tstarted\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), currProcess.id, currProcess.arrival, currProcess.runtime, currProcess.remain, currProcess.wait);
                        }

                        //printf("Will compare getCLk %d with currTIme %d\n", Clk, currTime);

                        if (currProcess.remain > 0 && currProcess.isblocked == 1)
                        {
                            usefulTime++;
                            /*send the remaining time to the process.c to decrement it*/
                            processMessage.mtype = currProcess.pid % 100000;
                            processMessage.remainingTime = currProcess.remain;
                            currProcess.remain = (currProcess.remain) - 1;   //Decremting Remaining Time Then will send STOP signal
                            currProcess.runtime = (currProcess.runtime) + 1; //Incrementing Running Time

                            val = msgsnd(msgq_id_SP, &processMessage, sizeof(processMessage.remainingTime), !IPC_NOWAIT);

                            if (val == -1)
                            {
                                perror("Error while schedular send to process");
                            }

                            /*Details Of Process In That time step*/
                            /*enqueuing the process again to the queue to take it's turn*/
                            if (currProcess.remain > 0)
                            {
                                stoppedProc=1;

                                kill(SIGSTOP, currProcess.pid); //sending to process to stop
                                //fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tstopped\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", currTime, currProcess.id, currProcess.arrival, currProcess.runtime, currProcess.remain, currProcess.wait);
                                //enqueue(&readyQueue, currProcess);
                            }
                            else
                            {
                                enqueue(&finishedQueue, currProcess);
                            }
                        }
                    }
                }
            }
            else
            {

                if ((readyQueue.count > 0 || finishedQueue.count > 0 || flagForFreq || stoppedProc) && currTime != Clk)
                { //if it's a new time step run process with the currTurn
                    //printf("here 1\n");
                    stoppedProc=0;
                    currTime = Clk;
                    if (currProcess.id != -1 && currProcess.remain!=0 && isempty(&finishedQueue) && flagForFreq == 0)
                    {
                        currProcess.wait = currTime - currProcess.arrival - currProcess.runtime;
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tstopped\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", currTime, currProcess.id, currProcess.arrival, currProcess.runtime, currProcess.remain, currProcess.wait);
                        enqueue(&readyQueue, currProcess);
                    }

                    if (!isempty(&finishedQueue)) //checking for finished processes
                    {                             //If process is finished

                        currProcess = dequeue(&finishedQueue);

                        currProcess.finishTime = Clk;
                        currProcess.wait = currProcess.finishTime - currProcess.arrival - currProcess.runtime;
                        TA = currProcess.wait + currProcess.runtime; //total turn around , waiting + running
                        WTA = (float)TA / currProcess.runtime;       //total turn around over runtime
                        TotalWait += currProcess.wait;
                        TotalWTA += WTA;

                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n", currTime, currProcess.id, currProcess.arrival, currProcess.runtime, currProcess.remain, currProcess.wait, TA, WTA);
                    }

                    if (!isempty(&readyQueue))
                    {

                        if (flagForFreq == 0)
                        {
                            currProcess = dequeue(&readyQueue); //sending signal continue to process
                            if (currProcess.isblocked == 1)
                            {
                                currProcess.wait = currTime - currProcess.arrival - currProcess.runtime;
                                fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tresumed\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", currTime, currProcess.id, currProcess.arrival, currProcess.runtime, currProcess.remain, currProcess.wait);

                                kill(SIGCONT, currProcess.pid);
                            }
                            else /* To avoid multiple forking for the same process */
                            {
                                currProcess.pid = fork();
                                currProcess.isblocked = 1;
                                currProcess.startTime = Clk;
                                currProcess.wait = currTime - currProcess.arrival;

                                /*if this is the child process make it execute the currProcess*/
                                if (currProcess.pid == 0)
                                {
                                    execl(path, "process.out", NULL);
                                }
                                printf("just forked process details : pid=%d  forked=%d  arrival= %d     remain=%d      runtime=%d\n", currProcess.id, currProcess.isblocked, currProcess.arrival, currProcess.remain, currProcess.runtime);
                                fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tstarted\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), currProcess.id, currProcess.arrival, currProcess.runtime, currProcess.remain, currProcess.wait);
                            }
                        }
                    }

                    if (currProcess.remain > 0 && currProcess.isblocked == 1)
                    {
                        //printf("PROC %i the remaining time is %i after & flag %i \n", currProcess.id, freqTimeRemain, flagForFreq);
                        //printf("running \n");
                        if (flagForFreq == 0)
                        {
                            flagForFreq = 1;
                            freqTimeRemain = freqTime - 1; //decremting
                        }
                        else
                        {
                            freqTimeRemain = freqTimeRemain - 1;
                        }
                        //printf("PROC %i the remaining time is %i after & flag %i \n", currProcess.id, freqTimeRemain, flagForFreq);
                        /*send the remaining time to the process.c to decrement it*/
                        usefulTime++;
                        processMessage.mtype = currProcess.pid % 100000;
                        processMessage.remainingTime = currProcess.remain;
                        currProcess.remain = (currProcess.remain) - 1;   //Decremting Remaining Time
                        currProcess.runtime = (currProcess.runtime) + 1; //Incrementing Running Time

                        val = msgsnd(msgq_id_SP, &processMessage, sizeof(processMessage.remainingTime), !IPC_NOWAIT); //sending to process the remaining time

                        if (val == -1)
                        {
                            perror("Error while schedular send to process");
                        }

                        /*Details Of Process In That time step*/
                        if (currProcess.remain > 0)
                        {
                            if (freqTimeRemain == 0)
                            {
                                flagForFreq = 0;
                                stoppedProc=1;
                                //printf("stopped");
                                kill(SIGSTOP, currProcess.pid); //sending to process to stop
                            }
                        }
                        else
                        {
                            flagForFreq = 0;
                            enqueue(&finishedQueue, currProcess);
                        }
                    }
                    //printf("%i   %i     %i    %i \n", finishedQueue.count, readyQueue.count, flagForFreq, flag);
                }
            }
        }
        printf("exiting the sch");
    }

    float CPU_utilization = ((float)usefulTime / (getClk())) * 100;
    fprintf(perfFile, " CPU utilization = %.2f %% \n Avg WTA = %.2f\n Avg Waiting = %.2f", CPU_utilization, TotalWTA / NumberOfProcesses, TotalWait / NumberOfProcesses);
    fclose(perfFile);
    fclose(pFile);

    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.
    //val = msgsnd(msgq_id_SM, &memoMessage, sizeof(memoMessage) - sizeof(long), !IPC_NOWAIT);//Deallocate the memory
    //val = msgrcv(msgq_id_MS, &memoMessage, sizeof(memoMessage) - sizeof(long), 0, !IPC_NOWAIT); //recieving the answer from memory 0= NO SPACE    1= SPACE FOUND
    destroyClk(true);
}
