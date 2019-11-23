#define _XOPEN_SOURCE 500
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int continuePrint = 1;

void handleConsoleInterupt(int sigNum)
{
    printf("\nOdebrano sygnal SIGINT\n");
    exit(0);
}

void handleConsoleStop(int sigNum)
{
    if (continuePrint)
    {
        printf("\nOczekuje na CTRL+Z - kontynuacja albo CTRL+C - zakonczenie programu\n");
        continuePrint = 0;
    }
    else
    {
        continuePrint = 1;
    }
}

int main(int argc, char* argv[])
{
    time_t rawTime;
    struct tm *timeInfo;

    struct sigaction action;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    action.sa_handler = handleConsoleStop;

    signal(SIGINT, handleConsoleInterupt);
    sigaction(SIGTSTP, &action, NULL);

    while (1)
    {
        if (continuePrint)
        {
            time(&rawTime);
            timeInfo = localtime(&rawTime);
            printf("Current time: %s", asctime(timeInfo));
        }
        else
        {
            pause();
        }
        
    }
    return 0;
}
