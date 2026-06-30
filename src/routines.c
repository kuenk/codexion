/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routines.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dcuenca <dcuenca@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/19 14:44:39 by dcuenca           #+#    #+#             */
/*   Updated: 2026/06/19 14:44:39 by dcuenca          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../include/codexion.h"

static int ft_check_end(t_program *pgm)
{
    int end;

    pthread_mutex_lock(&pgm->status_mutex);
    end = pgm->simulation_end;
    pthread_mutex_unlock(&pgm->status_mutex);
    return (end);
}

void ft_compile(t_coder *coder)
{
    pthread_mutex_t *first;
    pthread_mutex_t *second;

    first = &coder->left_dongle->mutex;
    second = &coder->right_dongle->mutex;

    if (coder->id % 2 == 0)
    {
        first = &coder->right_dongle->mutex;
        second = &coder->left_dongle->mutex;
    }

    pthread_mutex_lock(first);
    ft_print(coder, "has taken a dongle");
    pthread_mutex_lock(second);
    ft_print(coder, "has taken the other dongle");
    pthread_mutex_lock(&coder->global->status_mutex);
    coder->last_compile_start = ft_get_time();
    pthread_mutex_unlock(&coder->global->status_mutex);
    ft_print(coder, "is compiling");
    ft_usleep(coder->global->time_to_compile);
    coder->compile_count++;
    pthread_mutex_unlock(first);
    pthread_mutex_unlock(second);
}   

void ft_debug(t_coder *coder)
{
    ft_print(coder, "is debugging");
    ft_usleep(coder->global->time_to_debug);
}

void ft_refactor(t_coder *coder)
{   
    ft_print(coder, "is refactoring");
    ft_usleep(coder->global->time_to_refactor);
}

void *ft_coder_routine(void *arg)
{
    t_coder *coder; 
    coder = (t_coder *)arg;
    t_program *pgm;
    pgm = coder->global;
    if (pgm->total_coders == 1)
    {
        pthread_mutex_lock(&coder->left_dongle->mutex);
        ft_usleep(pgm->time_to_burnout);
        pthread_mutex_unlock(&coder->left_dongle->mutex);
        return (NULL);
    }
    if (coder->id % 2 == 0)
        ft_usleep(2);
    while (!ft_check_end(pgm) 
            && (pgm->num_compiles == -1
            || coder->compile_count < pgm->num_compiles))
    {
        ft_compile(coder);
        ft_debug(coder);
        ft_refactor(coder);
    }
    return NULL;
}
