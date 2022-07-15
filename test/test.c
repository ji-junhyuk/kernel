#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

char *get_next_line(int fd);

int main(void)
{
	int fd;
	char	*line;

	fd = open("junji2", O_RDONLY);
//	fd = 0;
	while (1)
	{
		line = get_next_line(fd);
		printf("line: %s", line);
		if (!line)
			break ;
		free(line);
	}
//	system("leaks a.out > leaks_result; cat leaks_result | grep leaked && rm -rf leaks_result");
	return (0);
}

