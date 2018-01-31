#include <stdlib.h>
#include <stdio.h>

void main(int argc, char * argv[])
{
int k;

printf("Number of arguments = %d\n",argc);

for (k=0; k<argc; k++) {
    printf("argv[%d] -> %s\n", k, argv[k]);
}
}

