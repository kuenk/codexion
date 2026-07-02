/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dcuenca <dcuenca@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/19 13:55:43 by dcuenca           #+#    #+#             */
/*   Updated: 2026/06/19 13:55:43 by dcuenca          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/codexion.h"
#include <stdio.h>
#include <pthread.h>

int	main(int argc, char **argv)
{
	t_program	program;

	if (ft_parsing(argc, argv, &program))
		return (1);
	if (ft_mem_alloc(&program))
		return (1);
	ft_init(&program);
	ft_start_simulation(&program);
	ft_clean_all(&program);
	return (0);
}
