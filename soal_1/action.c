#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <regex.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

#define ZIP_URL "https://drive.usercontent.google.com/u/0/uc?id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK&export=download"
#define ZIP_FILE "Clues.zip"
#define CLUES_DIR "Clues"
#define FILTER_DIR "Filtered"

bool directory_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

void run_command(const char *cmd, char *const argv[]) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(cmd, argv);
        perror("execvp gagal");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "'%s' gagal dijalankan.\n", cmd);
            exit(1);
        }
    } else {
        perror("fork gagal");
        exit(1);
    }
}

void download_and_extract() {
    if (directory_exists(CLUES_DIR)) {
        printf("Folder '%s' sudah ada king\n", CLUES_DIR);
        return;
    }

    printf("Mendownload '%s'\n", ZIP_FILE);
    char *wget_args[] = {"wget", "-qO", ZIP_FILE, (char *)ZIP_URL, NULL};
    run_command("wget", wget_args);

    printf("Mengekstrak '%s'....\n", ZIP_FILE);
    char *unzip_args[] = {"unzip", "-q", ZIP_FILE, NULL};
    run_command("unzip", unzip_args);

    if (unlink(ZIP_FILE) != 0) {
        fprintf(stderr, "Tidak bisa menghapus '%s'.\n", ZIP_FILE);
    }

    printf("Done sir, Folder '%s' resdy to launch\n", CLUES_DIR);
}

bool is_valid_filename(const char *filename) {
    regex_t regex;
    int result;

    result = regcomp(&regex, "^[a-zA-Z0-9]\\.txt$", REG_EXTENDED);
    if (result) return false;

    result = regexec(&regex, filename, 0, NULL, 0);
    regfree(&regex);

    return (result == 0);
}

void filter_files() {
    const char *subdirs[] = {
        "Clues/ClueA", "Clues/ClueB", "Clues/ClueC", "Clues/ClueD"
    };

    if (!directory_exists(FILTER_DIR)) {
        mkdir(FILTER_DIR, 0755);
    }

    for (int i = 0; i < 4; ++i) {
        DIR *dir = opendir(subdirs[i]);
        if (!dir) continue;

        struct dirent *entry;
        char src_path[512];
        char dest_path[512];

        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                const char *fname = entry->d_name;
                snprintf(src_path, sizeof(src_path), "%s/%s", subdirs[i], fname);

                if (is_valid_filename(fname)) {
                    snprintf(dest_path, sizeof(dest_path), "%s/%s", FILTER_DIR, fname);
                    rename(src_path, dest_path);
                } else {
                    unlink(src_path);
                }
            }
        }
        closedir(dir);
    }

    printf("Filtering selesai. Silahkan cek folder '%s'\n", FILTER_DIR);
}

int compare_names(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void combine_files() {
    DIR *dir = opendir(FILTER_DIR);
    if (!dir) {
        perror("Gagal membuka folder Filtered");
        return;
    }

    struct dirent *entry;
    char *angka_files[100], *huruf_files[100];
    int angka_count = 0, huruf_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".txt")) {
            if (isdigit(entry->d_name[0])) {
                angka_files[angka_count++] = strdup(entry->d_name);
            } else if (isalpha(entry->d_name[0])) {
                huruf_files[huruf_count++] = strdup(entry->d_name);
            }
        }
    }
    closedir(dir);

    qsort(angka_files, angka_count, sizeof(char *), compare_names);
    qsort(huruf_files, huruf_count, sizeof(char *), compare_names);

    FILE *combined = fopen("Combined.txt", "w");
    if (!combined) {
        perror("Tidak bisa membuka Combined.txt");
        return;
    }

    int a = 0, h = 0;
    while (a < angka_count || h < huruf_count) {
        if (a < angka_count) {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", FILTER_DIR, angka_files[a]);
            FILE *src = fopen(filepath, "r");
            if (src) {
                char ch;
                while ((ch = fgetc(src)) != EOF) {
                    fputc(ch, combined);
                }
                fclose(src);
                unlink(filepath);
            }
            free(angka_files[a]);
            a++;
        }

        if (h < huruf_count) {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", FILTER_DIR, huruf_files[h]);
            FILE *src = fopen(filepath, "r");
            if (src) {
                char ch;
                while ((ch = fgetc(src)) != EOF) {
                    fputc(ch, combined);
                }
                fclose(src);
                unlink(filepath);
            }
            free(huruf_files[h]);
            h++;
        }
    }

    fclose(combined);
    printf("Done capt, semua file telah digabung ke Combined.txt\n");
}

void decode_file() {
    FILE *src = fopen("Combined.txt", "r");
    if (!src) {
        perror("Tidak bisa membuka Combined.txt");
        return;
    }

    FILE *dest = fopen("Decoded.txt", "w");
    if (!dest) {
        perror("Tidak bisa membuat Decoded.txt");
        fclose(src);
        return;
    }

    char ch;
    while ((ch = fgetc(src)) != EOF) {
        if ((ch >= 'A' && ch <= 'Z')) {
            ch = ((ch - 'A' + 13) % 26) + 'A';
        } else if ((ch >= 'a' && ch <= 'z')) {
            ch = ((ch - 'a' + 13) % 26) + 'a';
        }
        fputc(ch, dest);
    }

    fclose(src);
    fclose(dest);
    printf("Your pass is ready sir\n");
}

int main(int argc, char *argv[]) {
    if (argc == 3 && strcmp(argv[1], "-m") == 0) {
        if (strcmp(argv[2], "Filter") == 0) {
            filter_files();
        } else if (strcmp(argv[2], "Combine") == 0) {
            combine_files();
        } else if (strcmp(argv[2], "Decode") == 0) {
            decode_file();
        } else {
            printf("Argumen tidak dikenali. Gunakan (Filter) / (Combine) / (Decode)\n");
        }
    } else {
        download_and_extract();
    }
    return 0;
}
