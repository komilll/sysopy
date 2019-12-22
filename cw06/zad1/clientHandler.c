#include "properties.h"

int desiredActions = 0;
int performedActions = 0;
pid_t currentChildPID = 1;
int memID = -1;
enum clientState state;

void putClientToQueue()
{
    if (desiredActions > 0 && state == IDLE){
        state = WAIT_IN_QUEUE;
        struct sysinfo info;
        sysinfo(&info);
        struct timespec monotime;
        clock_gettime(CLOCK_MONOTONIC, &monotime);
        printf("%ld.%ld Client: sit in queue ~ %d\n", monotime.tv_sec, monotime.tv_nsec, getpid());
    }
}

void createClient(int actions)
{
    desiredActions = actions;
    if (desiredActions == 0){
        return;
    }

    currentChildPID = fork();
    if (currentChildPID < 0){
        printf("Failed to create client\n");
        exit(-1);
    } else if (currentChildPID == 0){
        state = IDLE;
    } else{
        data = shmat(memID, NULL, 0);
        if (data->clientsLeft == MAX_CLIENT_COUNT){
            printf("Reached maximum number of clients to create\n");
            exit(-1);
        }
        data->clientPIDs[data->clientsLeft] = currentChildPID;
        data->clientsLeft++;
    }
}

int main(int argc, char* argv[])
{
    key_t key = ftok(PROPERTIES_PATH, PROPERTIES_PROGRAM_GENERATOR_ID);
    memID = shmget(key, sizeof(struct sharedData), 0);
    if (memID == -1){
        printf("Failed to create shared memory\n");
        exit(-1);
    }

    srand(getpid());
    for (int i = 0; i < 10; i++) {
        if (currentChildPID != 0){
            createClient(1 + rand() % 5);
            // createClient(2);
        }
    }
    data = shmat(memID, NULL, 0);

    struct sysinfo info;
    sysinfo(&info);
    int continueLoop = 1;

    while(continueLoop){
        if (currentChildPID == 0){
            takeSemaphore(data->semID);

            struct timespec monotime;
            clock_gettime(CLOCK_MONOTONIC, &monotime);

            switch (data->curBarberState){
                case WAITING_TO_SIT:
                    if (state == WAIT_IN_QUEUE && getpid() == data->clientPIDs[0]){
                        state = SIT_ON_CHAIR;
                        printf("%ld.%ld Client: sit on chair ~ %d with %d shaves left\n", monotime.tv_sec, monotime.tv_nsec, getpid(), desiredActions - performedActions);
                        data->curBarberState = SHAVING;
                    } else {
                        putClientToQueue();
                    }
                    break;
                case FINISHED_SHAVING:        
                    //Perform action only on client which is sitting on chair         
                    if (state == SIT_ON_CHAIR){
                        performedActions++;
                        if (desiredActions == performedActions){
                            printf("%ld.%ld Client: leave barber ~ %d\n", monotime.tv_sec, monotime.tv_nsec, getpid());
                            leaveQueue(memID);
                            continueLoop = 0;
                            state = IDLE;
                        } else {
                            //if (sitsLeft == 0) -> printf("leave barber due to full queue")                    
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
                        printf("%ld.%ld Client: sit on chair ~ %d with %d shaves left\n", monotime.tv_sec, monotime.tv_nsec, getpid(), desiredActions - performedActions);

                        // for (int i = 0; i < data->clientsLeft; i++){
                        //     printf("Client: %d\n", data->clientPIDs[i]);  
                        // } 
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
            printf("\n ~ No clients left - end clientHandler process ~ \n");
            return 0;
        }
    }

    return 0;
}