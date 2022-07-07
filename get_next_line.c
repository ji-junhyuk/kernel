/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: junji <junji@student.42seoul.kr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/02 14:01:18 by junji             #+#    #+#             */
/*   Updated: 2022/06/02 14:01:37 by junji            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

char	*store_line(int fd, char *buffer)
{
	char	*temp;
	int		read_size;

	temp = malloc(sizeof(char) * (BUFFER_SIZE + 1));
	if (!temp)
		return (0);
	while (1)
	{
		read_size = read(fd, temp, BUFFER_SIZE);
		if (read_size <= 0)
		{
			free(temp);
			return (buffer);
		}
		temp[read_size] = 0;
		buffer = ft_strjoin(buffer, temp);
		if (ft_strchr(buffer, '\n'))
			break ;
	}
	free(temp);
	return (buffer);
}

char	*extract_line(char *buffer)
{
	int		len;
	char	*line;

	len = 0;
	if (!*buffer)
		return (0);
	while (buffer[len] && buffer[len] != '\n')
		++len;
	if (!buffer[len])
		line = ft_strdup(buffer);
	else
		line = ft_substr(buffer, 0, len + 1);
	return (line);
}

char	*store_next_line(char *buffer)
{
	int		len;
	char	*next_line;

	len = 0;
	while (buffer[len] && buffer[len] != '\n')
		++len;
	if (!buffer[len])
	{
		free(buffer);
		return (0);
	}
	next_line = ft_substr(buffer, len + 1, ft_strlen(buffer + len + 1));
	if (!next_line)
		return (0);
	free(buffer);
	return (next_line);
}

char	*get_next_line(int fd)
{
	static char		*buffer;
	char			*line;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (0);
	buffer = store_line(fd, buffer);
	if (!buffer)
		return (0);
	line = extract_line(buffer);
	buffer = store_next_line(buffer);
	return (line);
}
