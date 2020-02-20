#include "ush.h"

static int count_args(char **args, int n_options);
static void fill_options(int n_options, t_jobs *jobs_op, char **args);
static void print_jobs_by_mask(t_shell *m_s, t_jobs jobs_op, int i);

int mx_builtin_commands_idex(t_shell *m_s, char *command) {
    int i = 0;

    for (i = 0; m_s->builtin_list[i] != NULL; i++) {
        if (strcmp(command, m_s->builtin_list[i]) == 0)
            return (i);
    }
    return (-1);
}

/*
jobs - List background processes
-l   list process IDs in addition to the normal information.
-p  Pass the -p option to jobs command to display PIDs only:  7895 7896
-r  Display only running jobs.
-s  Display only stopped jobs
*/

int mx_jobs(t_shell *m_s, t_process *p) {
    t_jobs jobs_op = {0, 0, 0};
    int job_id;
    int n_option;
    int n_args;

    if ((n_option = mx_count_options(p->argv, "lprs", "jobs", " [-lrs]")) < 0) {
        p->exit_code = 1;
        return p->exit_code;
    }
    n_args = count_args(p->argv, n_option);
    mx_set_last_job(m_s);
    fill_options(n_option, &jobs_op, p->argv);
    if (n_option >= 0 && n_args == 0) {
        for (int i = 0; i < MX_JOBS_NUMBER; i++)
            print_jobs_by_mask(m_s, jobs_op, i);
    }
    else if (n_args) {
        for (int j = n_option + 1; p->argv[j] != NULL; j++) {
            for (int i = m_s->max_number_job;  i > 0; i--) {
                if (m_s->jobs[i] != NULL) {
                    if ((job_id = mx_g_find_job(m_s, (p->argv[j]))) < 1) {
                        mx_error_fg_bg(p->argv[0],": job not found: ",
                                       p->argv[j], "\n");
                        return -1;
                    }
                    print_jobs_by_mask(m_s, jobs_op, i);
                    break;
                }
            }
        }
    }
    p->exit_code = 0;
//    mx_print_stack(m_s);
    return p->exit_code;
}

static void print_jobs_by_mask(t_shell *m_s, t_jobs jobs_op, int i) {
    if (jobs_op.l) {
        if (jobs_op.r) {
            if (m_s->jobs[i] != NULL && mx_job_is_running(m_s, i))
                mx_print_job_status(m_s, i, 1);
            }
            else if (jobs_op.s) {
                if (m_s->jobs[i] != NULL && !mx_job_is_running(m_s, i))
                    mx_print_job_status(m_s, i, 1);
            }
            else {
                if (m_s->jobs[i] != NULL)
                    mx_print_job_status(m_s, i, 1);
            }
    }
    else if (jobs_op.r && !jobs_op.s) {
        if (m_s->jobs[i] != NULL && mx_job_is_running(m_s, i))
            mx_print_job_status(m_s, i, 0);
    }
    else if (jobs_op.s && !jobs_op.r) {
        if (m_s->jobs[i] != NULL && !mx_job_is_running(m_s, i))
            mx_print_job_status(m_s, i, 0);
    }
    else {
        if (m_s->jobs[i] != NULL)
            mx_print_job_status(m_s, i, 0);
    }
}


static int count_args(char **args, int n_options) {
    int n_args = 0;

    for (int i = n_options + 1; args[i] != NULL; i++) {
        n_args++;
    }
//    if (n_args > 1)
//        mx_printerr("ush: jobs: bad option:\n");
//    if (n_args > 2)
//        mx_printerr("ush: cd: too many arguments\n");
    return n_args;
}

static void fill_options(int n_options, t_jobs *jobs_op, char **args) {
    for (int i = n_options; i > 0; i--) {
        for (int j = mx_strlen(args[i]); j > 0; j--) {
            if (args[i][j] == 'l')
                jobs_op->l = 1;
            if (args[i][j] == 'r')
                jobs_op->r = 1;
            if (args[i][j] == 's')
                jobs_op->s = 1;
        }
    }
}

/*
static void print_jobs_by_mask(t_shell *m_s, t_jobs jobs_op, int n_op) {
    if (!n_op) {
        for (int i = 0; i < MX_JOBS_NUMBER; i++) {
            if (m_s->jobs[i] != NULL)
                mx_print_job_status(m_s, i, 0);
        }
    }
    else if (jobs_op.l) {
        if (jobs_op.r) {
            for (int i = 0; i < MX_JOBS_NUMBER; i++)
                if (m_s->jobs[i] != NULL && mx_job_is_running(m_s, i))
                    mx_print_job_status(m_s, i, 1);
        }
        else if (jobs_op.s) {
            for (int i = 0; i < MX_JOBS_NUMBER; i++)
                if (m_s->jobs[i] != NULL && !mx_job_is_running(m_s, i))
                    mx_print_job_status(m_s, i, 1);
        }
        else {
            for (int i = 0; i < MX_JOBS_NUMBER; i++)
                if (m_s->jobs[i] != NULL)
                    mx_print_job_status(m_s, i, 1);
        }
    }
    else if (jobs_op.r && !jobs_op.s) {
        for (int i = 0; i < MX_JOBS_NUMBER; i++)
            if (m_s->jobs[i] != NULL && mx_job_is_running(m_s, i))
                mx_print_job_status(m_s, i, 1);
    }

    else if (jobs_op.s && !jobs_op.r) {
        for (int i = 0; i < MX_JOBS_NUMBER; i++)
            if (m_s->jobs[i] != NULL && !mx_job_is_running(m_s, i))
                mx_print_job_status(m_s, i, 1);
    }
    else {
        for (int i = 0; i < MX_JOBS_NUMBER; i++)
            if (m_s->jobs[i] != NULL)
                mx_print_job_status(m_s, i, 0);
    }
}

*/
