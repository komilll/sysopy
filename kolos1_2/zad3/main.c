#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

int main(int argc, char* argv[]) {
    if (argc != 2){
        printf("Not a suitable number of program parameters\n");
        exit(1);
    }

    int toChildFD[2];
    int toParentFD[2];

    pipe(toChildFD);
    pipe(toParentFD);

    int val1 = 0;
    int val2 = 0;
    int val3 = 0;
    pid_t pid;

    if ((pid = fork()) == 0){
        read(toChildFD[0], &val2, sizeof(val2));

        val2 = val2 * val2;

        write(toParentFD[1], &val2, sizeof(val2));

    } else{
        val1 = atoi(argv[1]);

        write(toChildFD[1], &val1, sizeof(val1));

        sleep(1);
        
        read(toParentFD[0], &val3, sizeof(val3));

        printf("%d square is: %d\n", val1, val3);
    }
}