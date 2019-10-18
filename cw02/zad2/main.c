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
        buf[i] = (mode & (1 << (8 - i))) ? chars[i] : '-';
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
}

void getActualPath(const char* dir, char* filename, char* returnPath)
{
    char dirCopy[strlen(dir) + 1];
    memset(returnPath, 0, strlen(returnPath));
    strcpy(dirCopy, dir);
    strcat(returnPath, dirCopy);
    strcat(returnPath, filename);
}

int compareTime(time_t fileTime)
{
    int timeDiff = difftime(modDateComp, fileTime);
    timeDiff -= 3600;
    // printf("Time diff: %d", timeDiff);
    return (modDateType == '<' && timeDiff < 0) || (modDateType == '>' && timeDiff > 0)
        || (modDateType == '=' && timeDiff == 0);
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
        if (compareTime(stats.st_mtime)){
            printFileData(actualPath, &stats);
        }

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

    //Try to print file data
    if (compareTime(stats->st_mtime))
        printFileData(filename, stats);

    return 0;
}

int parseArguments(int argc, char* argv[])
{
    modDateType = argv[2][0];

    if (strcmp(argv[2], "<") || strcmp(argv[2], ">") || strcmp(argv[2], "="))
    {
        const char* date = argv[3];
        if (strlen(date) != 19) {
            printf("Wrong date string length\n");
            return -2;
        }
        const char format[] = "%d.%m.%Y %H:%M:%S";
        struct tm timeStruct;
        strptime(date, format, &timeStruct);
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
    printf("\n");

    return 0;
}