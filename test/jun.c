#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

char*	increase(char *buffer)
{
	char *new;
	int len = strlen(buffer);
	new = malloc(sizeof(char) * (len + 5));
	strcpy(new, "hello jun");
	return (new);
}

void	increase2(int* num)
{
    (*num)++;
}
void	increase3(int num)
{
    num++;
}

static int num = 21;
int main(void)
{
	static char *buffer;
	buffer = malloc(sizeof(char) * 5);
	strcpy(buffer, "hell");

	printf("%s\n", buffer);
	buffer = increase(buffer);
	printf("%s\n", buffer);

	printf("%d\n", num);
	increase2(&num);
	printf("%d\n", num);

	printf("%d\n", num);
	increase3(num);
	printf("%d\n", num);
	return (0);

	char *copy;

	copy = malloc(sizeof(char) * 5);
	strcpy(copy, "hell");
	copy++;
	free(copy);
}
