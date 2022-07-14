#include "get_next_line.h"

#include <fcntl.h>
#include <stdio.h>

char *get_next_line(int fd);

int main() 
{
	char	*line;
	int		idx = 0;
	int fd;

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
    //free(line);
	//close(fd);
    return (0);
}
