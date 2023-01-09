#include <stdio.h>

void a()
{
    printf("OK\n");
}

int b()
{
    return 40;
}

int c(int x, int y)
{
    return x + y;
}

int *alloc(int x, int y)
{
    static int arr[2];
    arr[0] = x;
    arr[1] = y;
    return arr;
}
