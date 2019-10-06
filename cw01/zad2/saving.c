#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "saving.h"

void SaveFile()
{
    FILE *file;
    /* Base text and number of repeats in text file */
    const int repeatCount = 1000000;
    char baseText[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer sit amet nisi vel enim auctor auctor id ut risus.";    
    size_t outSize = strlen(baseText);

    /* Allocate enough memory for all of text repeats */
    char *outText = malloc(outSize * sizeof(char) * repeatCount);
    int i;
    for (i = 0; i < (int)outSize * repeatCount; ++i)
        outText[i] = baseText[i % outSize];
    
    /* Try to open file for writting */
    if ((file = fopen("lorem_ipsum.txt", "w")) == NULL)
    {
        printf("Failed to open file: lorem_ipsum.txt");
        return;
    }

    /* Save text to file and close file */
    fprintf(file, "%s", outText);
    fclose(file);
}