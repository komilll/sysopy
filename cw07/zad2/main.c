#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <sys/sem.h>
#include <semaphore.h>

#define MAX_CONFIG_FILENAME_LENGTH 255
#define MAX_LINE_LENGTH 1000

//////////////////////

FILE *config;
int P; //Producer count
int K; //Consumer count
int N; //Ring-buffer size
char textFilename[MAX_CONFIG_FILENAME_LENGTH]; //Main text to read from
int L; //Number to compare to line length
//0 - read text is shorter
//1 - read text is longer
//2 - read text is equal
int searchType;
int debug; //1 - additional info; other val - no info
int nk; //0 - read till EOF; >0 read for _nk_ miliseconds

//////////////////////

FILE* mainFile;
pthread_t* c_threads;
pthread_t* p_threads;
char** buffer;
sem_t* b_sem; //Ring-buffer semaphore
sem_t* c_sem; //Consumer semaphore
sem_t* p_sem; //Producer semaphore
sem_t* counter_sem; //Counter semaphore - control number of buffer slots available
int p_index = 0;
int c_index = 0;
int isWorking = 1;

//////////////////////

void sigHandler(int signo)
{
    if (signo != SIGINT && signo != SIGALRM){
        return;
    }
    for (int i = 0; i < P; ++i){
        pthread_cancel(p_threads[i]);
    }
    for (int i = 0; i < K; ++i){
        pthread_cancel(c_threads[i]);
    }
    printf("Correctly interrupted program execution\n");
    exit(0);
}

//Check if passed line has any chars (apart from EOF and '\0\)
int isEmptyLine(char* line)
{
    if (strlen(line) == 0){
        return 1;
    }
    for (int i = 0; i < MAX_LINE_LENGTH; ++i){
        if (line[i] == '\0' || line[i] == 10){
            return 1;
        }
        if (line[i] != ' '){
            return 0;
        }
    }
    return 1;
}

int compareLength(char* line)
{
    if (searchType == 0){
        return (strlen(line) < L);
    } else if (searchType == 1){
        return (strlen(line) < L);
    } else if (searchType == 2){
        return (strlen(line) == L);
    }
    printf("Unknown search type passed - terminate program\n");
    exit(-1);
}

void readConfigurationFile(char* filename)
{
    if ((config = fopen(filename, "r")) == NULL){
        printf("Failed to open configuration file\n");
        exit(-1);
    }
    fscanf(config, "%d %d %d %s %d %d %d %d", &P, &K, &N, textFilename, &L, &searchType, &debug, &nk);
    if (debug){
        printf("Config: \n\tProducer count: %d \n\tConsumer count: %d \n\tRing-buffer size: %d \n\tFile to read: %s \n\t", P, K, N, textFilename);
        printf("Line length compare: %d \n\tSearch type: %d \n\tDebug: %d \n\tMax time in ms: %d\n", L, searchType, debug, nk);
    }
    fclose(config);
}

void initialize()
{
    //Open main file
    if ((mainFile = fopen(textFilename, "r")) == NULL){
        printf("Failed to open main text file\n");
        exit(-1);
    }
    //Zero-initialize buffer
    buffer = calloc((size_t) N, sizeof(char*));

    //Prepare semaphores
    b_sem = malloc(N * sizeof(sem_t));
    for (int i = 0; i < N; ++i){
        sem_init(&b_sem[i], 0, 1);
    }
    c_sem = malloc(sizeof(sem_t));
    sem_init(c_sem, 0, 1);
    p_sem = malloc(sizeof(sem_t));
    sem_init(p_sem, 0, 1);
    counter_sem = malloc(sizeof(sem_t));
    sem_init(counter_sem, 0, N);

    //Intialize threads
    p_threads = malloc((size_t) P * sizeof(pthread_t));
    c_threads = malloc((size_t) K * sizeof(pthread_t));

    //Prepare signal
    if (signal(SIGINT, sigHandler) == SIG_ERR){
        printf("Failed to assign SIGINT function in client\n");
        exit(-1);
    }
    //Close after specified time
    if (nk > 0){
        if (signal(SIGALRM, sigHandler) == SIG_ERR){
            printf("Failed to assign SIGINT function in client\n");
            exit(-1);
        }
    }
}

