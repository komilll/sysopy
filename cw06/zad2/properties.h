#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

// #include <sys/sem.h>
// #include <sys/types.h>
// #include <sys/ipc.h>
// #include <sys/shm.h>

#define MAX_CLIENT_COUNT 64

const char* SEMAPHORE_NAME = "/barberSem";
const char* MEMORY_NAME = "/barberMem";

enum barberState{
    CHECK_QUEUE,
    SLEEPING,
    WAITING_TO_SIT,
    SHAVING,
    FINISHED_SHAVING,
    WAKING_UP
};

enum clientState{
    SIT_ON_CHAIR,
    WAIT_IN_QUEUE,
    IDLE
};

struct sharedData{
    sem_t* semID;
    int clientsLeft;
    pid_t clientPIDs[MAX_CLIENT_COUNT];
    enum barberState curBarberState;
} *data;

void takeSemaphore(sem_t* semaphoreID)
{
    if (semaphoreID == NULL){
        return;
    }
    sem_post(semaphoreID);
}

void giveSemaphore(sem_t* semaphoreID)
{
    if (semaphoreID == NULL){
        return;
    }
    sem_wait(semaphoreID);
}

void leaveQueue(int memID)
{
    data = mmap(NULL, sizeof(struct sharedData), PROT_READ | PROT_WRITE, MAP_SHARED, memID, 0);
    if (data == NULL){
        return;
    }
    data->clientsLeft--;

    for (int i = 0; i < data->clientsLeft; ++i){
        data->clientPIDs[i] = data->clientPIDs[i+1];
    }
}

void goAtQueueEnd(int memID)
{
    data = mmap(NULL, sizeof(struct sharedData), PROT_READ | PROT_WRITE, MAP_SHARED, memID, 0);
    if (data == NULL){
        return;
    }
    if (data->clientsLeft <= 1){
        return;
    }

    pid_t savedPID = data->clientPIDs[0];
    for (int i = 0; i < data->clientsLeft - 1; ++i){
        data->clientPIDs[i] = data->clientPIDs[i+1];
    }
    data->clientPIDs[data->clientsLeft - 1] = savedPID;
}

void moveToQueueBeginning(int memID, pid_t clientPID)
{
    data = mmap(NULL, sizeof(struct sharedData), PROT_READ | PROT_WRITE, MAP_SHARED, memID, 0);
    if (data == NULL){
        return;
    }
    if (data->clientsLeft <= 1){
        return;
    }
    
    int clientIndex = 0;
    for (int i = 0; i < data->clientsLeft; ++i){
        if (clientPID == data->clientPIDs[i]){
            clientIndex = i;
            break;
        }
    }

    for (int i = clientIndex; i > 0; --i){
        data->clientPIDs[i] = data->clientPIDs[i-1];
    }
    data->clientPIDs[0] = clientPID;
}