#include <dlfcn.h>
#include <stdio.h>
#include <time.h>

int main()
{
    /* Save starting clock time */
    clock_t startClock = clock();

    /* Try to dynamically open library */
    void *handle = dlopen("./libsaving.so", RTLD_LAZY);
    if (!handle)
    {
        printf("Failed to load library: %s\n", dlerror());
        return -1;
    }
    /* Declare and set function pointer */
    void (*libFunction) ();
    libFunction = (void (*) ())dlsym(handle, "SaveFile");
    if (dlerror() != NULL)
    {
        printf("Library function not found: %s\n", dlerror());
        return -2;
    }

    /* Call function from library */
    (*libFunction)();

    /* Close library */
    dlclose(handle);

    /* Save ending clock time */
    clock_t endClock = clock();

    /* Get results as seconds */
    double time = ((double) (endClock - startClock)) / CLOCKS_PER_SEC;
    
    /* Print out time */
    printf("Time passed: %f \n", time);

    return 0;
}