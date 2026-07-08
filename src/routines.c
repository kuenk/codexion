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

int	ft_check_end(t_program *pgm)
{
	int	end;

	pthread_mutex_lock(&pgm->status_mutex);
	end = pgm->simulation_end;
	pthread_mutex_unlock(&pgm->status_mutex);
	return (end);
}

static void	ft_phase(t_coder *coder, char *msg, long long duration)
{
	if (ft_check_end(coder->global))
		return ;
	ft_print(coder, msg);
	ft_usleep(duration);
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
		ft_phase(coder, "is debugging", pgm->time_to_debug);
		ft_phase(coder, "is refactoring", pgm->time_to_refactor);
	}
	return (NULL);
}
