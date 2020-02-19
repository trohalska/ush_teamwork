#include "ush.h"

static char *check_path(char **arr, char *command);
static char *get_error(char **name, char *command, int *status);
static void print_error(char *command, char *error);

int mx_launch_process(t_shell *m_s, t_process *p, int job_id, char *path, char **env,
                      int infile, int outfile, int errfile) {
    int status = 0;
    pid_t child_pid;
    p->status = STATUS_RUNNING;
    int shell_is_interactive = isatty(STDIN_FILENO);  //!!

    child_pid = fork();
    //TELL_WAIT();
    if (child_pid < 0) {
        perror("error fork");
        exit(1);
    }
    else if (child_pid == 0) {
        //TELL_PARENT(getpgid(0));
        if (shell_is_interactive) {
            if (m_s->jobs[job_id]->pgid == 0)
                m_s->jobs[job_id]->pgid = child_pid;
            setpgid (child_pid, m_s->jobs[job_id]->pgid);
            // mx_print_color(MAG, "child\t");
            // mx_print_color(MAG, "m_s->jobs[job_id]->pgid ");
            // mx_print_color(MAG, mx_itoa(m_s->jobs[job_id]->pgid));
            // mx_printstr("\n");
            if (m_s->jobs[job_id]->foreground)
           // if (p->foreground)
                tcsetpgrp(STDIN_FILENO, m_s->jobs[job_id]->pgid);
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            signal(SIGPIPE, mx_sig_h);
            // signal(SIGCHLD, SIG_DFL);
        }
        if (infile != STDIN_FILENO) {
            dup2(infile, STDIN_FILENO);
            close(infile);
        }
        if (outfile != STDOUT_FILENO) {
            dup2(outfile, STDOUT_FILENO);
            close(outfile);
        }
        if (errfile != STDERR_FILENO) {
            dup2(errfile, STDERR_FILENO);
            close(errfile);
        }
        char **arr = mx_strsplit(path, ':');
        char *command = p->argv[0];
        path  = check_path(arr, command);
        mx_del_strarr(&arr);
        char *error = get_error(&path, command, &status);
        if (execve(path, p->argv, env) < 0) {
            print_error(command, error);
            free(error);
            free(path);
            _exit(status);
        }
        free(path);
        free(error);
        exit(status);
    }
    else {
        p->pid = child_pid;
        //WAIT_CHILD();
        if (shell_is_interactive) {
            pid_t pid = child_pid;
            if (m_s->jobs[job_id]->pgid == 0)
                m_s->jobs[job_id]->pgid = pid;
            setpgid (pid, m_s->jobs[job_id]->pgid);
        }
        // mx_print_color(YEL, "parent\t");
        // mx_print_color(YEL, "p->pid \t");
        // mx_print_color(YEL, mx_itoa(p->pid));
        // mx_print_color(YEL, "\tm_s->jobs[job_id]->pgid ");
        // mx_print_color(YEL, mx_itoa(m_s->jobs[job_id]->pgid));
        // mx_printstr("\n");
    }

    return status >> 8;//WEXITSTATUS(status)
}

static char *check_path(char **arr, char *command) {
    int i = 0;
    char *name = NULL;
    int flag = 0;

    while (arr[i] != NULL && !flag) {
        DIR *dptr  = opendir(arr[i]);
        if (dptr != NULL) {
            struct dirent  *ds;
            while ((ds = readdir(dptr)) != 0) {
                if (strcmp(ds->d_name, command) == 0 && command[0] != '.') {
                    flag++;
                    char *tmp = mx_strjoin(arr[i], "/");
                    name = mx_strjoin(tmp, command);
                    free(tmp);
                    break;
                }
            }
            closedir(dptr);
        }
        i++;
    }
    return name;
}

static char *get_error(char **name, char *command, int *status) {
    char *error = NULL;

    *status = 127;
    if (strstr(command, "/")) {
        *name = command;
        struct stat buff;
        if (lstat(*name, &buff) < 0) {
            error = NULL;//strdup(": No such file or directory\n");
        }
        else {
            if (mx_get_type(buff) == 'd') {
                error = strdup(": is a directory\n");
                *status = 126;
            }
        }
    }
    else
        error = strdup(": command not found\n");
    return error;
}

static void print_error(char *command, char *error) {
    mx_printerr("ush: ");
    if (error) {
        mx_printerr(command);
        mx_printerr(error);
    }
    else
        perror(command);
}
