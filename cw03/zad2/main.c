#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

void readFile(const char* filename)
{

}

int main(int argc, char* argv[])
{
    //The only argument should be file to interpret
    if (argc != 2)
    {
        return -1;
    }

    readFile(argv[1]);

    return 0;
}