#include "properties.h"

int desiredActions = 0;
int performedActions = 0;
pid_t currentChildPID = 1;
int memID = -1;
enum clientState state;

void putClientToQueue()
{
    //If client has entered barber shop - put him in queue
    if (state == IDLE && desiredActions > 0){
        struct timespec monotime;
        clock_gettime(CLOCK_MONOTONIC, &monotime);

        for (int i = 0; i < data->clientsLeft; i++){
            if (data->clientPIDs[i] == getpid()){
                printf("%ld.%ld Client: sit in queue ~ %d\n", monotime.tv_sec, monotime.tv_nsec, getpid());
                state = WAIT_IN_QUEUE;
                return;
            }
        }       

        desiredActions = 0; 
        printf("%ld.%ld Client: leave shop due to full queue ~ %d\n", monotime.tv_sec, monotime.tv_nsec, getpid());
        exit(0);
    }
}

void createClient(int actions)
{
    //Set how many shaves client want
    if (actions == 0){
        return;
    }
    desiredActions = actions;

    //Create client process
    currentChildPID = fork();
    if (currentChildPID < 0){
        printf("Failed to create client\n");
        exit(-1);
    } else if (currentChildPID == 0){
        //Initialize client state - don't rely on zero-initializiation
        state = IDLE;
    } else{
        //Add client data to queued
        data = shmat(memID, NULL, 0);
        if (data->clientsLeft == MAX_CLIENT_COUNT){
            return;
        }
        data->clientPIDs[data->clientsLeft] = currentChildPID;
        data->clientsLeft++;
    }
}

int main(int argc, char* argv[])
{
    //Get semaphore and shared memory
    key_t key = ftok(PROPERTIES_PATH, PROPERTIES_PROGRAM_GENERATOR_ID);
    memID = shmget(key, sizeof(struct sharedData), 0);
    if (memID == -1){
        printf("Failed to create shared memory\n");
        exit(-1);
    }
    data = shmat(memID, NULL, 0);
    int clientsCount = 10;

    if (argc == 3){
        clientsCount = atoi(argv[1]);   
        int shaveCount = atoi(argv[2]);
        //Create clients with same shave number
        for (int i = 0; i < clientsCount; i++) {
            if (currentChildPID != 0){
                createClient(shaveCount);
            }
        }
    } else if (argc == 2 || argc == 1){
        if (argc == 2){
            clientsCount = atoi(argv[1]);
        }
        //Create clients with random shave number
        srand(getpid());
        for (int i = 0; i < clientsCount; i++) {
            if (currentChildPID != 0){
                createClient(1 + rand() % 5);
            }
        }
    } 

    struct timespec monotime;
    int continueLoop = 1;

    while(continueLoop){
        if (currentChildPID == 0){
            takeSemaphore(data->semID);
            clock_gettime(CLOCK_MONOTONIC, &monotime);

            switch (data->curBarberState){
                case WAITING_TO_SIT:
                    //Barber has invited client to sit - check if client is the invited one
                    if (state == WAIT_IN_QUEUE && getpid() == data->clientPIDs[0]){
                        state = SIT_ON_CHAIR;
                        printf("%ld.%ld Client: sit on chair ~ %d with %d shaves left\n", monotime.tv_sec, monotime.tv_nsec, getpid(), desiredActions - performedActions);
                        data->curBarberState = SHAVING;
                    } 
                    //Put him in queue if standing and not the invited one
                    else {
                        putClientToQueue();
                    }
                    break;
                case FINISHED_SHAVING:        
                    //Perform action only on client which is sitting on chair         
                    if (state == SIT_ON_CHAIR){
                        //Do shave and check if all shaves already done
                        performedActions++;
                        if (desiredActions == performedActions){
                            printf("%ld.%ld Client: leave barber ~ %d\n", monotime.tv_sec, monotime.tv_nsec, getpid());
                            leaveQueue(memID);
                            continueLoop = 0;
                            state = IDLE;
                        } else {
                            //Wait for next shave in queue
                            printf("%ld.%ld Client: sit in queue ~ %d\n", monotime.tv_sec, monotime.tv_nsec, getpid());
                            goAtQueueEnd(memID);
                            state = WAIT_IN_QUEUE;
                        }
                        data->curBarberState = CHECK_QUEUE;
                    } else {
                        putClientToQueue();
                    }
                    break;               
                case SLEEPING:
                    //Barber is sleeping only if there is no client on chair - safe to wake up and sit
                    if (state == IDLE){
                        state = SIT_ON_CHAIR;
                        data->curBarberState = WAKING_UP;
                        moveToQueueBeginning(memID, getpid());
                        printf("%ld.%ld Client: wake up barber ~ %d\n", monotime.tv_sec, monotime.tv_nsec, getpid());
                        clock_gettime(CLOCK_MONOTONIC, &monotime);
                        printf("%ld.%ld Client: sit on chair ~ %d with %d shaves left\n", monotime.tv_sec, monotime.tv_nsec, getpid(), desiredActions - performedActions);
                    }
                    break;

                case SHAVING:
                case CHECK_QUEUE:
                default:
                    putClientToQueue();
                    break;
            }
            giveSemaphore(data->semID);
        } else if (data->clientsLeft == 0){
            //No clients in barber shop - close handler process
            printf("\n ~ No clients left - end clientHandler process ~ \n");
            return 0;
        }
    }

    return 0;
}