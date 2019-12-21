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
    FINISHED_SHAVING
};

enum clientState{
    CHECK_BARBER,
    SIT_ON_CHAIR,
    GET_SHAVE,
    LEAVING,
    WAIT_IN_QUEUE
};

struct sharedData{
    int semID;
    int clientsLeft;
    int clientPIDs[MAX_CLIENT_COUNT];
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