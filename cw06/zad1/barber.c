#include "properties.h"

int semID = -1;
int memID = -1;

void cleanUpAtExit()
{
    if (semID != -1){
        semctl(semID, 0, IPC_RMID);
    }
    if (memID != -1){
        shmctl(memID, IPC_RMID, 0);
    }
}

void initializeBarber()
{
    key_t key = ftok(PROPERTIES_PATH, PROPERTIES_PROGRAM_GENERATOR_ID);

    semID = semget(key, 1, IPC_CREAT | IPC_EXCL);
    if (semID == -1){
        printf("Failed to create semaphore\n");
        exit(-1);
    }
    semctl(semID, 0, SETVAL, 0);

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
    if (signal(SIGINT, cleanUpAtExit) == SIG_ERR){
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

    initializeBarber();

    giveSemaphore(semID);
    data = shmat(memID, NULL, 0);
    if (data == NULL){
        printf("Failed to get data\n");
        return -1;
    }
    data->curBarberState = CHECK_QUEUE;

    struct sysinfo info;
    sysinfo(&info);

    while (1){
        takeSemaphore(semID);

        struct timespec monotime;
        clock_gettime(CLOCK_MONOTONIC, &monotime);   
        
        switch (data->curBarberState){
            case CHECK_QUEUE:
                printf("%ld.%ld Barber: client count - %d\n", monotime.tv_sec, monotime.tv_nsec, data->clientsLeft);

                if (data->clientsLeft == 0){
                    //Go to sleep
                    data->curBarberState = SLEEPING;
                    printf("%ld.%ld Barber: go to sleep\n", monotime.tv_sec, monotime.tv_nsec);
                } else {
                    //Wait until client sit on chair
                    data->curBarberState = WAITING_TO_SIT;
                    printf("%ld.%ld Barber: invite client %d to sit\n", monotime.tv_sec, monotime.tv_nsec, data->clientPIDs[0]);
                }
                break;

            case WAKING_UP:
                printf("%ld.%ld Barber: waking up\n", monotime.tv_sec, monotime.tv_nsec);
                data->curBarberState = SHAVING;
                break;

            case SHAVING:
                printf("%ld.%ld Barber: start shaving client %d\n", monotime.tv_sec, monotime.tv_nsec, data->clientPIDs[0]);
                //Call Shaving() function
                printf("%ld.%ld Barber: end shaving client %d\n", monotime.tv_sec, monotime.tv_nsec, data->clientPIDs[0]);
                data->curBarberState = FINISHED_SHAVING;
                break;

            case WAITING_TO_SIT:
            case SLEEPING:
            case FINISHED_SHAVING:
            default:
                break;
        }

        giveSemaphore(semID);
    }
    return 0;
}