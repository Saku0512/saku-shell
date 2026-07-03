#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE                1024
#define MAX_ARGS                64

typedef struct {
    char *args[MAX_ARGS];
    char *output_file;
} Command;

void print_prompt(void);
void remove_newline(char *line);
void init_command(Command *cmd);
int parse_args(char *line, Command *cmd);
int parse_redirect(Command *cmd);
int run_buildin(Command *cmd);
int setup_output_redirect(Command *cmd);
void run_command(Command *cmd);

int main(void) {
    char line[MAX_LINE];

    while (1) {
        Command cmd;
        init_command(&cmd);

        print_prompt();

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }

        remove_newline(line);

        if (line[0] == '\0') {
            continue;
        }

        parse_args(line, &cmd);
        
        if (parse_redirect(&cmd) < 0) {
            continue;
        }

        if (run_buildin(&cmd)) {
            continue;
        }

        run_command(&cmd);
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

void init_command(Command *cmd) {
    for (int i=0; i<MAX_ARGS; i++) {
        cmd->args[i] = NULL;
    }

    cmd->output_file = NULL;
}

int parse_args(char *line, Command *cmd) {
    int argc = 0;

    char *token = strtok(line, " \t");
    while (token != NULL && argc < MAX_ARGS - 1) {
        cmd->args[argc] = token;
        argc++;
        
        token = strtok(NULL, " \t");
    }

    cmd->args[argc] = NULL;
    
    return argc;
}

int parse_redirect(Command *cmd) {
    for (int i=0; cmd->args[i] != NULL; i++) {
        if (strcmp(cmd->args[i], ">") == 0) {
            if (cmd->args[i + 1] == NULL) {
                fprintf(stderr, "syntax error: expected file after >\n");
                return -1;
            }

            if (cmd->args[i + 2] != NULL) {
                fprintf(stderr, "syntax error: extra argument after output file\n");
                return -1;
            }

            cmd->output_file = cmd->args[i + 1];

            cmd->args[i] = NULL;

            if (cmd->args[0] == NULL) {
                fprintf(stderr, "syntax error: missing command\n");
                return -1;
            }

            return 0;
        }
    }

    return 0;
}

int run_buildin(Command *cmd) {
    char **args = cmd->args;

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

int setup_output_redirect(Command *cmd) {
    if (cmd->output_file == NULL) {
        return 0;
    }

    int fd = open(
        cmd->output_file,
        O_WRONLY | O_CREAT | O_TRUNC,
        0644
    );

    if (fd < 0) {
        perror("open");
        return -1;
    }

    if (dup2(fd, STDOUT_FILENO) < 0) {
        perror("dup2");
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}

void run_command(Command *cmd) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        if(setup_output_redirect(cmd) < 0) {
            exit(1);
        }

        execvp(cmd->args[0], cmd->args);

        perror("execvp");
        exit(1);
    }

    int status;
    waitpid(pid, &status, 0);
}
