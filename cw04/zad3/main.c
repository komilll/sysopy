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
    int pipes[lineCommandCount][2];
    for (int i = 0; i < lineCommandCount; ++i){
        if (pipe(pipes[i]) < 0){
            printf("Failed to pipe program!");
            exit(-1);
        }
    }

    for (int i = 0; i < lineCommandCount; ++i)
    {
        printf("\n");
        //Fork process to call new program
        pid_t pid = fork();
        if (pid < 0){
            printf("Fatal error, couldn't create process\n");
            exit(-1);
        } else if (pid == 0){
            char commandBuffer[200];
            strcpy(commandBuffer, commands[i]);
            char** arguments = readLine(file, i, commandBuffer, lineCommandCount);

            //Not first? Read previous data
            if (i > 0){
                close(pipes[i - 1][1]);
                if (dup2(pipes[i - 1][0], STDIN_FILENO) < 0){
                    exit(-1);
                }
            }
            //Not last? Write next data
            if (i + 1 < lineCommandCount){
                close(pipes[i][0]);
                if (dup2(pipes[i][1], STDOUT_FILENO) < 0){
                    exit(-1);
                }
            }

            //Call new program with parsed arguments
            execvp(arguments[0], arguments);
            exit(0);
        }
        
    }

    for (int i = 0; i < lineCommandCount - 1; ++i) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // for (int i = 0; i < lineCommandCount; ++i) {
        wait(0);
    // }
}

char** readLine(FILE* file, int index, char commandBuffer[200], int maxCount)
{
    char* buffer[maxArgumentCount];

    // strcpy(commandBuffer, commands[index]);
    //Program name in [0] array position
    // printf("BBB: %s\n", commandBuffer);
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

    printf("\n #%d -> Try to execute: >%s< with argument: %s : %s\n", index, buffer[0], buffer[1], buffer[2]);
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

    int commandCounter = 0;
    while ((fgets(lineBuffer, 1000, file)) != NULL){
        commandCounter = 0;
        while ((commands[commandCounter] = strtok(commandCounter == 0 ? lineBuffer : NULL, "|")) != NULL){
            commandCounter++;
            // printf("AAA: %s\n", commands[commandCounter-1]);
        }
        executeLine(file, commandCounter, commands);
    }
}

int main(int argc, char* argv[]){
    //The only argument should be file to interpret
    if (argc == 2) readFile(argv[1]);
    return (argc == 2) ? 0 : -1;
}