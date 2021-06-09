#include "headers.h"

void clearResources(int);
int msgq_id_SPG;
int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    int schAlgorithm=-1;
    int memAlgorithm=-1;
    int freqTime=-1;
    int txtIndex=-1;
    if(argv[4][1] == 'q'){
        for(int i=0;i<8;i++){
            printf("argc = %d", argc);
            //./process_generator.out processes.txt -sch 4 -q 2 -mem 2
            printf("the curr argv %s \n",argv[i]);
            if(argv[i][1]=='s'){
                schAlgorithm=++i;
                printf("the sch algo is %i \n",schAlgorithm);
            }else if(argv[i][1]=='q'){
                freqTime=++i;
                printf("the time freq is %i \n",freqTime);
            }else if(argv[i][1]=='m'){
                memAlgorithm=++i;
                printf("the memo algo is %i \n",memAlgorithm);
            }else if(argv[i][strlen(argv[i])-1]=='t'){
                txtIndex=i;
            }
        }
    }
    else {
        freqTime = 0;
        for(int i = 0;i < 6; i++){
            printf("argc = %d", argc);
            //./process_generator.out processes.txt -sch 4 -q 2 -mem 2
            printf("the curr argv %s \n",argv[i]);
            if(argv[i][1]=='s'){
                schAlgorithm=++i;
                printf("the sch algo is %i \n",schAlgorithm);
            }
            else if(argv[i][1]=='m'){
                memAlgorithm=++i;
                printf("the memo algo is %i \n",memAlgorithm);
            }else if(argv[i][strlen(argv[i])-1]=='t'){
                txtIndex=i;
            }
        }
    }

    /*Reading of the processes from the file*/
    ///scheduler.o testcase.txt -sch 5 -q 2 -mem 3
    FILE *fp;
    fp = fopen(argv[txtIndex], "r");
    queue processesQueue;
    initialize(&processesQueue);
    char buff[255];
    fgets(buff, 255, fp);
    int pid[3];
    while (fscanf(fp, "%s", buff) != EOF)
    {
        process procObj;
        procObj.id = atoi(buff);
        fscanf(fp, "%s", buff);
        procObj.arrival = atoi(buff);
        fscanf(fp, "%s", buff);
        procObj.runtime = atoi(buff);
        fscanf(fp, "%s", buff);
        procObj.priority = atoi(buff);
        fscanf(fp, "%s", buff);
        procObj.memsize = atoi(buff);
        procObj.remain = procObj.runtime;
        procObj.wait = 0;
        procObj.isblocked = 0;
        procObj.effectinetime=0;
        enqueue(&processesQueue, procObj);
    }

    // forking two processes (clk) , (scheduler)
    // note change the file path with your path
    char path[256];
    getcwd(path, sizeof(path));
    strcat(path, "/clk.out");
    printf("%s\n", path);
    pid[0] = fork();
    if (pid[0] == 0)
        execl(path, "clk.out", NULL);

    getcwd(path, sizeof(path));
    strcat(path, "/memory.out");
    pid[2] = fork();
    if (pid[2] == 0)
        execl(path, "memory.out", argv[memAlgorithm], NULL);

    getcwd(path, sizeof(path));
    strcat(path, "/scheduler.out");
    pid[1] = fork();
    // pass argv[1] to scheduler (the chosen scheduler algo)
    if (pid[1] == 0)
        execl(path, "scheduler.out", argv[schAlgorithm],argv[freqTime], NULL);

    printf("Clock not Initialized \n");
    initClk(); //Initializing the clock at the start of process generation
    printf("Clock Initialized \n");

    int currTime = getClk(); //saving the current clock tick in currTime

    // create message queue to communicate process_generator.c with schduler.c
    msgq_id_SPG = getID_SPG();

    msgbuff message;
    // type 1 means that there is a process arrived in the system
    message.allProcessesGenerated = 1;
    queue currProcesses;
    initialize(&currProcesses);
    int val;
    while (!isempty(&processesQueue))
    {
        currTime = getClk();
        // if there is a process arrived at that time, send it to the scheduler
        if (processesQueue.front->data.arrival <= currTime)
        {
            while (!isempty(&processesQueue) && processesQueue.front->data.arrival <= currTime)
            {
                enqueue(&currProcesses, dequeue(&processesQueue));
            }
            while (!isempty(&currProcesses))
            {
                message.processObj = dequeue(&currProcesses);
                if (currProcesses.count > 0)
                {
                    message.moreProcess = 1;
                }
                else
                {
                    message.moreProcess = 0;
                }
                printf(" i will send");
                val = msgsnd(msgq_id_SPG, &message, sizeof(message.processObj), !IPC_NOWAIT);
                printf("sent");
                if (val == -1)
                {
                    perror("Errror in send");
                }
            }
        }
    }
    /*Sending 2 to schedular to say no more processes is arriving, which will lead to make it's flag = 0 */
    message.allProcessesGenerated = 2;
    val = msgsnd(msgq_id_SPG, &message, sizeof(message.processObj), !IPC_NOWAIT);
    if (val == -1)
    {
        perror("Errror in send");
    }
    
    // wait for scheduler to finish its job
    int stat_loc;
    waitpid(pid[1], &stat_loc, 0);

    destroyClk(true);
}

void clearResources(int signum)
{
    destroyClk(true);
    msgctl(msgq_id_SPG, IPC_RMID, (struct msqid_ds *)0);
    msgctl(getID_SP(), IPC_RMID, (struct msqid_ds *)0);
    msgctl(getID_MS(), IPC_RMID, (struct msqid_ds *)0);
    msgctl(getID_SM(), IPC_RMID, (struct msqid_ds *)0);
    kill(getpid(), SIGKILL);
    //TODO Clears all resources in case of interruption
}
