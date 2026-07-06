/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dcuenca <dcuenca@student.42madrid.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/06 12:02:56 by dcuenca           #+#    #+#             */
/*   Updated: 2026/07/06 12:02:56 by dcuenca          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <stdlib.h>
# include <stdio.h>
# include <pthread.h>
# include <sys/time.h>
# include <unistd.h>
# include <string.h>

typedef struct s_program	t_program;
typedef struct s_coder		t_coder;
typedef struct s_dongle		t_dongle;

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	long long		available_at;
}	t_dongle;

typedef struct s_program
{
	pthread_mutex_t	write_mutex;
	pthread_mutex_t	status_mutex;
	t_coder			*coders;
	t_dongle		*dongles;
	int				total_coders;
	long long		time_to_burnout;
	long long		time_to_compile;
	long long		time_to_debug;
	long long		time_to_refactor;
	int				num_compiles;
	int				cooldown;
	int				scheduler;
	int				simulation_end;
	long long		start_time;
}	t_program;

typedef struct s_coder
{
	int			id;
	pthread_t	thread_id;
	int			compile_count;
	long long	last_compile_start;
	t_dongle	*left_dongle;
	t_dongle	*right_dongle;
	t_program	*global;
}	t_coder;

int			ft_parsing(int argc, char **argv, t_program *program);
int			ft_mem_alloc(t_program *pgm);
void		ft_init(t_program *pgm);
int			ft_start_simulation(t_program *pgm);
void		ft_clean_all(t_program *pgm);
void		*ft_coder_routine(void *arg);
int			ft_compile(t_coder *coder);
size_t		ft_get_time(void);
void		ft_usleep(size_t milliseconds);
void		ft_print(t_coder *coder, char *msg);

#endif