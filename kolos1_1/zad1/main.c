#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char* argv[]){
    DIR* directory;
    if (argc != 2){
        printf("Call %s path\n", argv[0]);
        return 1;
    }

    struct dirent* position;

    directory = opendir(argv[1]);
    if (directory == NULL){
        printf("Error: %d\n", errno);
        return 1;
    }

    errno = 0;
    while ((position = readdir(directory)) != NULL)
    {
        if (strcmp(position->d_name, ".") != 0 && strcmp(position->d_name, "..") != 0){
            printf("%s\n", position->d_name);
        }
    }

    if (errno != 0){
        printf("Failed to execute readdir: %d\n", errno);
        return 2;
    }

    closedir(directory);

    return 0;
}