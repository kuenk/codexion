

#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include <sys/time.h>

struct s_program;

typedef struct s_dongle
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int            is_available;
    long long      available_at;
    int            req_coder_id[2];
    long long      req_time[2];
    long long      req_deadline[2];
} t_dongle;

typedef struct s_program
{
    t_coder *coders;
    int     total_coders;
    long long     time_to_burnout;
    long long     time_to_compile;
    long long     time_to_debug;
    long long     time_to_refactor;
    int     num_compiles;
    int     cooldown;
    int     scheduler; 
}   t_program;

typedef struct s_coder
{
    int            id;
    pthread_t      thread_id;
    int            compile_count;
    long long      last_compile_start;
    t_dongle       *left_dongle;
    t_dongle       *right_dongle;

    struct s_codexion *global;
} t_coder;

#endif