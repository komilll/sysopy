#include "saving.h"
#include <time.h>
#include <stdio.h>

int main()
{
    /* Save ending clock time */
    clock_t startClock = clock();

    /* Call function from library */
    SaveFile();

    /* Save ending clock time */
    clock_t endClock = clock();

    /* Get results as seconds */
    double time = ((double) (endClock - startClock)) / CLOCKS_PER_SEC;
    
    /* Print out time */
    printf("Time passed: %f \n", time);

    return 0;
}