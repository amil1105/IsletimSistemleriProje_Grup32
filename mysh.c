//AMIL SHIKHIYEV G221210561
//Erkin Erdoğan B241210385
//Kianoush Seddighpour G221210571
//Manar AL SAYED ALI G221210558

// mysh.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_LINE 1024
#define MAX_ARGS 100
#define MAX_CMDS 10
#define MAX_PIPE_CMDS 10

typedef struct command {
    char *args[MAX_ARGS];
    char *input;
    char *output;
    int append;
    int background;
} command;

// Liste arka plan süreçleri için
typedef struct bg_process {
    pid_t pid;
    struct bg_process *next;
} bg_process;

bg_process *bg_head = NULL;

// Fonksiyon Prototipleri
void parse_command(char *line, command *cmd);
void sigchld_handler(int sig);
void sigint_handler(int sig);
void add_bg_process(pid_t pid);
void remove_bg_process(pid_t pid);
void wait_for_bg_processes();
void setup_signal_handlers();

// PATH içinde '.' olup olmadığını kontrol eden fonksiyon
int is_dot_in_path(const char *path) {
    char *path_dup = strdup(path);
    if (!path_dup) return 0;
    char *token = strtok(path_dup, ":");
    while (token != NULL) {
        if (strcmp(token, ".") == 0) {
            free(path_dup);
            return 1;
        }
        token = strtok(NULL, ":");
    }
    free(path_dup);
    return 0;
}

// Komut Ayrıştırma Fonksiyonu
void parse_command(char *line, command *cmd) {
    char *token;
    int arg_index = 0;
    cmd->input = NULL;
    cmd->output = NULL;
    cmd->append = 0;
    cmd->background = 0;

    // Tokenize by space
    token = strtok(line, " ");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                cmd->input = token;
            }
        }
        else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                cmd->output = token;
                cmd->append = 0;
            }
        }
        else if (strcmp(token, ">>") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                cmd->output = token;
                cmd->append = 1;
            }
        }
        else if (strcmp(token, "&") == 0) {
            cmd->background = 1;
        }
        else {
            cmd->args[arg_index++] = token;
        }
        token = strtok(NULL, " ");
    }
    cmd->args[arg_index] = NULL;
}

// SIGCHLD Sinyali Handler
void sigchld_handler(int sig) {
    (void)sig; // Kullanılmayan parametreyi işaretle
    pid_t pid;
    int status;
    // Döngü ile tüm tamamlanmış çocuk süreçleri yakalanır
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("\n[%d] retval: %d\n", pid, WIFEXITED(status) ? WEXITSTATUS(status) : -1);
        printf("> ");
        fflush(stdout);
        remove_bg_process(pid);
    }
}

// SIGINT Sinyali Handler
void sigint_handler(int sig) {
    (void)sig; // Kullanılmayan parametreyi işaretle
    printf("\n> ");
    fflush(stdout);
}

// Arka Plan Süreci Ekleme
void add_bg_process(pid_t pid) {
    bg_process *new_node = malloc(sizeof(bg_process));
    if (!new_node) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    new_node->pid = pid;
    new_node->next = bg_head;
    bg_head = new_node;
}
