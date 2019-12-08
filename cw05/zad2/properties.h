#ifndef _PROPERTIES_H_
#define _PROPERTIES_H_

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>

#define MAX_CLIENTS 5
#define PROJECT_ID 0x352
#define MAX_CHARACTERS_IN_MESSAGE 256
#define MAX_QUEUE_SIZE 10
//Use define to cast to correct number type
#define MESSAGE_SIZE sizeof(message)

typedef enum mtype{
    TIME = 1, END = 2, INT = 3, START = 4
} mtype;

typedef struct message{
    long mtype;    
    pid_t senderPID;
    char mtext[MAX_CHARACTERS_IN_MESSAGE]; //Has to be at the end
} message;

const char SERVER_NAME[] = "/server";

mqd_t getServerQueueID()
{
    return mq_open(SERVER_NAME, O_WRONLY);
}

#endif