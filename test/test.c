#include <stdio.h>

#ifndef OPEN_MAX
# define OPEN_MAX 512
#endif

int main(void)
{
	printf("%d\n", OPEN_MAX);
}

