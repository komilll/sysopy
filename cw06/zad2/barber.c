#include "properties.h"

sem_t* semID;
int memID = -1;
int stopLoop = 0;

void shave()
{
    struct timespec monotime;
    clock_gettime(CLOCK_MONOTONIC, &monotime);   
    printf("%ld.%ld Barber: start shaving client %d\n", monotime.tv_sec, monotime.tv_nsec, data->clientPIDs[0]);
}

void term_handler()
{
    stopLoop = 0;
    exit(0);   
}

void cleanUpAtExit()
{
    //Clean up at exit
    if (semID != NULL){
        sem_close(semID);
        sem_unlink(SEMAPHORE_NAME);
        semID = NULL;
    }
    if (memID != -1 && data != NULL){
        munmap(data, sizeof(struct sharedData));
        shm_unlink(MEMORY_NAME);
        memID = -1;
        data = NULL;
    }
}

void initializeBarber()
{
    //Initialize semaphore
    sem_unlink(SEMAPHORE_NAME);
    semID = sem_open(SEMAPHORE_NAME, O_CREAT | O_EXCL, 0666, 0);
    if (semID == SEM_FAILED){
        printf("Failed to create semaphore\n");
        exit(-1);
    }
    sem_post(semID);

    //Initialize shared memory
    shm_unlink(MEMORY_NAME);
    memID = shm_open(MEMORY_NAME, O_RDWR | O_CREAT | O_EXCL, 0666);
    if (memID == -1){
        printf("Failed to create shared memory\n");
        exit(-1);
    }    
    if (ftruncate(memID, sizeof(struct sharedData)) == -1){
        printf("Failed to alocate shared memory: %s\n", strerror(errno));
        exit(-1);
    }

    //Check data and initialize barber state
    data = mmap(NULL, sizeof(struct sharedData), PROT_READ | PROT_WRITE, MAP_SHARED, memID, 0);
    if (data == (void*)-1){
        printf("Failed to get data\n");
        exit(-1);
    }
    data->semID = semID;
    data->clientsLeft = 0;
    data->curBarberState = CHECK_QUEUE;
}

int main(int argc, char* argv[])
{
    //Prepare signals to stop execution
    if (signal(SIGINT, term_handler) == SIG_ERR){
        //--Practial purpose - it's easier to test with Ctrl+C
        printf("Failed to assign SIGINT function in client\n");
        return -1;
    }
    if (signal(SIGTERM, term_handler) == SIG_ERR){
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

    struct timespec monotime;

    while (1){
        if (stopLoop){
            continue;
        }
        takeSemaphore(semID);

        //Get current time
        // clock_gettime(CLOCK_MONOTONIC, &monotime);   
        
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