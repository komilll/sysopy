#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

void generate(const char* path, int count, int size)
{
    //Try to open file at path
    FILE* file = fopen(path, "w");
    if (!file) {
        printf("Failed to open file at given path");
        exit(-1);
    }

    //Try to open /dev/random to access random numbers
    FILE* randomFile = fopen("/dev/urandom", "r");
    if (!randomFile) {
        printf("Failed to open /dev/urandom file");
        exit(-1);
    }

    //Initialize buffer with needed size
    void* buffer = malloc(size * count);

    //Insert random data into file
    if (fread(buffer, count, size, randomFile) <= 0){
        printf("Failed to read from random file");
        exit(-1);
    }

    if (fwrite(buffer, count, size, file) <= 0){
        printf("Failed to write to new file");
        exit(-1);
    }

    //Clean-up files and buffer
    free(buffer);
    fclose(file);
    fclose(randomFile);
}


void sort_sys(const char* fileName, int count, int size)
{
    //Try to open file
    int file = open(fileName, O_RDWR);
    if (file < 0) {
        printf("Error, couldn't open file");
        return;
    }
    //Create buffers to hold single array element
    unsigned char* bufferCurrent = malloc(size);
    unsigned char* bufferOther = malloc(size);
    for (int i = 1; i < count; i++)
    {
        //Set file position to i-th position
        if (lseek(file, i * size, SEEK_SET) < 0){
            printf("Failed to seek given bit pointer");
            return;
        }

        //Read at i-th position and store 1st bit value
        if (read(file, bufferCurrent, 1) < 0){
            printf("Failed to read buffer");
            return;
        }
        unsigned char currentVal = bufferCurrent[0];
        int minPos = i;
        //Iterate previous element to find element to swap
        for (int k = i - 1; k >= 0; k--)
        {
            //Set file position to k-th position
            if (lseek(file, k * size, SEEK_SET) < 0){
                printf("Failed to seek given bit pointer");
                return;
            }
            //Read at k-th position and store 1st bit value
            if (read(file, bufferOther, 1) < 0){
                printf("Failed to read buffer");
                return;
            }
            unsigned char otherVal = bufferOther[0];

            //Searching for element to swap
            if (otherVal > currentVal)
            {
                unsigned char* bufferTmp = malloc(size);
                *bufferTmp = *bufferOther;

                //At k-th position store i-th pos value
                if (lseek(file, k * size, SEEK_SET) < 0) {
                    printf("Failed to seek given bit pointer");
                    return;
                }
                if (write(file, bufferCurrent, size) < 0){
                    printf("Failed to write at given position");
                    return;
                }
                //Set to i-th position and store k-th pos value
                if (lseek(file, minPos * size, SEEK_SET) < 0){
                    printf("Failed to seek given bit pointer");
                    return;
                }
                if (write(file, bufferTmp, size) < 0){
                    printf("Failed to write at given position");
                    return;
                }

                minPos = k;
                //Set file position to i-th position
                if (lseek(file, minPos * size, SEEK_SET) < 0){
                    printf("Failed to seek given bit pointer");
                    return;
                }
                //Read at i-th position and store 1st bit value
                if (read(file, bufferCurrent, 1) < 0){
                    printf("Failed to read buffer");
                    return;
                }
            }
        }
    }

    if (close(file) < 0){
        printf("Failed to close file correctly");
        return;
    }
}

