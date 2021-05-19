#include "headers.h"

void clearResources(int);
int msgq_id_1;
int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);

// read data from file and make queue of process and sets its data
    FILE *fp;
    fp = fopen("processes.txt", "r");
    queue pq;
    initialize(&pq);
    char buff[255];
    fgets(buff, 255, fp);
    int pid[2];
    while (fscanf(fp, "%s", buff) != EOF)
    {
        process p;
        p.id = atoi(buff);
        fscanf(fp, "%s", buff);
        p.arrival = atoi(buff);
        fscanf(fp, "%s", buff);
        p.runtime = atoi(buff);
        fscanf(fp, "%s", buff);
        p.priority = atoi(buff);
        p.remain=p.runtime;
        p.wait=0;
        enqueue(&pq, p);
    }
    
// forking two processes (clk) , (scheduler) 
// note change the file path with your path
    pid[0] = fork();
    if (pid[0] == 0)
        execl("/home/robert/Desktop/projects/OS_Scheduler/clk.out", "clk.out", NULL);

    pid[1] = fork();
    // pass argv[1] to scheduler (the chosen scheduler algo)
    if (pid[1] == 0)
        execl("/home/robert/Desktop/projects/OS_Scheduler/scheduler.out", "scheduler.out", argv[1],NULL);


    initClk();

    int x = getClk();

   // create message queue to communicate process_generator.c with schduler.c
    msgq_id_1 = msgget(Q1KEY, 0666 | IPC_CREAT);
    if (msgq_id_1 == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    msgbuff message;
    // type 1 means that there is a process arrived in the system 
    message.mtype = 1;
    int val;
    while (!isempty(&pq))
    {
        x = getClk();
     // if there is a process arrived at that time, send it to the scheduler
        if (pq.front->data.arrival <= x)
        {
            message.sp = dequeue(&pq);

            
            val = msgsnd(msgq_id_1, &message, sizeof(message.sp), !IPC_NOWAIT);
            if (val == -1)
                perror("Errror in send");
           
        }
        
    
    }
    // type 2 means that there is no other processes will arrive 
    message.mtype = 2;
    val = msgsnd(msgq_id_1, &message, sizeof(message.sp), !IPC_NOWAIT);
    if (val == -1)
        perror("Errror in send");

// wait for scheduler to finish its job
    sleep(100);

    destroyClk(true);
}

void clearResources(int signum)
{
    msgctl(msgq_id_1, IPC_RMID, (struct msqid_ds *)0);
    destroyClk(true);
    kill(getpid(), SIGKILL);
    //TODO Clears all resources in case of interruption
}
