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

static int ft_check_argc(int argc)
{
    if (!(argc == 9))
    {
        printf("Error: Expected 8 arguments\n");
        return (1);
    }
    return (0);
}

static int ft_check_numbers(char *str, int idx)
{
    int i;

    if (*str == '\0')
    {
        printf("Error: Argument %d is empty\n", idx);
        return (1);
    }

    i = 0;
    while (str[i])
    {
        if (str[i] < '0' || str[i] > '9')
        {
            printf("Error: ('%s') has invalid character '%c'. \n", str, str[i]);
            return (1);
        }
        i++;
    }
    return (0);
}


static int ft_validate_and_save(t_program *pgm, char **argv)
{
    pgm->total_coders = atoi(argv[1]);
    pgm->time_to_burnout = atoi(argv[2]);
    pgm->time_to_compile = atoi(argv[3]);
    pgm->time_to_debug = atoi(argv[4]);
    pgm->time_to_refactor = atoi(argv[5]);
    pgm->num_compiles = atoi(argv[6]);
    pgm->cooldown = atoi(argv[7]);
    if (strcmp(argv[8], "FIFO") == 0)
        pgm->scheduler = 0;
    else
        pgm->scheduler = 1;
    return (0);
}

static int ft_check_scheduler(char *str, int i)
{
    if (strcmp(str, "FIFO") != 0 && strcmp(str, "EDF") != 0)
    {
        printf("Error: Argument %d has invalid scheduler '%s'. Expected 'FIFO' or 'EDF'.\n", i, str);
        return (1);
    }
    return (0);
}

int ft_parsing(int argc, char **argv, t_program *program)
{
    int i;

    if (ft_check_argc(argc))
        return (1);

    i = 1;
    while (i < argc)
    {
        if (i < 8)
        {
            if  (ft_check_numbers(argv[i], i))
                return (1);
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