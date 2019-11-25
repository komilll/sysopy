#define _XOPEN_SOURCE 500
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

int signalsReceivedCounter = 0;
int signalsBackToParent = 0;
int parentPID = 0;
int childPID = 0;
int sig1;
int sig2;
int breakLoop = 0;

void printReceivedCounter()
{
    printf("Signals received: %d\n", signalsReceivedCounter);
}

void handle_int(int signum)
{
    //If parent received INT signal - send kill signal to child and stop main loop
    if (getpid() == parentPID)
    {
        kill(childPID, sig2);
        breakLoop = 1;
    }
}

void handle_usr1(int signum)
{
    //If parent - receive back signal
    //If child - send back signal to parent
    if (getpid() == parentPID) {
        signalsBackToParent++;
    }
    else {    
        signalsReceivedCounter++;
        kill(parentPID, sig1);
    }
}

void handle_usr2(int signum)
{
    //End signal - print informations and exit
    printReceivedCounter();
    exit(0);
}

void registerSignals()
{
    struct sigaction actionUSR1;
    actionUSR1.sa_flags = 0;
    sigemptyset(&actionUSR1.sa_mask);
    actionUSR1.sa_handler = handle_usr1;
    sigaction(sig1, &actionUSR1, NULL);

    struct sigaction actionINT;
    actionINT.sa_flags = 0;
    sigemptyset(&actionINT.sa_mask);
    actionINT.sa_handler = handle_int;
    sigaction(SIGINT, &actionINT, NULL);   

    struct sigaction actionUSR2;
    actionUSR2.sa_flags = 0;
    sigemptyset(&actionUSR2.sa_mask);
    actionUSR2.sa_handler = handle_usr2;
    sigaction(sig2, &actionUSR2, NULL);   
}

int main(int argc, char* argv[])
{
    if (argc != 3){
        printf("Wrong number of arguments!");
        return -1;
    }

    //Prepare parameters
    long L = strtol(argv[1], NULL, 10);
    long type = strtol(argv[2], NULL, 10);

    if (type > 3 || type < 1){
        printf("Wrong type argument");
        return -1;
    }

    //Prepare signal based on type (1/2 vs 3)
    sig1 = type == 3 ? SIGRTMIN+1 : SIGUSR1;
    sig2 = type == 3 ? SIGRTMIN+2 : SIGUSR2;
    registerSignals();

    //Create child process - save parent/child PIDs
    parentPID = getpid();
    childPID = fork();
    if (childPID < 0){
        printf("Failed to fork()");
        exit(-1);
    }
    else if (childPID == 0)
    {
        //Spinlock on child - just wait for signals
        while(1) pause();
    }
    else
    {
        //Continue sending signals to child
        int sendSignals = 0;
        for (int i = 0; i < L; i++)
        {
            if (breakLoop)
            {
                //Break loop - only entered when interrupted through console
                break;
            }
            if (type == 2)
            {
                //Type 2 ensures that all signals will be received correctly
                while (signalsBackToParent != sendSignals)
                    wait(0);
            }
            kill(childPID, sig1);
            sendSignals++;
        }
        //Print data and send ending signal
        printf("Signals sent: %d\n", sendSignals);
        kill(childPID, sig2);
        wait(0);
        printf("Signals received in parent: %d\n", signalsBackToParent);
    }

    return 0;
}