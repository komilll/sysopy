#include <stdio.h>
#include <math.h>
#include <time.h>

void FourthTest();

int main()
{
    FourthTest();
    return 0;
}

/* Sieve of Erathosthenes algorithm */
void FourthTest()
{
    /* Save starting clock time */
    clock_t startClock = clock();

    int repeatCounter;
    for (repeatCounter = 0; repeatCounter < 10000; ++repeatCounter)
    {
        /* Precalculate square root of range and alloc array */
        const int range = 1000000000;
        const int sqrtRange = (int)sqrt(range);
        char results[sqrtRange];    
        results[0] = 0;
        results[1] = 0;

        /* Fill array with '1' - mark as possibly primary number */
        int i;
        for (i = 2; i < sqrtRange; ++i)
        {
            results[i] = 1;
        }

        /* Find numbers which are not primary */
        int innerIt;
        for (i = 2; i < sqrtRange; ++i)
        {
            if (results[i] == 0)        
                continue;

            for (innerIt = 2 * i; innerIt < sqrtRange; innerIt += i)
                results[innerIt] = 0;        
        }

        /* Disabled due to long time of execution - writting to terminal */
        /* Print only primary numbers */            
        /*
            printf("\n \n");
            for (i = 2; i < sqrtRange; ++i)
            {
                if (results[i] == 1)
                    printf("%d, ", i);
            }
            printf("\n");
        */
    }
    /* Save ending clock time */
    clock_t endClock = clock();

    /* Get results as seconds */
    double time = ((double) (endClock - startClock)) / CLOCKS_PER_SEC;

    /* Print out time */
    printf("Time passed: %f \n", time);
}