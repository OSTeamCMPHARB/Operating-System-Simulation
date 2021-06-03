#include "headers.h"

int main(int argc, char *argv[])
{
    initClk();

    //create message queue to communicate process_generator.c with schduler.c
    int msgq_id_SPG = getID_SPG();
    //create message queue to communicate schduler.c with process.c
    int msgq_id_SP = getID_SP();
    //create message queue to communicate scheduler.c with memory.c
    int msgq_id_SM = getID_SM();
    int msgq_id_MS = getID_MS();

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
    fprintf(mFile, "#At\ttime\tx\tallocated\ty\tbytes\tfor process\tz\tfrom\ti\tto\tj \n");

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
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tblocked\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                        processRunning->isblocked = 1;
                        enqueuepriority(&readyQueue, currProcess, processRunning->priority); //change
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
        memoBuff memoRequestSend;
        memoBuff memoRequestRecieve;
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
                    while(!isemptypriority(&readyQueue))
                    {
                        currProcess = dequeuepriority(&readyQueue);
                        if(currProcess.isblocked == 1)
                        {
                            processRunning = &currProcess;
                            while(!isemptypriority(&temp))
                            {
                                process tempProc = dequeuepriority(&temp);
                                enqueuepriority(&readyQueue, tempProc, tempProc.remain);
                            }
                            break;
                        }
                        memoRequestSend.m.memorySize = currProcess.memsize;
                        memoRequestSend.m.proccesID = currProcess.id;
                        memoRequestSend.m.start = -1;
                        memoRequestSend.mtype = 3;
                        val = msgsnd(msgq_id_SM, &memoRequestSend, sizeof(memoRequestSend) - sizeof(long), !IPC_NOWAIT);
                        printf("after send 1 \n");
                        val = msgrcv(msgq_id_MS, &memoRequestRecieve, sizeof(memoRequestRecieve) - sizeof(long), 0, !IPC_NOWAIT);
                        printf("after recieve 1 \n"); 
                        if(val == -1) 
                        {
                            printf("error in communication with memory");
                        }
                        if(memoRequestRecieve.m.start == -1)
                        {
                            enqueuepriority(&temp, currProcess, currProcess.remain);
                        }
                        else
                        {
                            currProcess.address = memoRequestRecieve.m.start;
                            processRunning = &currProcess;
                            fprintf(mFile, "#At\ttime\t%d\tallocated\t%d\tbytes\tfor process\t%d\tfrom\t%d\tto\t%d \n", getClk(), memoRequestRecieve.m.memorySize, memoRequestRecieve.m.proccesID, memoRequestRecieve.m.start, memoRequestRecieve.m.start + memoRequestRecieve.m.memorySize - 1);
                            while(!isemptypriority(&temp))
                            {
                                process tempProc = dequeuepriority(&temp);
                                enqueuepriority(&readyQueue, tempProc, tempProc.remain);
                            }
                            break;
                        }
                    }

                    //currProcess = dequeuepriority(&readyQueue);
                    //processRunning = &currProcess;
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
                    bool enterflag = true;
                    process* tempPtr = beek(&readyQueue);
                    if(tempPtr->isblocked != 1 && processRunning->remain > tempPtr->remain)
                    {
                        memoRequestSend.m.memorySize = tempPtr->memsize;
                        memoRequestSend.m.proccesID = tempPtr->id;
                        memoRequestSend.m.start = -1;
                        memoRequestSend.mtype = 3;
                        val = msgsnd(msgq_id_SM, &memoRequestSend, sizeof(memoRequestSend) - sizeof(long), !IPC_NOWAIT);
                        printf("after send 2 \n");
                        if(val == -1)
                        {
                            printf("error while sending to memory");
                        }
                        val = msgrcv(msgq_id_MS, &memoRequestRecieve, sizeof(memoRequestRecieve) - sizeof(long), 0, !IPC_NOWAIT);
                        printf("after recieve 2 \n");
                        if(val == -1)
                        {
                            printf("error while recieve from memory");
                        }
                        if(memoRequestRecieve.m.start == -1)
                        {
                            enterflag = false;
                        }
                        else
                        {
                            tempPtr->address = memoRequestRecieve.m.start;
                            fprintf(mFile, "#At\ttime\t%d\tallocated\t%d\tbytes\tfor process\t%d\tfrom\t%d\tto\t%d \n", getClk(), memoRequestRecieve.m.memorySize, memoRequestRecieve.m.proccesID, memoRequestRecieve.m.start, memoRequestRecieve.m.start + memoRequestRecieve.m.memorySize - 1);
                        }
                    }
                    if (processRunning->remain > beek(&readyQueue)->remain && enterflag) //change
                    {
                        kill(processRunning->pid, SIGSTOP);
                        fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tblocked\t\tarrived\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), processRunning->id, processRunning->arrival, processRunning->runtime, processRunning->remain, processRunning->wait);
                        processRunning->isblocked = 1;
                        enqueuepriority(&readyQueue, currProcess, processRunning->remain); //change
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
                        memoRequestSend.m.memorySize = currProcess.memsize;
                        memoRequestSend.m.proccesID = currProcess.id;
                        memoRequestSend.m.start = currProcess.address;
                        memoRequestSend.mtype = 2;
                        val = msgsnd(msgq_id_SM, &memoRequestSend, sizeof(memoRequestSend) - sizeof(long), !IPC_NOWAIT);
                        fprintf(mFile, "#At\ttime\t%d\tfreed\t\t%d\tbytes\tfor process\t%d\tfrom\t%d\tto\t%d \n", getClk(), memoRequestSend.m.memorySize, memoRequestSend.m.proccesID, memoRequestSend.m.start, memoRequestSend.m.start + memoRequestSend.m.memorySize - 1);
                        printf("after send 3 \n");
                        if(val == -1)
                        {
                            printf("error while sending to memory at dealocation");
                        }
                        processRunning = NULL;
                    }
                }
            }
            //////////////////////////////////////////////////end trial
        }

        memoRequestSend.mtype = 1;
        val = msgsnd(msgq_id_SM, &memoRequestSend, sizeof(memoRequestSend) - sizeof(long), !IPC_NOWAIT);
        if(val == -1)
        {
            printf("error while sending to memory at the end");
        }
    }
    else if (selectAlgo == 5)
    { //RR Round Robin
        memoBuff memoRequests;
        msgbuff message;
        pbuff processMessage;
        queue readyQueue;    //queue for ready processes
        queue finishedQueue; //queue for finished processes
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
                if (message.allProcessesGenerated == 2)
                {
                    flag = 0;
                }
                else
                {
                    message.processObj.runtime = 0; //Didn't run before so intialize it to zero
                    message.processObj.forked = 0;  //Wasn't forked before
                    message.processObj.real = 1;    //Wasn't forked before
                    enqueue(&readyQueue, message.processObj);
                    NumberOfProcesses++;
                    while (message.moreProcess)
                    {
                        val = msgrcv(msgq_id_SPG, &message, sizeof(message.processObj), 0, !IPC_NOWAIT);
                        message.processObj.runtime = 0; //Didn't run before so intialize it to zero
                        message.processObj.forked = 0;  //Wasn't forked before
                        message.processObj.real = 1;    //Wasn't forked before
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
                currTime = Clk;

                if (!isempty(&finishedQueue))
                { //If process is finished
                    currProcess = dequeue(&finishedQueue);
                    memoRequests.m.memorySize=currProcess.memsize;
                    memoRequests.m.proccesID=currProcess.id;
                    memoRequests.m.start=currProcess.address;
                    memoRequests.mtype=2;
                    fprintf(mFile, "#At\ttime\t%d\tfreed\t\t%d\tbytes\tfor process\t%d\tfrom\t%d\tto\t%d \n", getClk(), memoRequests.m.memorySize, memoRequests.m.proccesID, memoRequests.m.start, memoRequests.m.start + memoRequests.m.memorySize - 1);

                    val = msgsnd(msgq_id_SM, &memoRequests, sizeof(memoRequests) - sizeof(long), !IPC_NOWAIT);//deallocate the memory
                    if(val==-1){
                        perror("error while deallocating request");
                    }
                    currProcess.real = 0;
                    currProcess.finishTime = Clk;
                    currProcess.wait = currProcess.finishTime - currProcess.arrival - currProcess.runtime;
                    TA = currProcess.wait + currProcess.runtime; //total turn around , waiting + running
                    WTA = (float)TA / currProcess.runtime;       //total turn around over runtime
                    TotalWait += currProcess.wait;
                    TotalWTA += WTA;
                    fprintf(pFile, "At\ttime\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n", currTime, currProcess.id, currProcess.arrival, currProcess.runtime, currProcess.remain, currProcess.wait, TA, WTA);
                    
                    if(isempty(&finishedQueue) && isempty(&readyQueue) && flag==0){
                        memoRequests.mtype=1;
                        val = msgsnd(msgq_id_SM, &memoRequests, sizeof(memoRequests) - sizeof(long), !IPC_NOWAIT);//deallocate the memory
                    }
                }

                if (!isempty(&readyQueue))
                {
                    currProcess = dequeue(&readyQueue); //sending signal continue to process
                    if (currProcess.forked==1)
                    {
                        kill(SIGCONT, currProcess.pid);
                    }

                    /* To avoid multiple forking for the same process */
                    if (currProcess.forked !=1)
                    {
                        memoRequests.m.memorySize=currProcess.memsize;
                        memoRequests.m.proccesID=currProcess.id;
                        memoRequests.m.start=-1;
                        memoRequests.mtype=3;
                        val = msgsnd(msgq_id_SM, &memoRequests, sizeof(memoRequests) - sizeof(long), IPC_NOWAIT);//allocate the memory
                        if(val==-1){
                            perror("failed to send request to memo");
                        }
                        val=msgrcv(msgq_id_MS, &memoRequests, sizeof(memoRequests) - sizeof(long), 0, !IPC_NOWAIT); //recieving the answer from memory 0= NO SPACE    1= SPACE FOUND
                        if(memoRequests.m.start!=-1){
                            
                            currProcess.pid = fork();
                            currProcess.forked = 1;
                            currProcess.startTime = Clk;
                            currProcess.address=memoRequests.m.start;

                            /*if this is the child process make it execute the currProcess*/
                            if (currProcess.pid == 0)
                            {
                                execl("/home/bishoy/Desktop/OSproject/OS_Scheduler-main/process.out", "process.out", NULL);
                            }
                            fprintf(mFile, "#At\ttime\t%d\tallocated\t%d\tbytes\tfor process\t%d\tfrom\t%d\tto\t%d \n", getClk(), memoRequests.m.memorySize, memoRequests.m.proccesID, memoRequests.m.start, memoRequests.m.start + memoRequests.m.memorySize - 1);
                            printf("running process details : pid=%d  forked=%d  arrival= %d     remain=%d      runtime=%d\n", currProcess.id, currProcess.forked, currProcess.arrival, currProcess.remain, currProcess.runtime);
                        }
                    }
                    
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

     //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.
//val = msgsnd(msgq_id_SM, &memoMessage, sizeof(memoMessage) - sizeof(long), !IPC_NOWAIT);//Deallocate the memory
//val = msgrcv(msgq_id_MS, &memoMessage, sizeof(memoMessage) - sizeof(long), 0, !IPC_NOWAIT); //recieving the answer from memory 0= NO SPACE    1= SPACE FOUND
    destroyClk(true);
}
