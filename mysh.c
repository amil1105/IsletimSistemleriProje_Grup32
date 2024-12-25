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
// Arka Plan Süreci Çıkarma
void remove_bg_process(pid_t pid) {
    bg_process *current = bg_head;
    bg_process *prev = NULL;
    while (current != NULL) {
        if (current->pid == pid) {
            if (prev == NULL) {
                bg_head = current->next;
            }
            else {
                prev->next = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

// Tüm Arka Plan Süreçlerini Bekleme
void wait_for_bg_processes() {
    bg_process *current = bg_head;
    while (current != NULL) {
        waitpid(current->pid, NULL, 0);
        printf("[%d] retval: 0\n", current->pid);
        current = current->next;
    }
}

// Sinyal Ayarları
void setup_signal_handlers() {
    struct sigaction sa_chld, sa_int;

    // SIGCHLD Handler
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) {
        perror("sigaction SIGCHLD");
        exit(EXIT_FAILURE);
    }

    // SIGINT Handler
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(EXIT_FAILURE);
    }
}
int main() {
    char line[MAX_LINE];
    char *cmd_sequence[MAX_CMDS];
    int num_sequences;
    pid_t pid;
    int status;

    // PATH'i güncelleme: mevcut PATH + ":." ekle
    char *path = getenv("PATH");
    if (path == NULL) {
        fprintf(stderr, "PATH ortam değişkeni bulunamadı.\n");
        exit(EXIT_FAILURE);
    }

    if (!is_dot_in_path(path)) {
        char new_path[MAX_LINE];
        snprintf(new_path, sizeof(new_path), ".:%s", path); // '.' ekleyerek PATH'in başına ekliyoruz
        if (setenv("PATH", new_path, 1) != 0) {
            perror("setenv failed");
            exit(EXIT_FAILURE);
        }
    }