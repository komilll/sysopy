#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void ThirdTest();

int main()
{
    ThirdTest();
    return 0;
}

void ThirdTest()
{
    /* Save starting clock time */
    clock_t startClock = clock();

    int i = 0;
    size_t pointerAdress = 0;
    for (i = 0; i < 1000000; ++i)
    {
        void *a = malloc(50000000 + i);
        pointerAdress += (size_t)(a);        
        free(a);
    }    
    printf("Use variable to avoid optimization: %d \n", (int)pointerAdress);

    /* Save ending clock time */
    clock_t endClock = clock();

    /* Get results as seconds */
    double time = ((double) (endClock - startClock)) / CLOCKS_PER_SEC;

    /* Print out time */
    printf("Time passed: %f \n", time);
}