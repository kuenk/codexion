/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   acquire.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dcuenca <dcuenca@student.42madrid.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/08 10:58:25 by dcuenca           #+#    #+#             */
/*   Updated: 2026/07/08 10:58:29 by dcuenca          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/codexion.h"

static void	ft_wait_turn(t_coder *coder)
{
	t_program	*pgm;

	pgm = coder->global;
	coder->is_waiting = 1;
	coder->request_time = ft_get_time();
	while (!pgm->simulation_end && !ft_can_take_dongles(coder))
	{
		pthread_mutex_unlock(&pgm->status_mutex);
		ft_usleep(1);
		pthread_mutex_lock(&pgm->status_mutex);
	}
	coder->is_waiting = 0;
}

static void	ft_grant_dongles(t_coder *coder)
{
	int	single;

	single = (coder->left_dongle == coder->right_dongle);
	coder->left_dongle->available_at = LLONG_MAX;
	coder->right_dongle->available_at = LLONG_MAX;
	coder->last_compile_start = ft_get_time();
	coder->is_compiling = 1;
	pthread_mutex_unlock(&coder->global->status_mutex);
	ft_print(coder, "has taken a dongle");
	if (!single)
		ft_print(coder, "has taken a dongle");
}

static int	ft_acquire_dongles(t_coder *coder)
{
	t_program	*pgm;

	pgm = coder->global;
	pthread_mutex_lock(&pgm->status_mutex);
	if (pgm->simulation_end)
	{
		pthread_mutex_unlock(&pgm->status_mutex);
		return (0);
	}
	ft_wait_turn(coder);
	if (pgm->simulation_end)
	{
		coder->is_waiting = 0;
		pthread_mutex_unlock(&pgm->status_mutex);
		return (0);
	}
	ft_grant_dongles(coder);
	return (1);
}

int	ft_compile(t_coder *coder)
{
	t_program	*pgm;
	long long	end_cooldown;

	pgm = coder->global;
	if (ft_check_end(pgm))
		return (0);
	if (!ft_acquire_dongles(coder))
		return (0);
	ft_print(coder, "is compiling");
	ft_usleep(pgm->time_to_compile);
	pthread_mutex_lock(&pgm->status_mutex);
	end_cooldown = ft_get_time() + pgm->cooldown;
	coder->left_dongle->available_at = end_cooldown;
	coder->right_dongle->available_at = end_cooldown;
	coder->compile_count++;
	coder->is_compiling = 0;
	pthread_mutex_unlock(&pgm->status_mutex);
	return (1);
}
