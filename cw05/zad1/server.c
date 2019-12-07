#define _XOPEN_SOURCE 500
#include "properties.h"
#include <errno.h>
#include <signal.h>

int isServerActive = 1;
int queueID = -1;
int activeClients = -1;
int clientConnectionIDs[MAX_CLIENTS];
key_t clientKeys[MAX_CLIENTS];

int createServerQueue()
{
    if (getServerKey() == -1){
        printf("Failed to get key on server creation\n");
        exit(-1);
    }
    return msgget(getServerKey(), IPC_CREAT | IPC_EXCL | 0666);
}

void processReceivedMessages(message *msg)
{
    //No message was passed, ignore
    if (msg == NULL){
        return;
    }

    if (msg->mtype == TIME){
        key_t key;
        int clientID = -1;
        sscanf(msg->mtext, "%d", &key);

        for (int i = 0; i < MAX_CLIENTS; i++){
            if (key == clientKeys[i]){
                clientID = i;
                break;
            }
        }

        if (clientID == -1){
            return;
        }

        message newMsg;
        newMsg.mtype = TIME;
        newMsg.senderPID = getpid();

        time_t rawTime;
        struct tm* timeInfo;
        time(&rawTime);
        timeInfo = localtime(&rawTime);
        sprintf(newMsg.mtext, "%s", asctime(timeInfo));

        if (msgsnd(clientConnectionIDs[clientID], &newMsg, MESSAGE_SIZE, 0) == -1){
            printf("Client of id: %d has closed queue connection\n", clientID);
            return;
        }
    }    
    //End queue execution on END command or interruption command
    else if (msg->mtype == END || msg->mtype == INT){
        printf("Received start message on server - END or INT\n");
        exit(-1);        
    } 
    else if (msg->mtype == START){
        activeClients++;
        
        key_t clientKey;
        sscanf(msg->mtext, "%d", &clientKey);
        int id = msgget(clientKey, 0);
        clientConnectionIDs[activeClients] = id;
        clientKeys[activeClients] = clientKey;

        if (id <= 0){
            printf("Failed to get correct id on start\n");
            printf("errno: %s\n", strerror(errno));
            exit(-1);
        }
        message newMsg;
        newMsg.mtype = START;
        newMsg.senderPID = getpid();
        sprintf(newMsg.mtext, "%d", activeClients);

        if (msgsnd(id, &newMsg, MESSAGE_SIZE, 0) == -1){
            printf("Failed to send message to client\n");
            printf("errno: %s\n", strerror(errno));
            exit(-1);
        }
    } 
    else{
        printf("Received start message on server - Uknown type\n");
        exit(-1);
    }
}

void closeQueueAtExit()
{
    if (queueID != -1)
    {
        if (msgctl(queueID, IPC_RMID, NULL) == -1){
            printf("Failed to delete queue at exit in server\n");        
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
        printf("Failed to create atexit function in server\n");
        return -1;
    }

    if (signal(SIGINT, int_handler) == SIG_ERR){
        printf("Failed to assign interruption function in server\n");
        return -1;
    }

    queueID = createServerQueue();
    if (queueID == -1){
        printf("Failed to create server queue\n");
        printf("errno: %s\n", strerror(errno));
        return -1;
    }

    struct msqid_ds state;
    message msg;
    while (1)
    {
        if (!isServerActive)
        {
            if (msgctl(queueID, IPC_STAT, &state) == -1){
                printf("Failed to get queue value in server\n");
                exit(-1);
            }
            //Stop server execution if no active anymore and all cmds processed
            if (state.msg_qnum == 0){
                break;
            }
        }
        

        if (msgrcv(queueID, &msg, MESSAGE_SIZE, 0, 0) == -1){
            // printf("Failed to receive message on server\n");
            // exit(-1);
        }
        else{
            processReceivedMessages(&msg);
        }
    }

    return 0;
}
