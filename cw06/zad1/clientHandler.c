#include "properties.h"

int desiredActions = 0;
int performedActions = 0;
pid_t currentChildPID = 0;
int memID = -1;
enum clientState state;

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
        
    } else{
        data = shmat(memID, NULL, 0);
        data->clientPIDs[0] = currentChildPID;
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

    createClient(1);
    data = shmat(memID, NULL, 0);

    struct sysinfo info;
    sysinfo(&info);

    while(1){
        if (currentChildPID == 0){
            takeSemaphore(data->semID);

            struct timespec monotime;
            clock_gettime(CLOCK_MONOTONIC, &monotime);            

            switch (data->curBarberState){
                case WAITING_TO_SIT:
                    state = SIT_ON_CHAIR;
                    printf("%ld Client: sit on chair\n", monotime.tv_nsec);
                    break;
                case FINISHED_SHAVING:
                    printf("%ld Client: leave barber\n", monotime.tv_nsec);
                    data->curBarberState = CHECK_QUEUE;
                    return 0;
                case SLEEPING:
                    state = SIT_ON_CHAIR;
                    data->curBarberState = SHAVING;
                    printf("%ld Client: sit on chair\n", monotime.tv_nsec);
                    break;
                case SHAVING:
                case CHECK_QUEUE:
                default:
                    break;
            }
            giveSemaphore(data->semID);
        }
    }

    return 0;
}