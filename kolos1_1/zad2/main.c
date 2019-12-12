#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main (int argc, char* argv[]){
    if (argc != 2){
        printf("WRONG NUMBER OF ARGUMENTS");
        return -1;
    }

    pid_t child;
    int status, retval;
    if ((child = fork()) < 0){
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (child == 0){
        sleep(100);
        exit(EXIT_SUCCESS);
    } else {
        int seconds = atoi(argv[1]);
        sleep(seconds);
        waitpid(child, &status, WNOHANG); //MOST IMPORTANT HERE

        kill(child, SIGKILL);

        if (waitpid(child, &status, 0) != -1){
            retval = WEXITSTATUS(status);
            printf("PID: %d, status: %d, return val: %d\n", getpid(), status, retval);
        } else {
            perror("WAITPID");
            strerror(errno);
        }
    }

    exit(EXIT_SUCCESS);
}