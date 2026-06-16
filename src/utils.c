




int ft_mem_alloc(t_program *pgm)
{
	pgm->coders = malloc(sizeof(t_coder) * pgm->total_coders);
	if (!pgm->coders)
	{
		printf("Error: Failed to allocate memory in ('coders').\n");
		return (1);
	}
	pgm->dongles = malloc(sizeof(t_dongle) * pgm->total_coders);
	if (!pgm->dongles)
	{
		printf("Error: Failed to allocate memory in ('dongles').\n");
		free(pgm->coders);
		return (1);
	}
	return (0);
}


int ft_strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2 && *s1 == *s2)
    {
        s1++;
        s2++;
    }
    return ((unsigned char)*s1 - (unsigned char)*s2);
}

int	ft_atoi(const char *str)
{
	int	result;
	int	i;
	int	sign;

	i = 0;
	result = 0;
	sign = 1;
	while (str[i] == '\r' || str[i] == '\t' || str[i] == '\n' || str[i] == '\f'
		|| str [i] == ' ' || str[i] == '\v')
		i++;
	if (str[i] == '-')
	{
		sign = -1;
		i++;
	}
	else if (str[i] == '+')
		i++;
	while (str[i] >= '0' && str[i] <= '9')
	{
		result = result * 10 + (str[i] - '0');
		i++;
	}
	return (sign * result);
}