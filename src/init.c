/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dcuenca <dcuenca@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/19 14:44:31 by dcuenca           #+#    #+#             */
/*   Updated: 2026/06/19 14:44:31 by dcuenca          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/codexion.h"

int ft_init_dongles(t_program *pgm)
{
    int i = 0;
    while (i < pgm->total_coders)
    {
        if (pthread_mutex_init(&pgm->dongles[i].mutex, NULL) != 0)
        {
            printf("Error: Failed to initialize mutex for dongle %d.\n", i);
            return 1;
        }
        i++;
    }
    return 0; // Placeholder implementation
}

int ft_init_coders(t_program *pgm)
{
    int i = 0;
    while (i < pgm->total_coders)
    {
        pgm->coders[i].id = i+1;
        pgm->coders[i].compile_count = 0;
        pgm->coders[i].last_compile_start = 0;
        pgm->coders[i].global = pgm;
        pgm->coders[i].left_dongle = &pgm->dongles[i];
        pgm->coders[i].right_dongle = &pgm->dongles[(i + 1) % pgm->total_coders];
        i++;
    }
    return 0; // Placeholder implementation
}


void ft_init(t_program *pgm)
{
    pthread_mutex_init(&pgm->write_mutex, NULL);
    pthread_mutex_init(&pgm->status_mutex, NULL);
    ft_init_dongles(pgm);
    ft_init_coders(pgm);
}