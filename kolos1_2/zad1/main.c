#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>

static void display_times(clock_t, struct tms*, struct tms*);

static void do_cmd(char* , int instruction, int final);

int main (int argc, char* argv[]){
    int i;

    setbuf(stdout, NULL);
    for (i = 1; i < argc; i++)
        do_cmd(argv[i], i, argc - 1);

    exit(0);
}

static void do_cmd(char* cmd, int instruction, int final)
{
    struct tms tmsstart, tmsend;
    clock_t start, end;
    int status;

    printf("\ncommand: %s\n", cmd);

    start = times(&tmsstart);
    status = system(cmd);
    if (status < 0){
        printf("Failed to execute command\n");
        exit(-1);
    }
    end = times(&tmsend);

    display_times(end - start, &tmsstart, &tmsend);

    if (instruction == final)
        exit(status);
}

static void display_times(clock_t real, struct tms* tmsstart, struct tms* tmsend)
{
    static long clktck = 0;

    if (clktck == 0)
    {
        if ((clktck = sysconf(_SC_CLK_TCK)) < 0){
            printf("sysconf error");
            exit(-1);
        }
    }

    printf("    real:   %7.2f\n", real / (double)clktck);
}