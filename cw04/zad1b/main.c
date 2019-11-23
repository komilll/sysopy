#define _XOPEN_SOURCE 500
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

int continuePrint = 1;
pid_t savedPID = -1;

void handleConsoleStop(int sigNum);

pid_t createProcess()
{
    pid_t pid = fork();
    if (pid < 0){
        printf("Fatal error, program termination\n");
        exit(-1);
    } else if (pid == 0) {
        while (1)
        {
            system("date");
        }
    } 
    // else {
    //     int returnVal = 0;
    //     wait(&returnVal);  
    //     if (returnVal != 0){            
    //         exit(-1);
    //     }      
    // }
    else
    {
        return pid;
    }
    
    return -1;
}

void registerStpHandler()
{
    struct sigaction action = {0};
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    action.sa_handler = handleConsoleStop;
    sigaction(SIGTSTP, &action, 0);
}

void handleConsoleInterupt(int sigNum)
{
    printf("\nKill main process and child process\n");
    kill(savedPID, SIGKILL);
    exit(0);
}

void handleConsoleStop(int sigNum)
{
    if (savedPID == -1)
    {
        return;
    }
    if (continuePrint)
    {
        printf("\nOczekuje na CTRL+Z - kontynuacja albo CTRL+C - zakonczenie programu\n");
        kill(savedPID, SIGKILL);
        continuePrint = 0;
    }
    else
    {
        savedPID = createProcess();
        continuePrint = 1;
    }
    registerStpHandler();
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTSTP);
    sigprocmask(SIG_UNBLOCK, &mask, 0);
}

int main(int argc, char* argv[])
{
    registerStpHandler();
    savedPID = createProcess();
    if (savedPID != -1)
    {
        signal(SIGINT, handleConsoleInterupt);

        while (1)
        {
            // sleep(1);
        }
    }

    return 0;
}
