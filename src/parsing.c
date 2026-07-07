/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dcuenca <dcuenca@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/23 12:03:38 by dcuenca           #+#    #+#             */
/*   Updated: 2026/06/23 12:03:38 by dcuenca          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/codexion.h"

static int	ft_check_argc(int argc)
{
	if (!(argc == 9))
	{
		printf("Error: Expected 8 arguments\n");
		return (1);
	}
	return (0);
}

static int	ft_parse_uint(const char *str, long long *out)
{
	long long	val;
	int			i;

	if (str == NULL || str[0] == '\0')
		return (1);
	if (str[0] == '-')
		return (1);
	val = 0;
	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (1);
		if (val > (LLONG_MAX - (str[i] - '0')) / 10)
			return (1);
		val = val * 10 + (str[i] - '0');
		i++;
	}
	*out = val;
	return (0);
}

static int	ft_validate_and_save(t_program *pgm, char **argv)
{
	long long	v;

	ft_parse_uint(argv[1], &v);
	if (v < 2 || v >= 200)
	{
		printf("Need between 2 and 200 coders.\n");
		return (1);
	}
	pgm->total_coders = (int)v;
	ft_parse_uint(argv[2], &pgm->time_to_burnout);
	ft_parse_uint(argv[3], &pgm->time_to_compile);
	ft_parse_uint(argv[4], &pgm->time_to_debug);
	ft_parse_uint(argv[5], &pgm->time_to_refactor);
	ft_parse_uint(argv[6], &v);
	pgm->num_compiles = (int)v;
	ft_parse_uint(argv[7], &v);
	pgm->cooldown = (int)v;
	if (strcmp(argv[8], "FIFO") == 0)
		pgm->scheduler = 0;
	else
		pgm->scheduler = 1;
	return (0);
}

static int	ft_check_scheduler(char *str, int i)
{
	if (strcmp(str, "FIFO") != 0 && strcmp(str, "EDF") != 0)
	{
		printf("Error: Argument %d has invalid scheduler '%s'. "
			"Expected 'FIFO' or 'EDF'.\n", i, str);
		return (1);
	}
	return (0);
}

int	ft_parsing(int argc, char **argv, t_program *program)
{
	int			i;
	long long	dummy;

	if (ft_check_argc(argc))
		return (1);
	i = 1;
	while (i < argc)
	{
		if (i < 8)
		{
			if (ft_parse_uint(argv[i], &dummy))
			{
				printf("Error: Argument %d ('%s') is not a valid number.\n",
					i, argv[i]);
				return (1);
			}
		}
		else
		{
			if (ft_check_scheduler(argv[i], i))
				return (1);
		}
		i++;
	}
	if (ft_validate_and_save(program, argv))
		return (1);
	return (0);
}
