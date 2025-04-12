#include <stdio.h>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      starterkit.c                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#define MAX_PATH 1024
#define LOG_FILE "activity.log"
#define DOWNLOAD "download.sh"
#define ZIP_URL "https://drive.usercontent.google.com/u/0/uc?id=1_5GxIGfQr3mNKuavJbte_AoRkEQLXSKS&export=download"

void write_log(const char *action, const char *filename, const char *status, pid_t pid) {
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);

    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "[%d-%m-%Y][%H:%M:%S]", local);

    FILE *log = fopen(LOG_FILE, "a");
    if (log == NULL) {
        perror("Failed to open log file");
        return;
    }

    if (strcmp(action, "Decrypt") == 0 || strcmp(action, "Shutdown") == 0) {
        fprintf(log, "%s - %s %s process with PID %d.\n", timestamp, status, action, pid);
    } else {
        fprintf(log, "%s - %s - %s %s.\n", timestamp, filename, status, action);
    }

    fclose(log);
}

char* base64_decode(const char* input) {
    const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int input_len = strlen(input);
    char *decoded = malloc(input_len);
    int i = 0, j = 0;
    unsigned char char_array_4[4], char_array_3[3];
    int decoded_len = 0;

    if (input_len % 4 != 0) {
        fprintf(stderr, "Invalid Base64 input length\n");
        free(decoded);
        return NULL;
    }

    while (input_len-- && input[i] != '=') {
        char_array_4[j++] = input[i++];
        if (j == 4) {
            for (j = 0; j < 4; j++) {
                char *pos = strchr(base64_table, char_array_4[j]);
                if (pos == NULL) {
                    free(decoded);
                    return NULL;
                }
                char_array_4[j] = pos - base64_table;
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0x0f) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x03) << 6) + char_array_4[3];

            for (j = 0; j < 3; j++) {
                decoded[decoded_len++] = char_array_3[j];
            }
            j = 0;
        }
    }

    if (j > 0) {
        for (int k = j; k < 4; k++)
            char_array_4[k] = 0;

        for (int k = 0; k < 4; k++) {
            char *pos = strchr(base64_table, char_array_4[k]);
            if (pos == NULL) {
                free(decoded);
                return NULL;
            }
            char_array_4[k] = pos - base64_table;
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0x0f) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x03) << 6) + char_array_4[3];

        for (int k = 0; k < j - 1; k++) {
            decoded[decoded_len++] = char_array_3[k];
        }
    }

    decoded[decoded_len] = '\0';
    return decoded;
}

void decrypt_filenames() {
    DIR *dir;
    struct dirent *entry;
    char old_path[MAX_PATH], new_path[MAX_PATH];

   dir = opendir("quarantine");
    if (dir == NULL) {
        perror("Failed to open quarantine directory");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char *decoded = base64_decode(entry->d_name);
        if (decoded == NULL) {
            fprintf(stderr, "Failed to decode filename: %s\n", entry->d_name);
            continue;
        }

        snprintf(old_path, sizeof(old_path), "quarantine/%s", entry->d_name);
        snprintf(new_path, sizeof(new_path), "quarantine/%s", decoded);

        if (rename(old_path, new_path) != 0) {
            perror("Failed to rename file");
            free(decoded);
            continue;
        }

        write_log("Decrypt", decoded, "Successfully decrypted", getpid());
        free(decoded);
    }

    closedir(dir);
}

void start_decrypt_daemon() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        printf("Decryption daemon started with PID: %d\n", pid);
        write_log("Decrypt", "", "Successfully started", pid);
        exit(EXIT_SUCCESS);
    }

    umask(0);
    setsid();
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    while (1) {
        decrypt_filenames();
        sleep(5);
    }
}

void move_files(const char *src_dir, const char *dest_dir, const char *action) {
    DIR *dir;
    struct dirent *entry;
    char src_path[MAX_PATH], dest_path[MAX_PATH];

    dir = opendir(src_dir);
    if (dir == NULL) {
        perror("Failed to open source directory");
        exit(EXIT_FAILURE);
    }

    mkdir(dest_dir, 0755);

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, entry->d_name);

        if (rename(src_path, dest_path) != 0) {
            perror("Failed to move file");
            continue;
        }

        write_log(action, entry->d_name, "Successfully moved", 0);
    }

    closedir(dir);
}

