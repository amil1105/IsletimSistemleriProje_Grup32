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


    // Sinyal işleyicilerini ayarlayın
    setup_signal_handlers();

    while (1) {
        // Prompt
        printf("> ");
        fflush(stdout);

        // Kullanıcıdan komut al
        if (!fgets(line, MAX_LINE, stdin)) {
            break; // Ctrl+D ile çıkış
        }

        // Yeni satır karakterini kaldır
        line[strcspn(line, "\n")] = '\0';

        // Boş girişleri atla
        if (strlen(line) == 0) {
            continue;
        }

        // Komutları noktalı virgüle göre böl
        char *seq = strtok(line, ";");
        num_sequences = 0;
        while (seq != NULL && num_sequences < MAX_CMDS) {
            cmd_sequence[num_sequences++] = seq;
            seq = strtok(NULL, ";");
        }

        for (int i = 0; i < num_sequences; i++) {
            // Her komut dizisini borulara göre böl
            command cmds[MAX_PIPE_CMDS];
            int num_cmds = 0;
            char *pipe_cmd = strtok(cmd_sequence[i], "|");
            while (pipe_cmd != NULL && num_cmds < MAX_PIPE_CMDS) {
                parse_command(pipe_cmd, &cmds[num_cmds]);
                num_cmds++;
                pipe_cmd = strtok(NULL, "|");
            }

            // Yerleşik Komut: quit
            if (num_cmds > 0 && strcmp(cmds[0].args[0], "quit") == 0) {
                // Eğer arka planda süreçler varsa bekle
                if (bg_head != NULL) {
                    wait_for_bg_processes();
                }
                exit(0);
            }

            // Yerleşik Komut: cd
            if (num_cmds > 0 && strcmp(cmds[0].args[0], "cd") == 0) {
                if (cmds[0].args[1] != NULL) {
                    if (chdir(cmds[0].args[1]) != 0) {
                        perror("cd failed");
                    }
                }
                else {
                    fprintf(stderr, "cd: expected argument\n");
                }
                continue;
            }

            int pipe_fds[2];
            int prev_fd = -1;
            pid_t child_pids[MAX_PIPE_CMDS];
            int child_count = 0;

            for (int j = 0; j < num_cmds; j++) {
                // Son komut değilse pipe oluştur
                if (j < num_cmds - 1) {
                    if (pipe(pipe_fds) < 0) {
                        perror("pipe failed");
                        exit(EXIT_FAILURE);
                    }
                }

                pid = fork();
                if (pid < 0) {
                    perror("fork failed");
                    exit(EXIT_FAILURE);
                }
                else if (pid == 0) {
                    // Child process

                    // SIGINT sinyalini varsayılan yap
                    signal(SIGINT, SIG_DFL);

                    // Önceki pipe'dan gelen veriyi oku
                    if (prev_fd != -1) {
                        if (dup2(prev_fd, STDIN_FILENO) == -1) {
                            perror("dup2 failed");
                            exit(EXIT_FAILURE);
                        }
                        close(prev_fd);
                    }

                    // Sonraki pipe'a yaz
                    if (j < num_cmds - 1) {
                        if (dup2(pipe_fds[1], STDOUT_FILENO) == -1) {
                            perror("dup2 failed");
                            exit(EXIT_FAILURE);
                        }
                        close(pipe_fds[0]);
                        close(pipe_fds[1]);
                    }

                    // I/O yönlendirmesini yap
                    if (cmds[j].input) {
                        int fd_in = open(cmds[j].input, O_RDONLY);
                        if (fd_in < 0) {
                            perror("Giriş dosyası bulunamadı");
                            exit(EXIT_FAILURE);
                        }
                        if (dup2(fd_in, STDIN_FILENO) == -1) {
                            perror("dup2 failed");
                            exit(EXIT_FAILURE);
                        }
                        close(fd_in);
                    }
                    if (cmds[j].output) {
                        int fd_out;
                        if (cmds[j].append) {
                            fd_out = open(cmds[j].output, O_WRONLY | O_CREAT | O_APPEND, 0644);
                        }
                        else {
                            fd_out = open(cmds[j].output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        }
                        if (fd_out < 0) {
                            perror("Çıkış dosyası açılamadı");
                            exit(EXIT_FAILURE);
                        }
                        if (dup2(fd_out, STDOUT_FILENO) == -1) {
                            perror("dup2 failed");
                            exit(EXIT_FAILURE);
                        }
                        close(fd_out);
                    }

                    // Komutu çalıştır
                    if (execvp(cmds[j].args[0], cmds[j].args) == -1) {
                        perror("exec failed");
                        exit(EXIT_FAILURE);
                    }
                }
                else {
                    // Parent process

                    child_pids[child_count++] = pid;

                    // Önceki pipe'ı kapat
                    if (prev_fd != -1) {
                        close(prev_fd);
                    }

                    // Sonraki komut için okunacak pipe'ı sakla
                    if (j < num_cmds - 1) {
                        close(pipe_fds[1]); // Close write end in parent
                        prev_fd = pipe_fds[0]; // Save read end for next command
                    }

                    // Arka plan işlemi ise ekle
                    if (cmds[j].background) {
                        add_bg_process(pid);
                        printf("[%d] running in background\n", pid);
                    }
                }
            }

            // Tüm çocuk süreçlerini bekle
            for (int k = 0; k < child_count; k++) {
                if (!cmds[k].background) {
                    waitpid(child_pids[k], &status, 0);
                }
            }
        }

        // Bellek sızıntılarını önlemek için süreçler tamamlandığında kaldırılır
        // Ancak burada sigchld_handler zaten kaldırıyor
    }
}
