#include "headers.h"

void clearResources(int);
int msgq_id_SPG;
int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);

    /*Reading of the processes from the file*/
    FILE *fp;
    fp = fopen("processes.txt", "r");
    queue processesQueue;
    initialize(&processesQueue);
    char buff[255];
    fgets(buff, 255, fp);
    int pid[2];
    while (fscanf(fp, "%s", buff) != EOF){
        process procObj;
        procObj.id = atoi(buff);
        fscanf(fp, "%s", buff);
        procObj.arrival = atoi(buff);
        fscanf(fp, "%s", buff);
        procObj.runtime = atoi(buff);
        fscanf(fp, "%s", buff);
        procObj.priority = atoi(buff);
        procObj.remain=procObj.runtime;
        procObj.forked=0;
        procObj.wait=0;
        enqueue(&processesQueue, procObj);
    }
    
    // forking two processes (clk) , (scheduler) 
    // note change the file path with your path
    pid[0] = fork();
    if (pid[0] == 0)
        execl("/home/hazem/Desktop/OS_Scheduler-main/clk.out", "clk.out", NULL);

    pid[1] = fork();
    // pass argv[1] to scheduler (the chosen scheduler algo)
    if (pid[1] == 0)
        execl("/home/hazem/Desktop/OS_Scheduler-main/scheduler.out", "scheduler.out", argv[1],NULL);


    initClk();//Initializing the clock at the start of process generation

    int currTime = getClk();//saving the current clock tick in currTime

    // create message queue to communicate process_generator.c with schduler.c
    msgq_id_SPG = msgget(Q1KEY, 0666 | IPC_CREAT);
    if (msgq_id_SPG == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    msgbuff message;
    // type 1 means that there is a process arrived in the system 
    message.allProcessesGenerated = 1;
    int val;
    while (!isempty(&processesQueue))
    {
        currTime = getClk();
        // if there is a process arrived at that time, send it to the scheduler
        if (processesQueue.front->data.arrival <= currTime)
        {
            val = msgsnd(msgq_id_SPG, &message, sizeof(message.processObj), !IPC_NOWAIT);
            /*if more than one process arrived at same time*/
            while(processesQueue.front->data.arrival<=currTime){
                message.processObj = dequeue(&processesQueue);
                val = msgsnd(msgq_id_SPG, &message, sizeof(message.processObj), !IPC_NOWAIT);

                if (val == -1){
                    perror("Errror in send");
                }
            }

            if (val == -1){
                perror("Errror in send");
            }
        }
    }
    /*Sending 2 to schedular to say no more processes is arriving, which will lead to make it's flag = 0 */
    message.allProcessesGenerated = 2;
    val = msgsnd(msgq_id_SPG, &message, sizeof(message.processObj), !IPC_NOWAIT);
    if (val == -1){
        perror("Errror in send");
    } 
    // wait for scheduler to finish its job
    sleep(100);

    destroyClk(true);
}

void clearResources(int signum)
{
    msgctl(msgq_id_SPG, IPC_RMID, (struct msqid_ds *)0);
    destroyClk(true);
    kill(getpid(), SIGKILL);
    //TODO Clears all resources in case of interruption
}
