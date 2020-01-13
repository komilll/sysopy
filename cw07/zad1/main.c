#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

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
pthread_mutex_t* b_mutex;
pthread_mutex_t* p_mutex;
pthread_mutex_t* c_mutex;
pthread_cond_t w_cond;
pthread_cond_t r_cond;
int p_index = 0;
int c_index = 0;
int isWorking = 1;

//////////////////////

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
            // printf("Line #%d: %d\n", i, line[i]);
            return 0;
        }
    }
    return 1;
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

    //Prepare mutexes
    b_mutex = malloc(N * sizeof(pthread_mutex_t));
    for (int i = 0; i < N; ++i)
        pthread_mutex_init(&b_mutex[i], NULL);
    c_mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(c_mutex, NULL);
    p_mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(p_mutex, NULL);
        
    //Prepare variable conditions
    pthread_cond_init(&w_cond, NULL);
    pthread_cond_init(&r_cond, NULL);

    //Intialize threads
    p_threads = malloc((size_t) P * sizeof(pthread_t));
    c_threads = malloc((size_t) K * sizeof(pthread_t));
}

void* producerFunc()
{
    int currentIndex = 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, mainFile)){
        pthread_mutex_lock(p_mutex);

        while (buffer[p_index] != NULL)
            pthread_cond_wait(&w_cond, p_mutex);

        currentIndex = p_index;
        pthread_mutex_lock(&b_mutex[currentIndex]);

        buffer[currentIndex] = malloc((strlen(line) + 1) * sizeof(char));
        strcpy(buffer[currentIndex], line);
        p_index = (p_index + 1) % N;        
        
        pthread_mutex_unlock(&b_mutex[currentIndex]);
        pthread_cond_broadcast(&r_cond);
        pthread_mutex_unlock(p_mutex);
    }

    return NULL;
}

void* consumerFunc()
{
    int currentIndex = 0;
    char *line;
    while (1){  
        pthread_mutex_lock(c_mutex);

        while(buffer[c_index] == NULL){
            if (!isWorking){
                pthread_mutex_unlock(c_mutex);
                return NULL;
            }
            pthread_cond_wait(&r_cond, c_mutex);
        }

        currentIndex = c_index;
        pthread_mutex_lock(&b_mutex[currentIndex]);
        line = buffer[currentIndex];
        if (!isEmptyLine(line) && strlen(line) < L){
            printf("%s\n", line);
        }
        buffer[currentIndex] = NULL;
        free(line);
        c_index = (c_index + 1) % N;
        pthread_mutex_unlock(&b_mutex[currentIndex]);
        pthread_cond_broadcast(&w_cond);
        pthread_mutex_unlock(c_mutex);
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
}

void joinThreads()
{
    for (int i = 0; i < P; ++i){
        pthread_join(p_threads[i], NULL);
    }
    isWorking = 0;
    pthread_cond_broadcast(&r_cond);
    for (int i = 0; i < K; ++i){
        pthread_join(c_threads[i], NULL);
    }
}

void cleanup()
{
    return;
    if (mainFile){
        fclose(mainFile);
    }

    for (int i = 0; i < N; ++i){
        if (buffer[i]){
            free(buffer[i]);
        }
        free(buffer);
    }

    for (int i = 0; i < N; ++i){
        pthread_mutex_destroy(&b_mutex[i]);
    }
    free(b_mutex);
    pthread_mutex_destroy(c_mutex);
    // free(c_mutex);
    pthread_mutex_destroy(p_mutex);
    // free(p_mutex);

    pthread_cond_destroy(&r_cond);
    pthread_cond_destroy(&w_cond);
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