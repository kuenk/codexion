/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_priority.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dcuenca <dcuenca@student.42madrid.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/08 10:57:50 by dcuenca           #+#    #+#             */
/*   Updated: 2026/07/08 10:58:33 by dcuenca          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/codexion.h"

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

int	ft_can_take_dongles(t_coder *coder)
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
