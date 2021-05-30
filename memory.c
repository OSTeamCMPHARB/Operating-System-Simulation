#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char *argv[])
{
    initClk();
    int msgq_id_SM = msgget(Q3KEY, 0666 | IPC_CREAT);//linking with scheduler
    bool memory[1024];
    //initialize with false=free space
    for(int i=0;i<1024;i++){
        memory[i]=false;
    }

    if (msgq_id_SM == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    int val;
    struct mbuff message;
    int type = getpid() % 100000;
    val = msgrcv(msgq_id_SM, &message, sizeof(message), type, !IPC_NOWAIT);//recieving the request of memory allocation

    //----------------------------------Algorithms Implementation----------------------------------//

    if(message.algorithmNum==1){//First Fit Algorithm
        while (message.finished!=1)
        {
            if(message.finished==2){//deallocate the memory space
                for(int i=message.start;i<message.memorySize;i++){
                    memory[i]=false;
                }
            }else if(message.finished==0){//find a memory space
                int counter=0;
                int i=0;
                int spaceFound=false;
                int start=0;
                while(counter<message.memorySize && i<1024 && !spaceFound){
                    if(!memory[i]){
                        start=i;//indicate the start of empty space
                        while(i<1024 && !memory[i] && !spaceFound){
                            counter++;
                            i++;
                            if(counter>=message.memorySize){//checking if we found a space for process
                                spaceFound=true;//will make me exit both loops & send to scheduler that the memory needed is available
                                break;
                            }
                        }
                        counter=0;
                    }
                    i++;
                }

                //if the space needed for memory is available allocate the space to be busy & send acceptance to scheduler
                if(spaceFound){
                    message.finished=1;//used as true in case the space is found
                    //allocate the space found
                    for(int j=start;j<message.memorySize;j++){
                        memory[j]=true;
                    }
                    
                }else{
                    message.finished=0;//used as true in case the space is found
                }
                val=msgsnd(msgq_id_SM,&message,sizeof(message),!IPC_NOWAIT);
            }


            // next memory allocation request
            val = msgrcv(msgq_id_SM, &message, sizeof(message), type, !IPC_NOWAIT);//recieving the request of memory allocation
            if(message.finished==1){//if program finished break & terminate
                break;
            }

        }//main while finished
    }else if(message.algorithmNum==2){

    }else if(message.algorithmNum==3){

    }else if(message.algorithmNum==4){

    }


    
    printf ("memory terminated at %d\n",getClk());

    destroyClk(false);

    return 0;
}
