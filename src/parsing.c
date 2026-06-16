

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

static int ft_check_numbers(char *str, int i)
{
    if (*str == '\0')
    {
        printf("Error: Argument %d is empty\n", i);
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


static int ft_validate_and_save(t_program *pgm, char **argv, int argc)
{
    pgm->total_coders = ft_atoi(argv[1]);
    pgm->time_to_burnout = ft_atoi(argv[2]);
    pgm->time_to_compile = ft_atoi(argv[3]);
    pgm->time_to_debug = ft_atoi(argv[4]);
    pgm->time_to_refactor = ft_atoi(argv[5]);
    pgm->num_compiles = ft_atoi(argv[6]);
    pgm->cooldown = ft_atoi(argv[7]);
    if (ft_strcmp(argv[8], "FIFO") == 0)
        pgm->scheduler = 0;
    else
        pgm->scheduler = 1;
    return (0);
}

static int ft_check_scheduler(char *str, int i)
{
    if (ft_strcmp(str, "FIFO") && ft_strcmp(str, "EDF"))
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
    if (ft_validate_and_save(program, argv, argc))
        return (1);
    return (0);
}