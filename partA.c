#include <stdio.h>
#include <math.h>

int isPrime(unsigned int num)
{
    if ((num % 2) == 0)
        return 0;
    for (int i = 3; i < sqrt(num); i += 2)
    {
        if ((num % i) == 0)
            return 0;
    }
    return 1;
}