#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/sysinfo.h>
#include <time.h>

#define MAX_CLIENT_COUNT 64

const char* PROPERTIES_PATH = "BarberProgram";
const int PROPERTIES_PROGRAM_GENERATOR_ID = 13634;

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
    int semID;
    int clientsLeft;
    pid_t clientPIDs[MAX_CLIENT_COUNT];
    enum barberState curBarberState;
} *data;

void takeSemaphore(int semaphoreID)
{
    struct sembuf buf;
    buf.sem_flg = 0;
    buf.sem_num = 0;
    buf.sem_op = -1;
    
    semop(semaphoreID, &buf, 1);
}

void giveSemaphore(int semaphoreID)
{
    struct sembuf buf;
    buf.sem_flg = 0;
    buf.sem_num = 0;
    buf.sem_op = 1;
    
    semop(semaphoreID, &buf, 1);
}

void leaveQueue(int memID)
{
    data = shmat(memID, NULL, 0);
    data->clientsLeft--;

    for (int i = 0; i < data->clientsLeft; ++i){
        data->clientPIDs[i] = data->clientPIDs[i+1];
    }
}

void goAtQueueEnd(int memID)
{
    data = shmat(memID, NULL, 0);
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
    data = shmat(memID, NULL, 0);
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