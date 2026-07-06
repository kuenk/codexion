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
static void ft_wait_for_dongle(t_coder *coder, t_dongle *d)
{
	long long	now;

	while (1)
    {
        pthread_mutex_lock(&d->mutex);

        now = ft_get_time();
        if (now < d->available_at)
        {
            pthread_mutex_unlock(&d->mutex);
            ft_usleep(1);
            continue;
        }
        if (coder->global->scheduler == 1)
        {
        }
        break ;
    }
}

static int	ft_check_end(t_program *pgm)
{
	int	end;

	pthread_mutex_lock(&pgm->status_mutex);
	end = pgm->simulation_end;
	pthread_mutex_unlock(&pgm->status_mutex);
	return (end);
}

int ft_is_urgent(t_coder *coder)
{
    int i;
    long long my_deadline;
    long long other_deadline;

    my_deadline = coder->last_compile_start + coder->global->time_to_burnout;
    i = 0;
    while (i < coder->global->total_coders)
    {
        // Comparamos nuestro deadline con el de los demás
        other_deadline = coder->global->coders[i].last_compile_start + coder->global->time_to_burnout;
        if (coder->id != coder->global->coders[i].id && other_deadline < my_deadline)
            return (0); // Hay alguien con un deadline más cercano
        i++;
    }
    return (1);
}

int ft_compile(t_coder *coder)
{
    t_dongle *first;
    t_dongle *second;

    // Asimetría para evitar deadlock
    first = (coder->id % 2 != 0) ? coder->left_dongle : coder->right_dongle;
    second = (coder->id % 2 != 0) ? coder->right_dongle : coder->left_dongle;
    while (1)
    {
        pthread_mutex_lock(&first->mutex);
        if (ft_get_time() >= first->available_at)
        {
            // Si es EDF, comprobamos si debemos esperar
            if (coder->global->scheduler == 1 && !ft_is_urgent(coder))
            {
                pthread_mutex_unlock(&first->mutex);
                ft_usleep(1); // Cedemos brevemente a alguien más urgente
                continue;
            }
            break;
        }
        pthread_mutex_unlock(&first->mutex);
        ft_usleep(1);
    }
    ft_print(coder, "has taken a dongle");
    pthread_mutex_lock(&second->mutex);
    ft_print(coder, "has taken the other dongle");
    pthread_mutex_lock(&coder->global->status_mutex);
    coder->last_compile_start = ft_get_time();
    pthread_mutex_unlock(&coder->global->status_mutex);
    ft_print(coder, "is compiling");
    ft_usleep(coder->global->time_to_compile);
    long long end_cooldown = ft_get_time() + coder->global->cooldown;
    first->available_at = end_cooldown;
    second->available_at = end_cooldown;
    coder->compile_count++;
    pthread_mutex_unlock(&second->mutex);
    pthread_mutex_unlock(&first->mutex);
    return (1);
}

void	ft_debug(t_coder *coder)
{
	if (ft_check_end(coder->global))
		return ;
	ft_print(coder, "is debugging");
	ft_usleep(coder->global->time_to_debug);
}

void	ft_refactor(t_coder *coder)
{
	if (ft_check_end(coder->global))
		return ;
	ft_print(coder, "is refactoring");
	ft_usleep(coder->global->time_to_refactor);
}

void	*ft_coder_routine(void *arg)
{
	t_coder		*coder;
	t_program	*pgm;

	coder = (t_coder *)arg;
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
		if (!ft_compile(coder))
			break ;
		ft_debug(coder);
		ft_refactor(coder);
	}
	return (NULL);
}