void sort_lib(const char* fileName, int count, int size)
{
    //Open file to read/write
    FILE* file = fopen(fileName, "r+");
    if (!file){
        printf("Error, couldn't open file");
        return;
    }
    //Create buffers to hold single array element
    unsigned char* bufferCurrent = malloc(size);
    unsigned char* bufferOther = malloc(size);
    for (int i = 1; i < count; i++)
    {
        //Set file position to i-th position
        if (fseek(file, i * size, 0)){
            printf("Failed to seek given bit pointer");
            exit(-1);
        }

        //Read at i-th position and store 1st bit value
        if (fread(bufferCurrent, size, 1, file) <= 0){
            printf("Failed to read buffer #i: %d", i);
            exit(-1);
        }
        unsigned char currentVal = bufferCurrent[0];
        int minPos = i;
        //Iterate previous element to find one to swap with
        for (int k = i - 1; k >= 0; k--)
        {
            //Set file position to k-th position
            if (fseek(file, k * size, 0)){
                printf("Failed to seek given bit pointer");
                exit(-1);
            }
            //Read at k-th position and store 1st bit value
            if (fread(bufferOther, size, 1, file) <= 0){
                printf("Failed to read buffer #k: %d", k);
                exit(-1);
            }
            unsigned char otherVal = bufferOther[0];

            //Searching for element to swap
            if (otherVal > currentVal)
            {
                unsigned char* bufferTmp = malloc(size);
                *bufferTmp = *bufferOther;

                //At k-th position store i-th pos value
                if (fseek(file, k * size, SEEK_SET)){
                    printf("Failed to seek given bit pointer");
                    exit(-1);
                }
                if (fwrite(bufferCurrent, size, 1, file) <= 0){
                    printf("Failed to write at given position");
                    exit(-1);
                }
                //Set to i-th position and store k-th pos value
                if (fseek(file, minPos * size, SEEK_SET)){
                    printf("Failed to seek given bit pointer");
                    exit(-1);
                }
                if (fwrite(bufferTmp, size, 1, file) <= 0){
                    printf("Failed to write at given position");
                    exit(-1);
                }

                minPos = k;
                //Set file position to i-th position
                if (fseek(file, minPos * size, SEEK_SET)){
                    printf("Failed to seek given bit pointer");
                    exit(-1);
                }
                //Read at i-th position and store 1st bit value
                if (fread(bufferCurrent, size, 1, file) < 0){
                    printf("Failed to read buffer");
                    exit(-1);
                }
            }
        }
    }
    fclose(file);
}

