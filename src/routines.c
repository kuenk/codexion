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

static t_coder	*ft_left_neighbor(t_coder *c)
{
	t_program	*pgm;
	int			n;
	int			idx;

	pgm = c->global;
	n = pgm->total_coders;
	idx = c->id - 1;
	return (&pgm->coders[(idx - 1 + n) % n]);
}

static t_coder	*ft_right_neighbor(t_coder *c)
{
	t_program	*pgm;
	int			n;
	int			idx;

	pgm = c->global;
	n = pgm->total_coders;
	idx = c->id - 1;
	return (&pgm->coders[(idx + 1) % n]);
}

static long long	ft_priority_key(t_program *pgm, t_coder *c)
{
	if (pgm->scheduler == 0)
		return (c->request_time);
	return (c->last_compile_start + pgm->time_to_burnout);
}

static int	ft_loses_to(t_program *pgm, t_coder *me, t_coder *rival)
{
	if (!rival->is_waiting)
		return (0);
	return (ft_priority_key(pgm, rival) < ft_priority_key(pgm, me));
}

static int	ft_can_take_dongles(t_coder *coder)
{
	t_program	*pgm;
	long long	now;

	pgm = coder->global;
	now = ft_get_time();
	if (now < coder->left_dongle->available_at)
		return (0);
	if (now < coder->right_dongle->available_at)
		return (0);
	if (coder->left_dongle != coder->right_dongle)
	{
		if (ft_loses_to(pgm, coder, ft_left_neighbor(coder)))
			return (0);
		if (ft_loses_to(pgm, coder, ft_right_neighbor(coder)))
			return (0);
	}
	return (1);
}

static int	ft_acquire_dongles(t_coder *coder)
{
	t_program	*pgm;
	int			single;

	pgm = coder->global;
	pthread_mutex_lock(&pgm->status_mutex);
	if (pgm->simulation_end)
	{
		pthread_mutex_unlock(&pgm->status_mutex);
		return (0);
	}
	coder->is_waiting = 1;
	coder->request_time = ft_get_time();
	while (!pgm->simulation_end && !ft_can_take_dongles(coder))
	{
		pthread_mutex_unlock(&pgm->status_mutex);
		ft_usleep(1);
		pthread_mutex_lock(&pgm->status_mutex);
	}
	if (pgm->simulation_end)
	{
		coder->is_waiting = 0;
		pthread_mutex_unlock(&pgm->status_mutex);
		return (0);
	}
	coder->left_dongle->available_at = LLONG_MAX;
	coder->right_dongle->available_at = LLONG_MAX;
	coder->last_compile_start = ft_get_time();
	coder->is_waiting = 0;
	coder->is_compiling = 1;
	single = (coder->left_dongle == coder->right_dongle);
	pthread_mutex_unlock(&pgm->status_mutex);
	ft_print(coder, "has taken a dongle");
	if (!single)
		ft_print(coder, "has taken a dongle");
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
	while (!ft_check_end(pgm) && coder->compile_count < pgm->num_compiles)
	{
		if (!ft_compile(coder))
			break ;
		ft_debug(coder);
		ft_refactor(coder);
	}
	return (NULL);
}