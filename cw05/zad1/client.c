#define _XOPEN_SOURCE 500
#include "properties.h"
#include <signal.h>
#include <errno.h>

int clientConnectionID = -1;
int serverID = -1;
int clientIDOnServer = -1;
int closeAtNextCommand = 0;
int doNotWaitForResponse = 0;

void closeQueueAtExit();

mtype getMessageType(FILE* file)
{
    //Read next line in file
    char line[1000];
    if ((fgets(line, 1000, file)) != NULL)
    {
        //Zero-terminate when end-of-line found
        int n = strlen(line);
        if (line[n - 1] == '\n')
            line[n-1] = 0;
        
        if (strcmp(line, "TIME") == 0){
            return TIME;
        } else if (strcmp(line, "END") == 0){
            doNotWaitForResponse = 1;
            return END;
        } else {
            printf("Unknown command in file: %s\n", line);
            exit(-1);
        }
    }
    //Read all lines - close after receiving last message
    closeAtNextCommand = 1;
    return START;
}

void createServerConnection(key_t clientKey)
{
    //Create private connection to server and receive ID
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
    //Cleanup at exit
    if (clientConnectionID != -1){
        if (msgctl(clientConnectionID, IPC_RMID, NULL) == -1){
            printf("Failed to delete queue at exit in client\n");        
        }
        clientConnectionID = -1;
    }
}

void int_handler()
{
    //Cleanup at interruption
    closeQueueAtExit();
    exit(-1);
}

int main(int argc, char* argv[])
{
    //Prepare atexit/signal for cleanup
    if (atexit(closeQueueAtExit) == -1){
        printf("Failed to create atexit function in client\n");
        return -1;
    }
    if (signal(SIGINT, int_handler) == SIG_ERR){
        printf("Failed to assign interruption function in client\n");
        return -1;
    }

    //Prepare path for key generation
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

    //Open private client-server connection
    clientConnectionID = msgget(clientKey, IPC_CREAT | IPC_EXCL | 0666);
    if (clientConnectionID == -1){
        printf("Failed to create client queue\n");
        return -1;
    }

    //Get public server queue and register client
    serverID = getServerQueueID();
    createServerConnection(clientKey);

    //Prepare message
    message msg;
    msg.mtype = TIME;
    msg.senderPID = getpid();    
    sprintf(msg.mtext, "%d", clientKey);

    //Open file to read commands
    FILE* file = fopen("messages.txt", "r");

    while (1)
    {
        //Get command, close if reached file end
        msg.mtype = getMessageType(file);
        if (msg.mtype == TIME){
            printf("Message sent: TIME\n");
        } else if (msg.mtype == END){
            printf("Message sent: END\n");
        }
        if (closeAtNextCommand){
            return 0;
        }

        //Send message by reading file - line by line
        if (msgsnd(serverID, &msg, MESSAGE_SIZE, 0) == -1){
            printf("Failed to send message to server\n");
            printf("errno: %s\n", strerror(errno));
            exit(-1);
        }

        if (doNotWaitForResponse){
            return 0;
        }

        //Try to receive response - do action or gently end execution
        if (msgrcv(clientConnectionID, &msg, MESSAGE_SIZE, 0, 0) != -1){
            if (msg.mtype == TIME){
                printf("Message received: %s\n", msg.mtext);
            } else if (msg.mtype == END || msg.mtype == INT){
                return 0;
            }
        }
    }
    return 0;
}