#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

const int maxArgumentCount = 10;

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
    char* buffer[maxArgumentCount];
    
    //Continue to read file line by line
    while ((fgets(lineBuffer, 1000, file)) != NULL){
        //Program name in [0] array position
        buffer[0] = strtok(lineBuffer, " ");
        
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
        buffer[i - 1][strlen(buffer[i - 1]) - 1] = '\0';
        for (int j = 0; j < i; ++j){
            arguments[j] = buffer[j];
        }
        arguments[i] = NULL;

        //Fork process to call new program
        pid_t pid = fork();
        if (pid < 0){
            printf("Fatal error, couldn't create process\n");
            exit(-1);
        } else if (pid == 0){
            //Call new program with parsed arguments
            execvp(arguments[0], arguments);
        } else {
            //Wait in interpreter program for results - terminate if called program failed
            int returnVal = 0;
            wait(&returnVal);
            if (returnVal != 0){
                printf("\n~Failed to execute program: >>>%s<<< in new process. Terminate interpreter\n", arguments[0]);
                exit(-1);
            }
        }
    }
}

int main(int argc, char* argv[]){
    //The only argument should be file to interpret
    if (argc == 2) readFile(argv[1]);
    return (argc == 2) ? 0 : -1;
}