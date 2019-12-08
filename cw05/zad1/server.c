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

void processStartCommand(message *msg)
{
    //Increase client counter
    activeClients++;
    if (activeClients > MAX_CLIENTS){
        printf("Max client count reached! Close server\n");
        exit(-1);
    }

    //Prepare client connection
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

    //Prepare accept connection message
    message newMsg;
    newMsg.mtype = START;
    newMsg.senderPID = getpid();
    sprintf(newMsg.mtext, "%d", activeClients);

    //Send message openning client-server private connection
    if (msgsnd(id, &newMsg, MESSAGE_SIZE, 0) == -1){
        printf("Failed to send message to client\n");
        printf("errno: %s\n", strerror(errno));
        exit(-1);
    }
}

void processTimeCommand(message *msg)
{
    //Get client key
    key_t key;
    sscanf(msg->mtext, "%d", &key);
    int clientID = -1;

    //Find client ID in array
    for (int i = 0; i < MAX_CLIENTS; i++){
        if (key == clientKeys[i]){
            clientID = i;
            break;
        }
    }

    //Error - client not found
    if (clientID == -1){
        printf("Failed to find client who requested command\n");
        exit(-1);
    }

    //Prepare message
    message newMsg;
    newMsg.mtype = TIME;
    newMsg.senderPID = getpid();

    //Prepare time
    time_t rawTime;
    struct tm* timeInfo;
    time(&rawTime);
    timeInfo = localtime(&rawTime);
    sprintf(newMsg.mtext, "%s", asctime(timeInfo));

    //Send message to client who requested time
    if (msgsnd(clientConnectionIDs[clientID], &newMsg, MESSAGE_SIZE, 0) == -1){
        printf("Client of id: %d has closed queue connection\n", clientID);
        return;
    }
}

void processReceivedMessages(message *msg)
{
    //No message was passed, ignore
    if (msg == NULL){
        return;
    }

    if (msg->mtype == TIME){
        processTimeCommand(msg);
    }    
    //End queue execution on END command or interruption command
    else if (msg->mtype == END || msg->mtype == INT){
        printf("Received message on server - END or INT\n");
        isServerActive = 0;        
    } 
    else if (msg->mtype == START){
        processStartCommand(msg);
    } 
    else{
        printf("Received start message on server - Uknown type\n");
        exit(-1);
    }
}

void closeQueueAtExit()
{
    //Cleanup at exit
    if (queueID != -1){
        if (msgctl(queueID, IPC_RMID, NULL) == -1){
            printf("Failed to delete queue at exit in server\n");        
        }
    }
}

void int_handler()
{
    //Cleanup at console interuption
    closeQueueAtExit();
    exit(-1);
}

int main(int argc, char* argv[])
{
    //Prepare atexit/signal for cleanup
    if (atexit(closeQueueAtExit) == -1){
        printf("Failed to create atexit function in server\n");
        return -1;
    }
    if (signal(SIGINT, int_handler) == SIG_ERR){
        printf("Failed to assign interruption function in server\n");
        return -1;
    }

    //Create public queue to receive messages
    queueID = createServerQueue();
    if (queueID == -1){
        printf("Failed to create server queue\n");
        printf("errno: %s\n", strerror(errno));
        return -1;
    }

    //Zero initialize structs
    struct msqid_ds state;
    struct message msg;
    while (1)
    {
        //If preparing to shutdown - check if there are messages left
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
        
        //Process message if any received
        if (msgrcv(queueID, &msg, MESSAGE_SIZE, 0, 0) != -1){
            processReceivedMessages(&msg);
        }
    }

    return 0;
}
