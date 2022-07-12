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

	fd = open("junji", O_RDONLY);
	while (1)
	{
		line = get_next_line(fd);
		printf("line: %s", line);
		if (!line)
			break ;
		free(line);
	}
	return (0);
}
