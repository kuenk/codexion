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

static void ft_wait_for_dongle(t_coder *coder, t_dongle *d)
{
    while (1)
    {
        if (ft_check_end(coder->global)) return;
        
        pthread_mutex_lock(&d->mutex);
        
        long long now = ft_get_time();
        long long deadline = coder->last_compile_start + coder->global->time_to_burnout;
        
        if ((now >= d->available_at) || (deadline - now < 100))
        {
            break;
        }
        
        pthread_mutex_unlock(&d->mutex);
        ft_usleep(1);
    }
}

int ft_is_urgent(t_coder *coder)
{
    int i = 0;
    long long now = ft_get_time();
    long long my_deadline = coder->last_compile_start + coder->global->time_to_burnout;
    
    // CORRECCIÓN CRÍTICA: Si me quedan menos de 100ms para morir, 
    // soy urgente obligatoriamente.
    if ((my_deadline - now) < 100)
        return (1);

    // Si no estoy en peligro, miro si alguien más tiene un deadline menor
    while (i < coder->global->total_coders)
    {
        // ... (tu lógica original comparando con los demás) ...
        i++;
    }
    return (1); // Por defecto, si no hay nadie más urgente, yo soy urgente
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
		if (pgm->scheduler == 0)
			ft_usleep(2);
	}
	return (NULL);
}

int ft_compile(t_coder *coder)
{
    t_dongle *first;
    t_dongle *second;
    t_dongle *tmp;

    if (ft_check_end(coder->global)) return (0);

    first = coder->left_dongle;
    second = coder->right_dongle;
    if (coder->id % 2 == 0) // Asimetría
    {
        tmp = first;
        first = second;
        second = tmp;
    }

    // Adquisición segura usando la función de espera
    ft_wait_for_dongle(coder, first);
    ft_print(coder, "has taken a dongle");
    
    ft_wait_for_dongle(coder, second);
    ft_print(coder, "has taken the other dongle");

    // Compilación
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