void eradicate_files() {
    DIR *dir;
    struct dirent *entry;
    char path[MAX_PATH];

    dir = opendir("quarantine");
    if (dir == NULL) {
        perror("Failed to open quarantine directory");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "quarantine/%s", entry->d_name);

        if (remove(path) != 0) {
            perror("Failed to delete file");
            continue;
        }

        write_log("Eradicate", entry->d_name, "Successfully deleted", 0);
    }

    closedir(dir);
}

void shutdown_daemon(pid_t pid) {
    if (kill(pid, SIGTERM) != 0) {
        perror("Failed to kill process");
        exit(EXIT_FAILURE);
    }

    write_log("Shutdown", "", "Successfully shut off", pid);
    printf("Successfully shut off decryption process with PID %d\n", pid);
}

int run_shell_script(const char *script_path) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        return -1;
    }
    else if (pid == 0) {
        // Child process - jalankan script
        execl("/bin/sh", "sh", script_path, NULL);

        // Jika sampai sini berarti exec gagal
        perror("execl failed");
        _exit(1);
    }
    else {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    }
}

void create_download_script() {
    FILE *script = fopen(DOWNLOAD, "w");
    if (!script) {
        perror("Failed to create download script");
   exit(EXIT_FAILURE);
    }

    fprintf(script, "#!/bin/sh\n");
    fprintf(script, "echo 'Downloading starter kit...'\n");
    fprintf(script, "if ! wget --quiet --show-progress \"%s\" -O temp.zip; then\n", ZIP_URL);
    fprintf(script, "    echo 'Download failed' >&2\n");
    fprintf(script, "    exit 1\n");
    fprintf(script, "fi\n");
    fprintf(script, "echo 'Unzipping...'\n");
    fprintf(script, "if ! unzip -q temp.zip -d starter_kit; then\n");
    fprintf(script, "    echo 'Unzip failed' >&2\n");
    fprintf(script, "    rm temp.zip\n");
    fprintf(script, "    exit 1\n");
    fprintf(script, "fi\n");
    fprintf(script, "rm temp.zip\n");
    fprintf(script, "echo 'Done!'\n");
    fprintf(script, "exit 0\n");

    fclose(script);

    chmod(DOWNLOAD, 0755);
}

int is_dir_empty(const char *path) {
    DIR *dir = opendir(path);
    if (dir == NULL) return 1;

    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (++count > 2) break; 
    }
    closedir(dir);
    return count <= 2;
}

int main(int argc, char *argv[]) {
    struct stat st = {0};
   if (stat("quarantine", &st) == -1) {
        mkdir("quarantine", 0755);
    }
    if (stat("starter_kit", &st) == -1) {
        mkdir("starter_kit", 0755);
    }

    if (argc < 2 || (strcmp(argv[1], " ")) == 0) {
        if (is_dir_empty("starter_kit")) {
            printf("Preparing starter kit...\n");
          
            create_download_script();

            if (run_shell_script(DOWNLOAD) != 0) {
                fprintf(stderr, "Failed to setup starter kit\n");
                remove(DOWNLOAD);
                exit(EXIT_FAILURE);
            }
          
            remove(DOWNLOAD);
            write_log("Download", "", "Starter kit downloaded and extracted", getpid());
        }
    }
    else if (strcmp(argv[1], "--decrypt") == 0){

        start_decrypt_daemon();
    }
    else if (strcmp(argv[1], "--quarantine") == 0) {
        move_files("starter_kit", "quarantine", "Quarantine");
    }
    else if (strcmp(argv[1], "--return") == 0) {
        move_files("quarantine", "starter_kit", "Return");
    }
    else if (strcmp(argv[1], "--eradicate") == 0) {
        eradicate_files();
    }
    else if (strcmp(argv[1], "--shutdown") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Invalid tidak ada <PID>\n");
            return EXIT_FAILURE;
        }
        pid_t pid = atoi(argv[2]);
        shutdown_daemon(pid);
    }
    else {
        fprintf(stderr, "Opsi invalid\n");
        fprintf(stderr, "Argumen tidak valid\n");
        return EXIT_FAILURE;
    }
   return EXIT_SUCCESS;
}

