/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_bonus.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: junji <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/15 15:15:52 by junji             #+#    #+#             */
/*   Updated: 2022/07/15 16:09:14 by junji            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line_bonus.h"

char	*read_line(int fd, char *buffer)
{
	char	temp[BUFFER_SIZE + 1];
	int		read_size;

	temp[0] = 0;
	while (!ft_strchr(temp, '\n'))
	{
		read_size = read(fd, temp, BUFFER_SIZE);
		temp[read_size] = '\0';
		if (read_size == 0)
			break ;
		if (read_size == -1)
		{
			if (buffer)
				free(buffer);
			return (0);
		}
		if (!buffer)
			buffer = ft_strdup(temp);
		else
			buffer = ft_strjoin(buffer, temp);
	}
	return (buffer);
}

char	*extract_line(char *buffer)
{
	int		len;
	char	*line;

	len = 0;
	while (*buffer != '\n' && *buffer)
	{
		++len;
		++buffer;
	}
	if (*buffer == '\n')
		line = ft_substr(buffer - len, 0, len + 1);
	else
		line = ft_strdup(buffer - len);
	return (line);
}

char	*change_next_line(char *buffer)
{
	char	*next_line;
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
	buffer -= len;
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
	static char		*buffer[OPEN_MAX];
	char			*line;

	if (fd < 0 && BUFFER_SIZE <= 0 && fd > OPEN_MAX)
		return (0);
	buffer[fd] = read_line(fd, buffer[fd]);
	if (!buffer[fd])
		return (0);
	line = extract_line(buffer[fd]);
	buffer[fd] = change_next_line(buffer[fd]);
	return (line);
}
