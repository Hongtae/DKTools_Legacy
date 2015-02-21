#include <stdio.h>
#include <time.h>
#include <stdlib.h>

static int randInit = 1;

int rand_r(unsigned int *seedp)
{
	if (randInit)
	{
		srandom(*seedp);
		randInit = 0;
	}
	return random();
}
