#ifndef _PROPERTIES_H_
#define _PROPERTIES_H_

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>

#define MAX_CLIENTS 5
#define PROJECT_ID 0x352
#define MAX_CHARACTERS_IN_MESSAGE 256

typedef enum mtype{
    TIME = 1, END = 2, INT = 3, START = 4
} mtype;

typedef struct message{
    long mtype;    
    pid_t senderPID;
    char mtext[MAX_CHARACTERS_IN_MESSAGE]; //Has to be at the end
} message;

const size_t MESSAGE_SIZE = sizeof(message) - sizeof(long);

key_t getServerKey()
{
    char* path = getenv("HOME");
    if (path == NULL){
        printf("Failed to generate environment path\n");
        return -1;
    }
    key_t key = ftok(path, PROJECT_ID);
    if (key == -1){
        printf("Failed to generated ftok key");
        return -1;
    }
    return key;
}

int getServerQueueID()
{
    return msgget(getServerKey(), 0);
}

#endif