void* producerFunc()
{
    //Prepare index and char array
    int currentIndex = 0;
    char line[MAX_LINE_LENGTH];
    struct timespec monotime;

    //Continue execution if any line in the file wasn't read
    while (fgets(line, MAX_LINE_LENGTH, mainFile)){
        if (debug) printf("Producer #%ld gets line\n", pthread_self());

        //Lock p_semaphore at beginning
        sem_wait(p_sem);

        //Wait to free ring buffer at producer index
        // while (buffer[p_index] != NULL)
        //     pthread_cond_wait(&w_cond, p_mutex);
        sem_wait(counter_sem);

        //Lock buffer index
        currentIndex = p_index;
        sem_wait(&b_sem[currentIndex]);

        clock_gettime(CLOCK_MONOTONIC, &monotime);  
        if (debug) printf("Producer #%ld takes buffer index: %d - time: %ld.%ld\n", pthread_self(), p_index, monotime.tv_sec, monotime.tv_nsec);

        //Pass data to buffer
        buffer[currentIndex] = malloc((strlen(line) + 1) * sizeof(char));
        strcpy(buffer[currentIndex], line);
        p_index = (p_index + 1) % N;        

        if (debug) printf("Producer #%ld saved line to buffer index: %d\n", pthread_self(), p_index);
        
        //Unlock buffer; Unlock producer
        sem_post(&b_sem[currentIndex]);        
        sem_post(p_sem);
        if (debug) printf("Producer #%ld unlocks mutexes and broadcast to cond to consumers\n", pthread_self());
    }
    if (debug) printf("Producer #%ld finished his work - terminate\n", pthread_self());
    return NULL;
}

void* consumerFunc()
{
    //Prepare index and line string pointer
    int currentIndex = 0;
    char *line;
    struct timespec monotime;

    while (1){  
        //Lock c_semaphore at beginning
        sem_wait(c_sem);
        if (debug) printf("~~Consumer #%ld try to take c_mutex\n", pthread_self());

        //Wait to fill ring buffer at consumer index
        while(buffer[c_index] == NULL){
            //WARNING: sem_post/sem_wait is used here because O3 opitimization ignores this spinlock; O0 works fine without this
            sem_post(c_sem);
            //If there is signal to stop - unlock mutex and finish thread
            if (!isWorking){
                if (debug) printf("~~Consumer #%ld finished work\n", pthread_self());
                return NULL;
            }
            sem_wait(c_sem);
        }        

        //Lock buffer index
        currentIndex = c_index;
        sem_wait(&b_sem[currentIndex]);        
        
        clock_gettime(CLOCK_MONOTONIC, &monotime);
        if (debug) printf("~~Consumer #%ld found not-empty buffer index: %d - time: %ld.%ld\n", pthread_self(), c_index, monotime.tv_sec, monotime.tv_nsec);

        //Read line, compare, cleanup
        line = buffer[currentIndex];
        if (!isEmptyLine(line) && compareLength(line)){
            if (debug) printf("~~Consumer #%ld found correct line\n", pthread_self());
            printf("%s\n", line);
        }
        buffer[currentIndex] = NULL;
        free(line);
        c_index = (c_index + 1) % N;        

        //Unlock buffer; Broadcast info to writers; Unlock consumer
        sem_post(&b_sem[currentIndex]);
        sem_post(counter_sem);
        sem_post(c_sem);
        if (debug) printf("~~Consumer #%ld unlocks its mutexes and broadcast to writers\n", pthread_self());
        usleep(10);
    }

    return NULL;
}

void runThreads()
{
    for (int i = 0; i < P; ++i){
        pthread_create(&p_threads[i], NULL, producerFunc, NULL);
    }
    for (int i = 0; i < K; ++i){
        pthread_create(&c_threads[i], NULL, consumerFunc, NULL);
    }
    //Stop execution after specified time
    if (nk > 0){
        ualarm(nk, 0);
        // alarm(nk);
    }
}

void joinThreads()
{
    for (int i = 0; i < P; ++i){
        pthread_join(p_threads[i], NULL);
    }
    //When producers finished working 
    //- indicate that consumers after finding empty buffer should just close themselves
    isWorking = 0;
    for (int i = 0; i < K; ++i){
        pthread_join(c_threads[i], NULL);
    }
}

void cleanup()
{
    if (mainFile){
        fclose(mainFile);
    }

    for (int i = 0; i < N; ++i){
        if (buffer[i]){
            free(buffer[i]);
        }
    }

    for (int i = 0; i < N; ++i){
        sem_destroy(&b_sem[i]);
    }
    free(b_sem);
    sem_destroy(c_sem);
    free(c_sem);
    sem_destroy(p_sem);
    free(p_sem);
    sem_destroy(counter_sem);
    free(counter_sem);
}

int main(int argc, char* argv[])
{
    if (argc != 2){
        printf("No configuration file specified in makefile\n");
        return -1;
    }
    readConfigurationFile(argv[1]);

    initialize();

    runThreads();

    joinThreads();

    cleanup();

    return 0;
}