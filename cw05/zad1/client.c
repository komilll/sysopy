#define _XOPEN_SOURCE 500
#include "properties.h"
#include <signal.h>
#include <errno.h>

int clientConnectionID = -1;
int serverID = -1;
int clientIDOnServer = -1;

void closeQueueAtExit();

void createServerConnection(key_t clientKey)
{
    message msg;
    msg.mtype = START;
    msg.senderPID = getpid();    
    sprintf(msg.mtext, "%d", clientKey);

    if (msgsnd(serverID, &msg, MESSAGE_SIZE, 0) == -1){
        printf("Failed to register client connection to server\n");
        printf("errno: %s\n", strerror(errno));
        exit(-1);
    }

    if (msgrcv(clientConnectionID, &msg, MESSAGE_SIZE, 0, 0) == -1){
        printf("Failed to receive initial message from server\n");
        printf("errno: %s\n", strerror(errno));
        exit(-1);
    }

    sscanf(msg.mtext, "%d", &clientIDOnServer);
    printf("Client ID on server: %d\n", clientIDOnServer);
}

void closeQueueAtExit()
{
    if (clientConnectionID != -1)
    {
        if (msgctl(clientConnectionID, IPC_RMID, NULL) == -1){
            printf("Failed to delete queue at exit in client\n");        
        }
    }
}

void int_handler()
{
    closeQueueAtExit();
    exit(-1);
}

int main(int argc, char* argv[])
{
    if (atexit(closeQueueAtExit) == -1){
        printf("Failed to create atexit function in client\n");
        return -1;
    }

    if (signal(SIGINT, int_handler) == SIG_ERR){
        printf("Failed to assign interruption function in client\n");
        return -1;
    }

    char* path = getenv("HOME");
    if (path == NULL){
        printf("Failed to generate environment path\n");
        return -1;
    }
    //Make sure that key will be unique
    key_t clientKey = ftok(path, getpid());
    if (clientKey == -1){
        printf("Failed to generated ftok key\n");
        return -1;
    }    

    clientConnectionID = msgget(clientKey, IPC_CREAT | IPC_EXCL | 0666);
    if (clientConnectionID == -1){
        printf("Failed to create client queue\n");
        return -1;
    }

    serverID = getServerQueueID();

    createServerConnection(clientKey);

    message msg;
    msg.mtype = TIME;
    msg.senderPID = getpid();    
    sprintf(msg.mtext, "%d", clientKey);

    while (1)
    {
        if (msgsnd(serverID, &msg, MESSAGE_SIZE, 0) == -1){
            printf("Failed to send message to server\n");
            printf("errno: %s\n", strerror(errno));
            exit(-1);
        }

        if (msgrcv(clientConnectionID, &msg, MESSAGE_SIZE, 0, 0) == -1){
            // printf("Failed to receive server response\n");
            // printf("errno: %s\n", strerror(errno));
            // exit(-1);
        } else {
            printf("Message: %s\n", msg.mtext);
        }
    }
    return 0;
}