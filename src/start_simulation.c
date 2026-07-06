/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   start_simulation.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dcuenca <dcuenca@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/19 14:44:37 by dcuenca           #+#    #+#             */
/*   Updated: 2026/06/19 14:44:37 by dcuenca          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/codexion.h"

static int	ft_check_death(t_program *pgm, int i)
{
	pthread_mutex_lock(&pgm->status_mutex);
	if ((long long)(ft_get_time() - pgm->coders[i].last_compile_start)
		> pgm->time_to_burnout)
	{
		pthread_mutex_lock(&pgm->write_mutex);
		pgm->simulation_end = 1;
		printf("%llu %d has burned out.\n",
			(unsigned long long)(ft_get_time() - pgm->start_time),
			pgm->coders[i].id);
		pthread_mutex_unlock(&pgm->write_mutex);
		pthread_mutex_unlock(&pgm->status_mutex);
		return (1);
	}
	pthread_mutex_unlock(&pgm->status_mutex);
	return (0);
}

static int	ft_all_coders_finished(t_program *pgm)
{
	int	i;

	if (pgm->num_compiles == -1)
		return (0);
	i = 0;
	while (i < pgm->total_coders)
	{
		if (pgm->coders[i].compile_count < pgm->num_compiles)
			return (0);
		i++;
	}
	return (1);
}

void	*ft_supervisor(void *arg)
{
	t_program	*pgm;
	int			i;

	pgm = (t_program *)arg;
	while (1)
	{
		pthread_mutex_lock(&pgm->status_mutex);
		if (pgm->simulation_end)
		{
			pthread_mutex_unlock(&pgm->status_mutex);
			break ;
		}
		pthread_mutex_unlock(&pgm->status_mutex);
		i = 0;
		while (i < pgm->total_coders)
		{
			if (ft_check_death(pgm, i))
				return (NULL);
			i++;
		}
		if (ft_all_coders_finished(pgm))
		{
			pthread_mutex_lock(&pgm->status_mutex);
			pgm->simulation_end = 1;
			pthread_mutex_unlock(&pgm->status_mutex);
			break ;
		}
		ft_usleep(1);
	}
	return (NULL);
}

int	ft_start_simulation(t_program *pgm)
{
	int			i;
	pthread_t	supervisor;

	i = 0;
	pgm->start_time = ft_get_time();
	while (i < pgm->total_coders)
	{
		pgm->coders[i].last_compile_start = pgm->start_time;
		pthread_create(&pgm->coders[i].thread_id, NULL,
			ft_coder_routine, &pgm->coders[i]);
		i++;
	}
	pthread_create(&supervisor, NULL, ft_supervisor, pgm);
	i = 0;
	while (i < pgm->total_coders)
	{
		pthread_join(pgm->coders[i].thread_id, NULL);
		i++;
	}
	pthread_join(supervisor, NULL);
	return (0);
}
