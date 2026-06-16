

int ft_start_simulation(t_program *pgm)
{
    int i = 0;
    while (i < pgm->total_coders)
    {
        pthread_create(&pgm->coders[i].thread_id, NULL, ft_coder_routine, &pgm->coders[i]);
        i++;
    }

    i = 0;
    while (i < pgm->total_coders)
    {
        pthread_join(pgm->coders[i].thread_id, NULL);
        i++;
    }   
    return 0; // Placeholder implementation
}   