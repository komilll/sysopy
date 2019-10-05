#include <stdio.h>
#include <time.h>

void FirstTest();

int main()
{
    FirstTest();
    return 0;
}

void FirstTest()
{
    /* Save starting clock time */
    clock_t startClock = clock();

    int sum = 0;    
    int i;
    for (i = 0; i < 1000000000; ++i)
    {
        sum += i;
    }

    /* Save ending clock time */
    clock_t endClock = clock();

    /* Get results as seconds */
    double time = ((double) (endClock - startClock)) / CLOCKS_PER_SEC;

    /* Print out time; Remember to print 'sum' variable to avoid optimizing it out */
    printf("Sum: %d \n", sum);
    printf("Time passed: %f \n", time);
}