void copy_sys(const char* inFileName, const char* outFileName, int count, int size)
{
    //Open src and dst files
    int fileIn = open(inFileName, O_RDONLY);
    int fileOut = open(outFileName, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if (fileIn < 0){
        printf("Couldn't open file in\n");
        exit(-1);
    }
    if (fileOut < 0){
        printf("Couldn't open file out\n");
        exit(-1);
    }
    //Create buffer to hold element to copy
    unsigned char* copyBuffer = malloc(size);
    for (int i = 0; i < count; i++)
    {
        //Set file position to i-th position
        if (lseek(fileIn, i * size, SEEK_SET) < 0){
            printf("Failed to seek given bit pointer");
            exit(-1);
        }
        if (read(fileIn, copyBuffer, size) < 0){
            printf("Failed to read file buffer");
            exit(-1);
        }
        if (write(fileOut, copyBuffer, size) < 0){
            printf("Failed to write to file");
            exit(-1);
        }
    }    
    //Cleanup
    free(copyBuffer);
    close(fileIn);
    close(fileOut);
}

void copy_lib(const char* inFileName, const char* outFileName, int count, int size)
{
    //Open src/dst files
    FILE* fileIn = fopen(inFileName, "r");
    FILE* fileOut = fopen(outFileName, "w");
    if (!fileIn){
        printf("Couldn't open file in\n");
        exit(-1);
    }
    if (!fileOut){
        printf("Couldn't open file out\n");
        exit(-1);
    }
    //Create buffer to hold single element
    unsigned char* copyBuffer = malloc(size);
    for (int i = 0; i < count; i++)
    {
        //Set file position to i-th position
        if (fseek(fileIn, i * size, SEEK_SET)){
            printf("Failed to seek given bit pointer");
            exit(-1);
        }
        if (fread(copyBuffer, size, 1, fileIn) <= 0){
            printf("Failed to read file buffer");
            exit(-1);
        }
        if (fwrite(copyBuffer, size, 1, fileOut) <= 0){
            printf("Failed to write to file");
            exit(-1);
        }
    }    
    //Cleanup
    free(copyBuffer);
    fclose(fileIn);
    fclose(fileOut);
}

int main(int argc, char* argv[])
{
    if (!(argc >= 5 && argc <= 6))
    {
        return -1;
    }
    
    if (argc == 5)
    {
        const char* txt = ".txt";
        const char* copyTxt = "Copy.txt";
        char* fileName = (char*)malloc(1 + strlen(argv[2]) + strlen(txt));
        char* copyFilename = (char*)malloc(1 + strlen(argv[2]) + strlen(copyTxt));

        const int count = atoi(argv[3]);        
        const int size = atoi(argv[4]);

        strcat(fileName, argv[2]);
        strcat(fileName, txt);
        strcat(copyFilename, argv[2]);
        strcat(copyFilename, copyTxt);

        if (strcmp(argv[1], "generate") == 0)
        {
            generate(fileName, count, size);
            copy_lib(fileName, copyFilename, count, size);
            return 0;
        }
        else if (strcmp(argv[1], "sort_sys") == 0)
        {
            clock_t startClock = clock();
            sort_sys(fileName, count, size);
            clock_t endClock = clock();

            double time = ((double) (endClock - startClock) / CLOCKS_PER_SEC);
            printf("sort_sys size(%d) record(%d) time: %f\n", size, count, time);

            FILE* resultFile = fopen("sort.txt", "a");
            fprintf(resultFile, "sort_sys size(%d) record(%d) time: %f\n", size, count, time);
            fclose(resultFile);
            return 0;
        }
        else if (strcmp(argv[1], "sort_lib") == 0)
        {
            clock_t startClock = clock();
            sort_lib(fileName, count, size);
            clock_t endClock = clock();

            double time = ((double) (endClock - startClock) / CLOCKS_PER_SEC);
            printf("sort_lib size(%d) record(%d) time: %f\n", size, count, time);

            FILE* resultFile = fopen("sort.txt", "a");
            fprintf(resultFile, "sort_lib size(%d) record(%d) time: %f\n", size, count, time);
            fclose(resultFile);
            return 0;
        }
    }
    else 
    {
        const char* txt = ".txt";
        char* fileName = (char*)malloc(1 + strlen(argv[2]) + strlen(txt));
        char* copyFilename = (char*)malloc(1 + strlen(argv[3]) + strlen(txt));
        const int count = atoi(argv[4]);        
        const int size = atoi(argv[5]);

        strcat(fileName, argv[2]);
        strcat(fileName, txt);
        strcat(copyFilename, argv[3]);
        strcat(copyFilename, txt);

        if (strcmp(argv[1], "copy_sys") == 0)
        {
            clock_t startClock = clock();
            copy_lib(fileName, copyFilename, count, size);
            clock_t endClock = clock();
            
            double time = ((double) (endClock - startClock) / CLOCKS_PER_SEC);
            printf("copy_sys size(%d) record(%d) time: %f\n", size, count, time);

            FILE* resultFile = fopen("copy.txt", "a");
            fprintf(resultFile, "copy_sys size(%d) record(%d) time: %f\n", size, count, time);
            fclose(resultFile);
            return 0;
        }
        else if (strcmp(argv[1], "copy_lib") == 0)
        {
            clock_t startClock = clock();
            copy_lib(fileName, copyFilename, count, size);
            clock_t endClock = clock();
            
            double time = ((double) (endClock - startClock) / CLOCKS_PER_SEC);
            printf("copy_lib size(%d) record(%d) time: %f\n", size, count, time);

            FILE* resultFile = fopen("copy.txt", "a");
            fprintf(resultFile, "copy_lib size(%d) record(%d) time: %f\n", size, count, time);
            fclose(resultFile);
            return 0;
        }

    }

    return 0;
}