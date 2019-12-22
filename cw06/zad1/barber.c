#include "properties.h"

int semID = -1;
int memID = -1;

void shave()
{
    struct timespec monotime;
    clock_gettime(CLOCK_MONOTONIC, &monotime);   
    printf("%ld.%ld Barber: start shaving client %d\n", monotime.tv_sec, monotime.tv_nsec, data->clientPIDs[0]);
}

void cleanUpAtExit()
{
    //Clean up at exit
    if (semID != -1){
        semctl(semID, 0, IPC_RMID);
    }
    if (memID != -1){
        shmctl(memID, IPC_RMID, 0);
    }
}

void initializeBarber()
{
    //Initialize semaphore
    key_t key = ftok(PROPERTIES_PATH, PROPERTIES_PROGRAM_GENERATOR_ID);

    semID = semget(key, 1, IPC_CREAT | IPC_EXCL);
    if (semID == -1){
        printf("Failed to create semaphore\n");
        exit(-1);
    }
    semctl(semID, 0, SETVAL, 0);

    //Initialize shared memory
    memID = shmget(key, sizeof(struct sharedData), IPC_CREAT | IPC_EXCL | 0666);
    if (memID == -1){
        printf("Failed to create shared memory\n");
        exit(-1);
    }    

    data = shmat(memID, NULL, 0);
    if (data != NULL){
        data->semID = semID;
        data->clientsLeft = 0;
        data->curBarberState = CHECK_QUEUE;
    }
}

int main(int argc, char* argv[])
{
    //Prepare signals to stop execution
    if (signal(SIGINT, cleanUpAtExit) == SIG_ERR){
        //--Practial purpose - it's easier to test with Ctrl+C
        printf("Failed to assign SIGINT function in client\n");
        return -1;
    }
    if (signal(SIGTERM, cleanUpAtExit) == SIG_ERR){
        printf("Failed to assign SIGTERM function in client\n");
        return -1;
    }

    //Prepare atexit/signal for cleanup
    if (atexit(cleanUpAtExit) == -1){
        printf("Failed to create atexit function in client\n");
        return -1;
    }

    //Initialize sempahore and shared memory
    initializeBarber();

    //Set value of semaphore to 1
    giveSemaphore(semID);

    //Check data and initialize barber state
    data = shmat(memID, NULL, 0);
    if (data == NULL){
        printf("Failed to get data\n");
        return -1;
    }
    data->curBarberState = CHECK_QUEUE;
        
    struct timespec monotime;

    while (1){
        takeSemaphore(semID);

        //Get current time
        clock_gettime(CLOCK_MONOTONIC, &monotime);   
        
        switch (data->curBarberState){
            case CHECK_QUEUE:
                printf("%ld.%ld Barber: client count - %d\n", monotime.tv_sec, monotime.tv_nsec, data->clientsLeft);

                //Go to sleep - no clients
                if (data->clientsLeft == 0){
                    data->curBarberState = SLEEPING;
                    printf("%ld.%ld Barber: go to sleep\n", monotime.tv_sec, monotime.tv_nsec);
                }
                //Wait until client sit on chair - (clientsLeft > 0 )
                else {                    
                    data->curBarberState = WAITING_TO_SIT;
                    printf("%ld.%ld Barber: invite client %d to sit\n", monotime.tv_sec, monotime.tv_nsec, data->clientPIDs[0]);
                }
                break;

            case WAKING_UP:
                //Client is trying to wake up barber
                printf("%ld.%ld Barber: waking up\n", monotime.tv_sec, monotime.tv_nsec);
                data->curBarberState = SHAVING;
                break;

            case SHAVING:
                //Do actual shaving and set FINISHED_SHAVING to inform client about it
                shave(); //Used only to demonstrate time difference between start-end of shaving
                clock_gettime(CLOCK_MONOTONIC, &monotime);   
                printf("%ld.%ld Barber: end shaving client %d\n", monotime.tv_sec, monotime.tv_nsec, data->clientPIDs[0]);
                data->curBarberState = FINISHED_SHAVING;
                break;

            case WAITING_TO_SIT:
            case SLEEPING:
            case FINISHED_SHAVING:
            //States which are waiting for client action
            default:
                break;
        }

        giveSemaphore(semID);
    }
    return 0;
}