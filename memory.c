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
                        message.m.end=start+message.m.memorySize-1;
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
    {//Next fit Algorithm
        printf("hello from next fit algorithm \n");
        int last_filled_bit = -1;
        int empty_bytes = 1024;
        while(true)
        {
            val = msgrcv(msgq_id_SM, &message, sizeof(message) - sizeof(long), 0, !IPC_NOWAIT);
            if(val != -1)
            {
                if (message.mtype == 1)
                {
                    printf("number of full bytes = %d \n", empty_bytes);
                    break;
                }
                else if(message.mtype == 2)
                {
                    for (int i = message.m.start; i < message.m.memorySize + message.m.start; i++)
                    {
                        memory[i] = false;
                        empty_bytes++;
                    }
                }
                else if(message.mtype == 3)
                {
                    int begin_search_index = last_filled_bit;
                    last_filled_bit++;
                    int start_memory = last_filled_bit;
                    int free_space_counter = 0;
                    bool found_enough_space = false;
                    while(last_filled_bit != begin_search_index && !found_enough_space)
                    {
                        if(last_filled_bit == 1024) 
                        {
                            last_filled_bit = 0;
                            start_memory = 0;
                            free_space_counter = 0;
                        }
                        if(memory[last_filled_bit]){
                            last_filled_bit++;
                            start_memory = last_filled_bit;
                            free_space_counter = 0;
                            continue;
                        }
                        free_space_counter++;
                        last_filled_bit++;
                        if(free_space_counter == message.m.memorySize - 1){
                            found_enough_space =  true;
                        } 
                    }
                    if(found_enough_space)
                    {
                        printf("finished computing \n");
                        message.mtype = 1; //used as true in case the space is found
                        message.m.start = start_memory;
                        message.m.end=free_space_counter + start_memory;
                        for(int i = start_memory; i <= free_space_counter + start_memory; i++){
                            memory[i] = true;
                            empty_bytes--;
                        }
                    }
                    else
                    {
                        message.m.start = -1;
                    }
                    val = msgsnd(msgq_id_MS, &message, sizeof(message) - sizeof(long), !IPC_NOWAIT);
                }
            }
        }
    }
    //////////////////////////////////////end trial///////////////////////
    else if (memoAlgorithm == 3)
    {
        int x=0;

        while (true)
        {
            val = msgrcv(msgq_id_SM, &message, sizeof(message) - sizeof(long), 0, !IPC_NOWAIT); //recieving the request of memory allocation 3=allocate / 1=terminate / 2=deallocate
            if (val != 0)
            {
                if (message.mtype == 1)
                {
                   // printf("%d",x);
                    break; //terminate program is finished
                }
                else if (message.mtype == 2)
                { //deallocate the following memory
                x--;
                    for (int i = message.m.start; i < message.m.memorySize + message.m.start; i++)
                    {
                        memory[i] = false;
                    }
                }
                else if (message.mtype == 3)
                {
                    //printf("computing in memo \n");
                    int counter = 0;
                    int i = 0;
                    int start = 0;
                    int min = 5000;
                    int temp;
                    while (i < 1024)
                    {
                        temp = 0;
                        counter = i;
                        while (i < 1024 && !memory[i])
                        {
                            temp++;
                            i++;
                        }
                        if (temp >= message.m.memorySize && temp < min)//1023
                        {
                            start = counter;
                            min = temp;
                        }
                        i++;
                    }
                    if (min != 5000)
                    {
                        printf("finished computing \n");
                        message.mtype = 1; //used as true in case the space is found
                        message.m.start = start;
                        message.m.end= message.m.memorySize + start-1;
                        //allocate the space found
                        x++;
                        for (int j = start; j < message.m.memorySize + start; j++)
                        {
                            memory[j] = true;
                        }
                    }
                    else
                    {
                        message.m.start = -1;
                    }
                    val = msgsnd(msgq_id_MS, &message, sizeof(message) - sizeof(long), !IPC_NOWAIT);
                }
            }
        }
    }
    else if (memoAlgorithm == 4)
    {
        int x=1024;
        while (true)
        {
            val = msgrcv(msgq_id_SM, &message, sizeof(message) - sizeof(long), 0, !IPC_NOWAIT); //recieving the request of memory allocation
            if (val != 0)
            {
                if (message.mtype == 1)
                {
                    printf("memo empty size:  %d\n",x);
                    break;
                }
                else if (message.mtype == 2)
                {
                   

                    for (int i = message.m.start; i <=message.m.end ; i++)
                    {
                        memory[i] = false;
                        x++;
                    }
                }
                else
                {
                    int sizeB = 1024;
                    int spaceFound = 1;
                    while (message.m.memorySize <= sizeB)
                    {
                        sizeB = sizeB / 2;
                    }
                    sizeB = sizeB * 2;
                    int start = 0;
                    int end = sizeB - 1;

                    for (int i = start; i <= end; i++)
                    {
                        if (memory[i] == true)
                        {
                            start = start + sizeB;
                            i = start;
                            end = end + sizeB;
                            if (end > 1023)
                            {
                                spaceFound = 0;
                                break;
                            }
                        }
                    }
                    if (spaceFound)
                    {
                        printf("finished computing \n");
                        message.mtype = 1; //used as true in case the space is found
                        message.m.start = start;
                        message.m.end = end; ///the end index to print it in memory.log
                        //allocate the space found
                        for (int j = start; j <= end; j++)
                        {
                            memory[j] = true;
                            x--;
                        }
                    }
                    else
                    {
                        message.mtype = 2; //used as true in case the space is not found
                        message.m.start=-1;
                    }

                    //if the space needed for memory is available allocate the space to be busy & send acceptance to scheduler

                    printf("sending permission \n");
                    val = msgsnd(msgq_id_MS, &message, sizeof(message) - sizeof(long), !IPC_NOWAIT);
                }
            }
        }
    }

    printf("memory terminated at %d\n", getClk());
    destroyClk(false);

    return 0;
}
