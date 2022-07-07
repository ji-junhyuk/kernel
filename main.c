#include "get_next_line.h"

#include <fcntl.h>
#include <stdio.h>

char *get_next_line(int fd);

int main() 
{
	char	*line;
	int		fd;
	int		idx = 0;

    fd = open("test1.txt", O_RDONLY);
    while (1)
    {
		printf("get_next_line()\n");
		line = get_next_line(fd);
		if (!line)
			break ;
    	printf("\n>>>main line %d<<< %s", ++idx, line);	
        free(line);
    }
    free(line);
	close(fd);
	system("leaks a.out");
    return (0);
}
