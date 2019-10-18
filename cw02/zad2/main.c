#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <linux/limits.h>
#include <limits.h>

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

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

void printDirData(const char* dirName, const char* args, time_t timeComp)
{
    DIR* dir = opendir(dirName);
    if (!dir)
    {
        printf("Directory couldn't be openned\n");
        exit(-1);
    }

    struct dirent* curFile = readdir(dir);
    struct stat curFileStat;
    while (curFile)
    {
        // if (strcmp(curFile->d_type, DT_REG) == 0)
        {
            if (lstat(curFile->d_name, &curFileStat) < 0)
            {
                curFile = readdir(dir);
                continue;
            }
            if (S_ISDIR(curFileStat.st_mode))
            {
                printf("Dir: %s\n", curFile->d_name);
                curFile = readdir(dir);
                continue;
            }
            if (S_ISLNK(curFileStat.st_mode))
            {
                printf("Link: %s\n", curFile->d_name);
                curFile = readdir(dir);
                continue;
            }
            if (!S_ISREG(curFileStat.st_mode))
            {
                curFile = readdir(dir);
                continue;
            }

            char date[36];
            char accessRights[10];
            if (strcmp(args, "<") == 0)
            {
                if (!(curFileStat.st_mtime < timeComp))
                {
                    curFile = readdir(dir);
                    continue;
                }
            }
            else if (strcmp(args, ">") == 0)
            {
                if (!(curFileStat.st_mtime > timeComp))
                {
                    curFile = readdir(dir);
                    continue;
                }
            }
            else if (strcmp(args, "=") == 0)
            {
                if (!(curFileStat.st_mtime == timeComp))
                {
                    curFile = readdir(dir);
                    continue;
                }
            }
            printf("%s\n  last modification: %s", curFile->d_name, formatDate(date, curFileStat.st_mtime));
            printf("\n  size in bytes: %ld", curFileStat.st_size);
            printf("\n  rights: %s", strmode(curFileStat.st_mode, accessRights));
            char actualPath[PATH_MAX + 1]; 
            char* pathPtr = realpath(curFile->d_name, actualPath);
            if (!pathPtr)
            {
                printf("Failed to find real path: %d\n", errno);
                exit(-1);
            }
            printf("\n  full path: %s\n", actualPath);
        }
        curFile = readdir(dir);
    }

    if (closedir(dir) < 0)
    {
        printf("Directory didn't closed properly\n");
        exit(-1);
    }
}

int main(int argc, char* argv[])
{
    printDirData("./", "<", 0);

    if (argc != 4)
    {
        return -1;
    }

    if (strcmp(argv[2], "<") || strcmp(argv[2], ">") || strcmp(argv[2], "="))
    {
        struct tm timeStruct;
        char dayStr[] = {argv[3][0], argv[3][1]};
        timeStruct.tm_mday = atoi(dayStr);
        char monthStr[] = {argv[3][3], argv[3][4]};
        timeStruct.tm_mon = atoi(monthStr) - 1;
        char yearStr[] = {argv[3][6], argv[3][7], argv[3][8], argv[3][9]};
        timeStruct.tm_year = atoi(yearStr);

        time_t time = mktime(&timeStruct);
        printDirData(argv[1], "<", time);
    }
    else
    {
        printf("Second argument is wrong. Type '<', '>' or '='\n");
        return -2;
    }

    return 0;
}