/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_a.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dcuenca <dcuenca@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/23 12:03:52 by dcuenca           #+#    #+#             */
/*   Updated: 2026/06/23 12:03:52 by dcuenca          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../include/codexion.h"
int ft_mem_alloc(t_program *pgm)
{
	pgm->coders = malloc(sizeof(t_coder) * pgm->total_coders);
	if (!pgm->coders)
	{
		printf("Error: Failed to allocate memory in ('coders').\n");
		return (1);
	}
	pgm->dongles = malloc(sizeof(t_dongle) * pgm->total_coders);
	if (!pgm->dongles)
	{
		printf("Error: Failed to allocate memory in ('dongles').\n");
		free(pgm->coders);
		return (1);
	}
	return (0);
}


size_t ft_get_time(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + (tv.tv_usec / 1000));
}

void ft_usleep(size_t milliseconds)
{
	size_t start_time = ft_get_time();
	while (ft_get_time() - start_time < milliseconds)
		usleep(100);
}

void ft_clean_all(t_program *pgm)
{
    int i;
    i = 0;
    while (i < pgm->total_coders)
    {
        pthread_mutex_destroy(&pgm->dongles[i].mutex);
        i++;
    }
    pthread_mutex_destroy(&pgm->write_mutex);
	pthread_mutex_destroy(&pgm->status_mutex);

    if (pgm->coders)
        free(pgm->coders);
    if (pgm->dongles)
        free(pgm->dongles);
}

void ft_print(t_coder *coder, char *msg)
{
    pthread_mutex_lock(&coder->global->write_mutex);
	printf("%llu %d %s\n",
				(unsigned long long)(ft_get_time() - coder->global->start_time),
				coder->id, msg);
    pthread_mutex_unlock(&coder->global->write_mutex);
}