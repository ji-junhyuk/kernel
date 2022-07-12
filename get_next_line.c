#include <unistd.h>
#include <string.h>
#include <stdlib.h>

char	*ft_strchr(const char *s, int c)
{
	while (*s)
	{
		if (*s == (char)c)
			return ((char *)s);
		s++;
	}
	if (*s == (char)c)
		return ((char *)s);
	return (0);
}

char *read_line(int fd, char *buffer)
{
	int read_size;

	while (1)
	{
		read_size = read(fd, buffer, BUFFER_SIZE);
		if (read_size <= 0)
		{
			break ;
		}
		if (ft_strchr(buffer, '\n'))
			break ;
	}
	return (buffer);
}

char *extract_line(char *buffer)
{

}

char	*get_next_line(int fd)
{
	static char *buffer;
	char *word;

	if (fd < 0 && BUFFER_SIZE <= 0)
		return (0);
	read_line(fd, buffer);
	word = extract_line(buffer);

	return ("hello");
}
