#include "headers.h"

int main(int agrc, char *argv[])
{

    initClk();
    int count = 0;
    int memoAlgorithm = atoi(argv[1]);
    bool memory[1024];
    //initialize with false=free space
    for (int i = 0; i < 1024; i++)
    {
        memory[i] = false;
    }


    int msgq_id_SM = getID_SM();
    int msgq_id_MS = getID_MS();

    
    int val;

    memoBuff message;
    // //----------------------------------Algorithms Implementation----------------------------------//
    printf("select memo algo = %d\n", memoAlgorithm);
    if (memoAlgorithm == 1)
    { //First Fit Algorithm
        while (true)
        {
            val = msgrcv(msgq_id_SM, &message, sizeof(message) - sizeof(long), 0, !IPC_NOWAIT); //recieving the request of memory allocation 3=allocate / 1=terminate / 2=deallocate
            if (val != 0)
            {
                if (message.mtype == 1)
                {
                    break;//terminate program is finished
                }
                else if (message.mtype == 2)
                {//deallocate the following memory
                    for (int i = message.m.start; i < message.m.memorySize + message.m.start; i++)
                    {
                        memory[i] = false;
                    }

                }
                else if(message.mtype==3)
                {
                    //printf("computing in memo \n");
                    int counter = 0;
                    int i = 0;
                    int spaceFound = 0;
                    int start = 0;
                    while (counter < message.m.memorySize && i < 1024 && !spaceFound)
                    {
                        if (!memory[i])
                        {
                            start = i; //indicate the start of empty space
                            while (i < 1024 && !memory[i] && !spaceFound)
                            {
                                counter++;
                                i++;
                                if (counter >= message.m.memorySize)
                                {                      //checking if we found a space for process
                                    spaceFound = 1; //will make me exit both loops & send to scheduler that the memory needed is available
                                    break;
                                }
                            }
                            counter = 0;
                        }
                        i++;
                    }
                    //if the space needed for memory is available allocate the space to be busy & send acceptance to scheduler
                    if (spaceFound)
                    {
                        printf("finished computing \n");
                        message.mtype = 1; //used as true in case the space is found
                        message.m.start=start;
                        //allocate the space found
                        for (int j = start; j < message.m.memorySize + start; j++)
                        {
                            memory[j] = true;
                        }
                    }
                    else
                    {
                        message.m.start=-1;
                    }
                    //printf("sending permission \n");
                    
                    val = msgsnd(msgq_id_MS, &message, sizeof(message) - sizeof(long), !IPC_NOWAIT);
                    spaceFound=0;
                }
            }
        }
    }
    else if (memoAlgorithm == 2)
    {
    }
    else if (memoAlgorithm == 3)
    {
    }
    else if (memoAlgorithm == 4)
    {
    }

    printf("memory terminated at %d\n", getClk());
    destroyClk(false);

    return 0;
}
