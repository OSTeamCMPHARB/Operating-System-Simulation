#include "headers.h"

int main(int argc, char *argv[])
{
    initClk();
        
    //create message queue to communicate process_generator.c with schduler.c
    int msgq_id_SPG = msgget(Q1KEY, 0666 | IPC_CREAT);
    if (msgq_id_SPG == -1)
    {
        perror("Error in create SPG");
        exit(-1);
    }
    //create message queue to communicate schduler.c with process.c
    int msgq_id_SP = msgget(Q2KEY, 0666 | IPC_CREAT);
    if (msgq_id_SP == -1)
    {
        perror("Error in create SP");
        exit(-1);
    }
    //create message queue to communicate scheduler.c with memory.c
    int msgq_id_SM = msgget(Q3KEY, 0664 | IPC_CREAT);
    if (msgq_id_SM == -1)
    {
        perror("Error in create SM");
        exit(-1);
    }
    int val;
    int currTime = getClk();
    int selectAlgo = atoi(argv[1]);
    //writing output in scheduler.log
    FILE *pFile;
    pFile = fopen("Scheduler.log", "w");
    fprintf(pFile, "#At\ttime\tx\tprocess\ty\tstate\t\tarr\tw\ttotal\tz\tremain\ty\twait\tk \n");
    //writing output in memory.log
    FILE *mFile;
    mFile = fopen("memory.log", "w");
    fprintf(mFile, "#At\ttime\tx\tallocated\ty\tbytes\t\tfor process\tz\tfrom\ti\tto\tj \n");

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
                    execl("/home/bishoy/Desktop/OSproject/OS_Scheduler-main/process.out", "process.out", NULL);
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

        while ((flag == 1) || !isemptypriority(&readyQueue) || (getClk() <= finishTime))
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
                    execl("/home/bishoy/Desktop/OSproject/OS_Scheduler-main/process.out", "process.out", NULL);
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
        while ((flag == 1) || !isemptypriority(&readyQueue) || (getClk() <= finishTime))
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
                    enqueuepriority(&readyQueue, message.processObj, message.processObj.priority); //changed
                    NumberOfProcesses++;
                    while (message.moreProcess)
                    {
                        val = msgrcv(msgq_id_SPG, &message, sizeof(message.processObj), 0, !IPC_NOWAIT);
                        enqueuepriority(&readyQueue, message.processObj, message.processObj.priority); //changed
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
                        printf("process with PID = %d resumed at time step = %d  the remaining time = %d the finish time = %d \n", processRunning->id, getClk(), processRunning->remain, finishTime);
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tresumed\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                    }
                    else
                    {
                        processRunning->pid = fork();
                        if (processRunning->pid == 0)
                        {
                            execl("/home/ghraboly/Desktop/OS_Scheduler-main/process.out", "process.out", NULL);
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
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tblocked\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                        processRunning->isblocked = 1;
                        enqueuepriority(&readyQueue, currProcess, message.processObj.priority); //change
                        currProcess = dequeuepriority(&readyQueue);
                        processRunning = &currProcess;
                        if (processRunning->isblocked == 1)
                        {
                            kill(processRunning->pid, SIGCONT);
                            finishTime = currTime + processRunning->remain;
                            printf("process with PID = %d continue at time step = %d the remaining time = %d the finish time = %d \n", processRunning->id, getClk(), processRunning->remain, finishTime);
                            fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tresumed\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                        }
                        else
                        {
                            processRunning->pid = fork();
                            if (processRunning->pid == 0)
                            {
                                execl("/home/ghraboly/Desktop/OS_Scheduler-main/process.out", "process.out", NULL);
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
        while ((flag == 1) || !isemptypriority(&readyQueue) || (getClk() <= finishTime))
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
                        printf("process with PID = %d resumed at time step = %d  the remaining time = %d the finish time = %d \n", processRunning->id, getClk(), processRunning->remain, finishTime);
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tresumed\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                    }
                    else
                    {
                        processRunning->pid = fork();
                        if (processRunning->pid == 0)
                        {
                            execl("/home/bishoy/Desktop/OSproject/OS_Scheduler-main/process.out", "process.out", NULL);
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
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tblocked\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                        processRunning->isblocked = 1;
                        enqueuepriority(&readyQueue, currProcess, message.processObj.runtime); //change
                        currProcess = dequeuepriority(&readyQueue);
                        processRunning = &currProcess;
                        if (processRunning->isblocked == 1)
                        {
                            kill(processRunning->pid, SIGCONT);
                            finishTime = currTime + processRunning->remain;
                            printf("process with PID = %d continue at time step = %d the remaining time = %d the finish time = %d \n", processRunning->id, getClk(), processRunning->remain, finishTime);
                            fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tresumed\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                        }
                        else
                        {
                            processRunning->pid = fork();
                            if (processRunning->pid == 0)
                            {
                                execl("/home/bishoy/Desktop/OSproject/OS_Scheduler-main/process.out", "process.out", NULL);
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
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tfinished\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n", currTime, processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait, TA, WTA);
                        processRunning = NULL;
                    }
                }
            }
            //////////////////////////////////////////////////end trial
        }
    }
    else if (selectAlgo == 5)
    { //RR Round Robin Algorithm
        msgbuff message;
        struct memobuff toMemory;
        pbuff processMessage;
        queue readyQueue;    //queue for ready processes
        queue finishedQueue; //queue for finished processes
        struct mbuff memoRequest;
        initialize(&readyQueue);
        initialize(&finishedQueue);

        int flag = 1; //inticades that the schedular is not busy

        process *processRunning = NULL;
        process currProcess;
        int finishTime = 0;
        currTime = -1;
        int printClk = -1;
        int TA = 0;
        float WTA = 0;
        while (flag || !isempty(&readyQueue) || !isempty(&finishedQueue))
        {
            int Clk = getClk();
            if (printClk != getClk())
            {
                printClk = getClk();
                printf("Current Time %d\n", getClk());
            }
            /*if there is a message from process_generator insert the arrived process in the queue*/
            val = msgrcv(msgq_id_SPG, &message, sizeof(message.processObj), 0, IPC_NOWAIT);
            if (val != -1)
            {
                printf("here");
                if (message.allProcessesGenerated == 2)
                {
                    flag = 0;
                }
                else
                {
                    printf("else");
                    message.processObj.runtime = 0; //Didn't run before so intialize it to zero
                    message.processObj.forked = 0;  //Wasn't forked before
                    message.processObj.real = 1;    //Wasn't forked before
                    enqueue(&readyQueue, message.processObj);
                    NumberOfProcesses++;
                    while (message.moreProcess)
                    {
                        printf("more");
                        val = msgrcv(msgq_id_SPG, &message, sizeof(message.processObj), 0, !IPC_NOWAIT);
                        message.processObj.runtime = 0; //Didn't run before so intialize it to zero
                        enqueue(&readyQueue, message.processObj);
                        NumberOfProcesses++;
                    }
                    if (message.allProcessesGenerated == 2)
                    {
                        flag = 0;
                    }
                }
            }
            if ((readyQueue.count > 0 || finishedQueue.count > 0) && currTime != Clk)
            { //if it's a new time step run process with the currTurn
            printf("if here");
                currTime = Clk;

                if (!isempty(&finishedQueue))
                { //If process is finished
                    currProcess = dequeue(&finishedQueue);
                    currProcess.real = 0;
                    currProcess.finishTime = Clk;
                    currProcess.wait = currProcess.finishTime - currProcess.startTime - currProcess.runtime;
                    TA = currProcess.wait + currProcess.runtime; //total turn around , waiting + running
                    WTA = (float)TA / currProcess.runtime;       //total turn around over runtime
                    TotalWait += currProcess.wait;
                    TotalWTA += WTA;
                    fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n", currTime, currProcess.id, currProcess.arrival, currProcess.runtime, currProcess.remain, currProcess.wait, TA, WTA);
                }

                if (!isempty(&readyQueue))
                {
                    currProcess = dequeue(&readyQueue); //sending signal continue to process
                    if (currProcess.forked)//if forked before & was stopped continue it
                    {
                        kill(SIGCONT, currProcess.pid);
                    }

                    /* To avoid multiple forking for the same process */
                    if (!currProcess.forked)
                    {
                        printf("Sending MEMO Req \n");
                        /*First time for process to fork so check here if there is enough memory for the process*/
                        memoRequest.finished=0;
                        // memoRequest.algorithmNum=selectMemo;
                        memoRequest.memorySize=currProcess.memsize;
                        printf("sending now \n");
                        toMemory.m=memoRequest;
                        toMemory.mtype=8282;
                        val=msgsnd(msgq_id_SM,&toMemory,sizeof(toMemory.m),!IPC_NOWAIT);//sending request to allocate the process
                        printf("after send");
                        if(val==-1){
                            printf("error while sending memo request");
                        }
                        
                        val = msgrcv(msgq_id_SM, &toMemory, sizeof(toMemory.m), 8282, !IPC_NOWAIT);//recieving the answer from memory 0= NO SPACE    1= SPACE FOUND
                        memoRequest=toMemory.m;
                        if(val==-1){
                            printf("error while recv memo request");
                        }
                        if(memoRequest.finished==1){//means there is available memo so fork the process
                            printf("accepted from memo \n");
                            currProcess.pid = fork();
                            currProcess.forked = 1;
                            currProcess.startTime = Clk;
                            /*if this is the child process make it execute the currProcess*/
                            if (currProcess.pid == 0)
                            {
                                execl("/home/hazem/Desktop/OS_Scheduler-main1/process.out", "process.out", NULL);
                            }
                        }else{//means there is no avaiable memory
                            //----EDIT LATER add the waiting for memo space 
                            enqueue(&readyQueue,currProcess); //sending signal continue to process
                        }
                        
                    }
                    printf("running process details : pid=%d  forked=%d  arrival= %d     remain=%d      runtime=%d\n", currProcess.pid, currProcess.forked, currProcess.arrival, currProcess.remain, currProcess.runtime);
                    printf("Will compare getCLk %d with currTIme %d\n", Clk, currTime);

                    if (currProcess.remain > 0)
                    {
                        /*send the remaining time to the process.c to decrement it*/
                        processMessage.mtype = currProcess.pid % 100000;
                        processMessage.remainingTime = currProcess.remain;
                        currProcess.remain = (currProcess.remain) - 1;   //Decremting Remaining Time Then will send STOP signal
                        currProcess.runtime = (currProcess.runtime) + 1; //Incrementing Remaining Time
                        val = msgsnd(msgq_id_SP, &processMessage, sizeof(processMessage.remainingTime), !IPC_NOWAIT);

                        if (val == -1)
                        {
                            perror("Error while schedular send to process");
                        }

                        /*Details Of Process In That time step*/
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tstarted\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), currProcess.id, currProcess.arrival, currProcess.runtime, currProcess.remain, currProcess.wait);

                        kill(SIGSTOP, currProcess.pid); //sending to process to stop
                        /*enqueuing the process again to the queue to take it's turn*/
                        if (currProcess.remain > 0)
                        {
                            enqueue(&readyQueue, currProcess);
                        }
                        else
                        {
                            enqueue(&finishedQueue, currProcess);
                        }
                    }
                }
            }
        }
    }

    float CPU_utilization = ((float)usefulTime / (getClk() - 1)) * 100;
    fprintf(perfFile, " CPU utilization = %.2f\n Avg WTA = %.2f\n Avg Waiting = %.2f", CPU_utilization, TotalWTA / NumberOfProcesses, TotalWait / NumberOfProcesses);
    fclose(perfFile);
    fclose(pFile);
    fclose(mFile);

    msgctl(msgq_id_SP, IPC_RMID, (struct msqid_ds *)0);
    msgctl(msgq_id_SM, IPC_RMID, (struct msqid_ds *)0);

    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.

    destroyClk(true);
}
