#include "get_next_line.h"

char *read_line(int fd, char *buffer)
{
	char	*temp;
	int		read_size;

	temp = malloc(sizeof(char) * (BUFFER_SIZE + 1));
	if (!temp)
		return (0);
	read_size = read(fd, temp, BUFFER_SIZE);
	temp[read_size] = '\0';
	while (read_size > 0)
	{
		if (!buffer)
			buffer = ft_strdup(temp);
		else
			buffer = ft_strjoin(buffer, temp);
		// strjoin이 버퍼가 할당되지 않았을 경우
		if (ft_strchr(temp, '\n'))
			break ;
		read_size = read(fd, temp, BUFFER_SIZE);
	}
	free(temp);
	return (buffer);
}

char *extract_line(char *buffer)
{
	int len;
	char *line;

	len = 0;
	while (*buffer != '\n' && *buffer)
	{
		++len; 
		++buffer;
	}
	// len == 0일 때 (바로 개행1개일때 널이 아니라 개행이랑 널문자가 들어가야하는데
	if (*buffer == '\n')
		line = ft_substr(buffer - len, 0, len + 1);
	else
		line = ft_strdup(buffer - len);
	return (line);
}

char *point_next_line(char *buffer)
{
	char 	*next_line;
	char	*init_pos;
	int		len;

	init_pos = buffer;
	while (*buffer != '\n' && *buffer)
		++buffer;
	len = buffer - init_pos;
	if (!(*buffer))
	{
		free(buffer - len);
		return (0);
	}
	next_line = ft_substr(buffer, len + 1, ft_strlen(buffer + len + 1));
	if (!next_line)
	{
		free(buffer);
		return (0);
	}
	free(buffer);
	return (next_line);
}

char	*get_next_line(int fd)
{
	static char *buffer;
	char *line;

	if (fd < 0 && BUFFER_SIZE <= 0)
		return (0);
	buffer = read_line(fd, buffer);
	if (!buffer)
		return (0);
	line = extract_line(buffer);
	buffer = point_next_line(buffer);
	return (line);
}
