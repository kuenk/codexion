#include "../include/codexion.h"
#include <stdio.h>
#include <pthread.h>

int main(int argc, char **argv)
{
    if (ft_parsing(argc, argv, &program))
        return (1);
    if (ft_mem_alloc(&program))
        return (1);
    ft_init(&program);
    ft_start_simulation(&program);
    return (0);
}