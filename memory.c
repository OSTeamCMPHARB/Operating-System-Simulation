#include "headers.h"

int main(int agrc, char *argv[])
{
    int count=0;
    initClk();
    // while(count<10){
    //     printf("Memory sssssssssssssssssss \n ");

    //     printf("Memory started ");
    //     count++;
    // }
    
    int memoAlgorithm=atoi(argv[1]);
    
    
    bool memory[1024];
    struct memobuff toScheduler;
    //initialize with false=free space
    for(int i=0;i<1024;i++){
        memory[i]=false;
    }
    

    int msgq_id_SM = msgget(Q3KEY, 0664 | IPC_CREAT);//linking with scheduler
    if (msgq_id_SM == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    
    int val;
    struct mbuff message;
    int type = getpid() % 100000;
    val = msgrcv(msgq_id_SM, &toScheduler, sizeof(toScheduler.m), 0, !IPC_NOWAIT);//recieving the request of memory allocation
    if(val==-1){
        perror("error in recieving from scheduler to memo \n");
    }
    message=toScheduler.m;
    printf("Memory select algo is %d \n",memoAlgorithm);

    // //----------------------------------Algorithms Implementation----------------------------------//

    if(memoAlgorithm==1){//First Fit Algorithm
        while (message.finished!=1)
        {
            if(message.finished==2){//deallocate the memory space
                for(int i=message.start;i<message.memorySize+message.start;i++){
                    memory[i]=false;
                }
            }else if(message.finished==0){//find a memory space
                printf("computing in memo \n");
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
                    printf("finished computing \n");
                    message.finished=1;//used as true in case the space is found
                    //allocate the space found
                    for(int j=start;j<message.memorySize+start;j++){
                        memory[j]=true;
                    }
                    
                }else{
                    message.finished=0;//used as true in case the space is found
                }
                toScheduler.mtype=8282;
                toScheduler.m=message;
                printf("sending permission \n");
                val=msgsnd(msgq_id_SM,&toScheduler,sizeof(toScheduler.m),!IPC_NOWAIT);
            }


            // next memory allocation request

            val = msgrcv(msgq_id_SM, &toScheduler, sizeof(toScheduler.m), 0, !IPC_NOWAIT);//recieving the request of memory allocation
            message=toScheduler.m;
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
