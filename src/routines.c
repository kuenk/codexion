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

static int	ft_check_end(t_program *pgm)
{
	int	end;

	pthread_mutex_lock(&pgm->status_mutex);
	end = pgm->simulation_end;
	pthread_mutex_unlock(&pgm->status_mutex);
	return (end);
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

void	ft_get_dongles(t_coder *coder, pthread_mutex_t **f, pthread_mutex_t **s)
{
	if (coder->id % 2 == 0)
	{
		*f = &coder->right_dongle->mutex;
		*s = &coder->left_dongle->mutex;
	}
	else
	{
		*f = &coder->left_dongle->mutex;
		*s = &coder->right_dongle->mutex;
	}
}

int	ft_perform_compile(t_coder *coder, pthread_mutex_t *f, pthread_mutex_t *s)
{
	pthread_mutex_lock(&coder->global->status_mutex);
	coder->last_compile_start = ft_get_time();
	if (coder->global->simulation_end)
	{
		pthread_mutex_unlock(&coder->global->status_mutex);
		return (pthread_mutex_unlock(f), pthread_mutex_unlock(s), 0);
	}
	pthread_mutex_unlock(&coder->global->status_mutex);
	ft_print(coder, "is compiling");
	ft_usleep(coder->global->time_to_compile);
	coder->compile_count++;
	pthread_mutex_unlock(f);
	pthread_mutex_unlock(s);
	return (1);
}

int	ft_compile(t_coder *coder)
{
	pthread_mutex_t	*first;
	pthread_mutex_t	*second;

	if (ft_check_end(coder->global))
		return (0);
	ft_get_dongles(coder, &first, &second);
	pthread_mutex_lock(first);
	ft_print(coder, "has taken a dongle");
	pthread_mutex_lock(second);
	ft_print(coder, "has taken the other dongle");
	if (!ft_perform_compile(coder, first, second))
		return (0);
	return (1);
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
	ft_usleep(coder->id * 10);
	
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
