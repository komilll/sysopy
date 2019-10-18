#define _XOPEN_SOURCE 500
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <linux/limits.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <ftw.h>

char modDateType;
time_t modDateComp;

char* formatDate(char* dateStr, time_t timeVal)
{
    strftime(dateStr, 36, "%d.%m.%Y %H:%M:%S", localtime(&timeVal));
    return dateStr;
}

char* strmode(mode_t mode, char* buf)
{
    const char chars[] = "rwxrwxrwx";
    for (size_t i = 0; i < 9; i++)
    {
        buf[i] = (mode & (1 << (8 - i))) ? chars[i] : '-';
    }
    buf[9] = '\0';
    return buf;
}

void printFileData(const char* filename, const struct stat* stats)
{
    char date[36];
    char accessRights[10];
    printf("%s\n  last modification: %s", filename, formatDate(date, stats->st_mtime));
    printf("\n  size in bytes: %ld", stats->st_size);
    printf("\n  rights: %s\n", strmode(stats->st_mode, accessRights));
    // char actualPath[PATH_MAX + 1]; 
    // char* pathPtr = realpath(filename, actualPath);
    // if (!pathPtr)
    // {
    //     printf("Failed to find real path: %d\n", errno);
    //     exit(-1);
    // }
    // printf("\n  full path: %s\n", actualPath);
}

void getActualPath(const char* dir, char* filename, char* returnPath)
{
    char dirCopy[strlen(dir) + 1];
    memset(returnPath, 0, strlen(returnPath));
    strcpy(dirCopy, dir);
    strcat(returnPath, dirCopy);
    strcat(returnPath, filename);
}

void printDirDataStat(const char* dirName)
{
    DIR* dir = opendir(dirName);
    if (!dir){
        printf("Directory couldn't be openned\n");
        exit(-1);
    }

    struct dirent* curFile = readdir(dir);
    struct stat stats;
    char actualPath[PATH_MAX + 1]; 
    while (curFile)
    {
        getActualPath(dirName, curFile->d_name, actualPath);

        //Trying to get stats (including link files)
        if (lstat(actualPath, &stats) < 0){
            printf("Failed to read file: %s\n", curFile->d_name);
            curFile = readdir(dir);
            continue;
        }
        //Ignore dirs and link files
        if (S_ISDIR(stats.st_mode) || S_ISLNK(stats.st_mode) || !S_ISREG(stats.st_mode)){
            curFile = readdir(dir);
            continue;
        }

        //Try to print file data
        if (strcmp(&modDateType, "<") == 0 && stats.st_mtime < modDateComp)
            printFileData(actualPath, &stats);
        else if (strcmp(&modDateType, ">") == 0 && stats.st_mtime > modDateComp)
            printFileData(actualPath, &stats);
        else if (strcmp(&modDateType, "=") == 0 && stats.st_mtime == modDateComp)
            printFileData(actualPath, &stats);

        curFile = readdir(dir);
    }

    if (closedir(dir) < 0){
        printf("Directory didn't closed properly\n");
        exit(-1);
    }
}

int printDirDataNftw(const char* filename, const struct stat* stats, int fileFlags, struct FTW* ftwPtr)
{
    if (S_ISDIR(stats->st_mode) || S_ISLNK(stats->st_mode) || !S_ISREG(stats->st_mode))
        return 0;

    if (strcmp(&modDateType, ">") == 0 && stats->st_mtime < modDateComp)
        printFileData(filename, stats);
    else if (strcmp(&modDateType, "<") == 0 && stats->st_mtime > modDateComp)
        printFileData(filename, stats);
    else if (strcmp(&modDateType, "=") == 0 && stats->st_mtime == modDateComp)
        printFileData(filename, stats);

    return 0;
}

int parseArguments(int argc, char* argv[])
{
    modDateType = argv[2][0];

    if (strcmp(argv[2], "<") || strcmp(argv[2], ">") || strcmp(argv[2], "="))
    {
        const char* date = argv[3];
        if (strlen(date) != 19 && strlen(date) != 10)
        {
            printf("Wrong date string length\n");
            return -2;
        }
        if (strlen(date) == 19){
            if (date[2] != '.' || date[5] != '.' || date[10] != ' ' || date[13] != ':' || date[16] != ':')
            {
                printf("Wrong params\n");
                return -2;
            }
        }
        if (strlen(date) == 10){
            if (date[2] != '.' || date[5] != '.')
            {
                printf("Wrong params\n");
                return -2;
            }
        }
        struct tm timeStruct;
        char dayStr[] = {date[0], date[1]};
        timeStruct.tm_mday = atoi(dayStr);

        char monthStr[] = {date[3], date[4]};
        timeStruct.tm_mon = atoi(monthStr) - 1;

        char yearStr[] = {date[6], date[7], date[8], date[9]};
        timeStruct.tm_year = atoi(yearStr);

        if (strlen(date) == 19)
        {
            char hourStr[] = {date[11], date[12]};
            timeStruct.tm_hour = atoi(hourStr);

            char minuteStr[] = {date[14], date[15]};
            timeStruct.tm_min = atoi(minuteStr);

            char secondStr[] = {date[17], date[18]};
            timeStruct.tm_sec = atoi(secondStr);
        }
        modDateComp = mktime(&timeStruct);
    }
    else
    {
        printf("Second argument is wrong. Type '<', '>' or '='\n");
        return -2;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != 5 || parseArguments(argc, argv) < 0)
    {
        return -1;
    }
    if (strcmp(argv[4], "nftw") == 0){
        nftw(argv[1], printDirDataNftw, 5, FTW_PHYS);
    } else if (strcmp(argv[4], "dir") == 0){
        printDirDataStat(argv[1]);
    }

    return 0;
}