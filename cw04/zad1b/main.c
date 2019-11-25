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
    //Create child process
    pid_t pid = fork();
    if (pid < 0){
        printf("Fatal error, program termination\n");
        exit(-1);
    } else if (pid == 0) {
        //Continue printing date in console in child
        while (1)
            system("date");
    } else {
        //Return chil process id to use it later
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
    if (savedPID == -1){
        //Child process - ignore signal
        return;
    }

    if (continuePrint)
    {
        //Stopped printing - kill child process
        printf("\nOczekuje na CTRL+Z - kontynuacja albo CTRL+C - zakonczenie programu\n");
        kill(savedPID, SIGKILL);
        continuePrint = 0;
    }
    else
    {
        //Started printing - create child process
        savedPID = createProcess();
        continuePrint = 1;
    }
}

int main(int argc, char* argv[])
{
    //Register handler and create first child
    registerStpHandler();
    savedPID = createProcess();
    if (savedPID != -1)
    {
        //Prepare interupt signal handler for parent process
        signal(SIGINT, handleConsoleInterupt);

        //And wait for signals
        while (1)
        {
            pause();
        }
    }

    return 0;
}
