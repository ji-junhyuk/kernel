/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: junji <junji@student.42seoul.kr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/02 14:05:09 by junji             #+#    #+#             */
/*   Updated: 2022/06/02 14:05:10 by junji            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

char	*ft_strjoin(char *s1, char *s2)
{
	char	*copy;
	char	*init_pos;
	size_t	size;

	if (!s1)
	{
		s1 = malloc(sizeof(char) * 1);
		s1[0] = 0;
	}
	if (!s1 || !s2)
		return (0);
	init_pos = s1;
	size = ft_strlen(s1) + ft_strlen(s2);
	copy = malloc(sizeof(char) * (size + 1));
	if (!copy)
		return (0);
	while (*s1)
		*copy++ = *s1++;
	while (*s2)
		*copy++ = *s2++;
	*copy = 0;
	copy -= size;
	free(init_pos);
	return (copy);
}

char	*ft_strchr(const char *s, int c)
{
	int	idx;

	idx = 0;
	while (s[idx])
	{
		if (s[idx] == c)
			return ((char *)&s[idx]);
		++idx;
	}
	if (s[idx] == c)
		return ((char *)&s[idx]);
	return (0);
}

char	*ft_substr(char const *s, unsigned int start, size_t len)
{
	char	*copy;
	size_t	idx;

	if (!s)
		return (0);
	copy = malloc(sizeof(char) * (len + 1));
	if (!copy)
		return (0);
	if (ft_strlen(s) <= start)
	{
		copy[0] = '\0';
		return (copy);
	}
	idx = -1;
	while (++idx < len)
		copy[idx] = s[start++];
	copy[idx] = '\0';
	return (copy);
}

size_t	ft_strlen(const char *s)
{
	size_t	len;

	len = 0;
	while (s[len])
		++len;
	return (len);
}

char	*ft_strdup(const char *src)
{
	size_t	src_len;
	char	*copy;

	src_len = ft_strlen(src);
	copy = malloc(sizeof(char) * (src_len + 1));
	if (!copy)
		return (0);
	while (*src)
		*copy++ = *src++;
	*copy = '\0';
	copy -= src_len;
	return (copy);
}
