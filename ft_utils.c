#include "interpreter.h"

int	ft_isalpha(int c)
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		return (1);
	else
		return (0);
}
int	ft_isdigit(int c)
{
	return (c >= '0' && c <= '9');
}
int ft_isspace(char c) 
{
	if (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r')
		return 1;
	return 0;
}
void	*ft_memset(void *b, int c, size_t len)
{
	unsigned char	*p;

	p = (unsigned char *)b;
	while (len--)
		*p++ = (unsigned char)c;
	return (b);
}

