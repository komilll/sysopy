#define _XOPEN_SOURCE 500
#include "properties.h"
#include <errno.h>
#include <signal.h>

int isServerActive = 1;
mqd_t queueID = -1;
int activeClients = -1;
int clientConnectionIDs[MAX_CLIENTS];
pid_t clientPIDs[MAX_CLIENTS];

mqd_t createServerQueue(struct mq_attr attributes)
{
    return mq_open(SERVER_NAME, O_RDONLY | O_CREAT | O_EXCL, 0666, &attributes);
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
    char clientPath[20];
    sprintf(clientPath, "/%d", msg->senderPID);
    printf("Client path: %s\n", clientPath);
    int id = mq_open(clientPath, O_WRONLY);
    clientConnectionIDs[activeClients] = id;
    clientPIDs[activeClients] = msg->senderPID;
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
    if (mq_send(id, (char*)&newMsg, MESSAGE_SIZE, 0) == -1){
        printf("Failed to send message to client\n");
        printf("errno: %s\n", strerror(errno));
        exit(-1);
    }
}

void processTimeCommand(message *msg)
{
    //Get client key
    char clientPath[20];
    sprintf(clientPath, "/%d", msg->senderPID);
    mqd_t id = mq_open(clientPath, O_WRONLY);

    //Error - client not found
    if (id == -1){
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
    if (mq_send(id, (char*) &newMsg, MESSAGE_SIZE, 0) == -1){
        printf("Client of id: %d has closed queue connection\n", id);
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
    for (int i = 0; i <= activeClients; ++i){
        if (mq_close(clientConnectionIDs[i]) == -1){
            printf("Failed to close client connection: %d\n", i);
        }

        if (kill(clientPIDs[i], SIGINT) == -1){
            printf("Failed to kill client: %d\n", i);
        }
    }
    activeClients = 0;

    if (queueID != -1){
        if (mq_close(queueID) == -1){
            printf("Failed to delete queue at exit in server\n");        
        }

        if (mq_unlink(SERVER_NAME) == -1){
            printf("Failed to unlink queue at exit in server\n");        
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

    mq_unlink(SERVER_NAME);

    //Create public queue to receive messages
    struct mq_attr attributes;
    attributes.mq_maxmsg = MAX_QUEUE_SIZE;
    attributes.mq_msgsize = MESSAGE_SIZE;

    queueID = createServerQueue(attributes);
    if (queueID == -1){
        printf("Failed to create server queue\n");
        printf("errno: %s\n", strerror(errno));
        return -1;
    }

    //Zero initialize structs
    struct mq_attr state;
    struct message msg;
    while (1)
    {
        //If preparing to shutdown - check if there are messages left
        if (!isServerActive)
        {
            if (mq_getattr(queueID, &state) == -1){
                printf("Failed to get queue value in server\n");
                exit(-1);
            }
            //Stop server execution if no active anymore and all cmds processed
            if (state.mq_curmsgs == 0){
                break;
            }
        }
        
        //Process message if any received
        if (mq_receive(queueID, (char*)&msg, MESSAGE_SIZE, NULL) != -1){
            processReceivedMessages(&msg);
        }
    }

    return 0;
}
