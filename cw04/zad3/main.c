#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

const int maxProgramCount = 10;
const int maxArgumentCount = 10;

char** readLine(FILE* file, int index, char commandBuffer[200], int maxCount);

void executeLine(FILE* file, int lineCommandCount, char** commands)
{
    //Prepare pipe arrays
    int pipesPrev[2] = {0,0};
    int pipesCurr[2] = {0,0};
    int childPIDs[maxProgramCount];

    for (int i = 0; i < lineCommandCount; ++i)
    {
        if (pipe(pipesCurr) < 0){
            printf("Failed to pipe program!");
            exit(-1);
        }

        //Fork process to call new program
        pid_t pid = fork();
        childPIDs[i] = pid;
        if (pid < 0){
            printf("Fatal error, couldn't create process\n");
            exit(-1);
        } else if (pid == 0){
            //Prepare arguments
            char commandBuffer[200];
            strcpy(commandBuffer, commands[i]);
            char** arguments = readLine(file, i, commandBuffer, lineCommandCount);

            //Not first? Read previous data
            if (i > 0){
                dup2(pipesPrev[0], STDIN_FILENO);
                close(pipesPrev[1]);
                close(pipesPrev[0]);
            }
            //Not last? Write next data
            if (i + 1 < lineCommandCount){
                dup2(pipesCurr[1], STDOUT_FILENO);
                close(pipesCurr[0]);
                close(pipesCurr[1]);
            }

            //Call new program with parsed arguments
            execvp(arguments[0], arguments);
            exit(0);
        } else {
            if (i > 0){
                close(pipesPrev[0]);
                close(pipesPrev[1]);
            }
            if (i + 1 < lineCommandCount){
                pipesPrev[0] = pipesCurr[0];
                pipesPrev[1] = pipesCurr[1];
            }
        }
    }
    
    //Wait until all processes in line are finished
    int retStatus = 0;
    for (int i = 0; i < lineCommandCount; ++i){
        waitpid(childPIDs[i], &retStatus, 0);
    }
}

char** readLine(FILE* file, int index, char commandBuffer[200], int maxCount)
{
    char* buffer[maxArgumentCount];
    buffer[0] = strtok(commandBuffer, " ");
    
    //Continue to read arguments
    int i = 0;
    while (buffer[i] != NULL){
        ++i;
        if (i == maxArgumentCount){
            printf("Too much arguments for program call\n");
            exit(-1);
        }
        buffer[i] = strtok(NULL, " ");
    }
    //Parse text buffer arguments to char**
    char** arguments = (char**) calloc(i + 1, sizeof(char*));
    buffer[i - 1][strlen(buffer[i - 1]) - (maxCount - 1 == index ? 1 : 0)] = '\0';
    for (int j = 0; j < i; ++j){
        arguments[j] = buffer[j];
    }
    arguments[i] = NULL;

    return arguments;
}

void readFile(const char* filename)
{
    //Try to open file
    FILE* file = fopen(filename, "r");
    if (!file){
        printf("Couldn't find file to interpret\n");
        exit(-1);
    }

    //Prepare chars for file data
    char lineBuffer[1000];
    char* commands[maxProgramCount];

    //Read line by line
    int commandCounter = 0;
    while ((fgets(lineBuffer, 1000, file)) != NULL){
        commandCounter = 0;
        //Get all commands from line
        while ((commands[commandCounter] = strtok(commandCounter == 0 ? lineBuffer : NULL, "|")) != NULL){
            commandCounter++;
        }
        //Execute those commands
        executeLine(file, commandCounter, commands);
    }
}

int main(int argc, char* argv[]){
    //The only argument should be file to interpret
    if (argc == 2) readFile(argv[1]);
    return (argc == 2) ? 0 : -1;
}