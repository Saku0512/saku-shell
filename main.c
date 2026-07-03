#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE    1024
#define MAX_ARGS    64

void print_prompt(void);
void remove_newline(char *line);
int parse_args(char *line, char **args);
int run_buildin(char **args);
void run_command(char **args);

int main(void) {
    char line[MAX_LINE];
    char *args[MAX_ARGS];

    while (1) {
        print_prompt();

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }

        remove_newline(line);

        if (line[0] == '\0') {
            continue;
        }

        parse_args(line, args);

        if (run_buildin(args)) {
            continue;
        }

        run_command(args);
    }

    return 0;
}

void print_prompt(void) {
    printf("saku-shell> ");
    fflush(stdout);
}

void remove_newline(char *line) {
    line[strcspn(line, "\n")] = '\0';
}

int parse_args(char *line, char **args) {
    int argc = 0;

    char *token = strtok(line, " \t");
    while (token != NULL && argc < MAX_ARGS - 1) {
        args[argc] = token;
        argc++;
        token = strtok(NULL, " \t");
    }

    args[argc] = NULL;
    return argc;
}

int run_buildin(char **args) {
    if (args[0] == NULL) {
        return 1;
    }

    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }

    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: missing argument\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
        return 1;
    }

    return 0;
}

void run_command(char **args) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        execvp(args[0], args);

        perror("execvp");
        exit(1);
    }

    int status;
    waitpid(pid, &status, 0);
}
