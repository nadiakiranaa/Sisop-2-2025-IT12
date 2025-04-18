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
``bool directory_exists()`` adalah deklarasi fungsi yang menerima argumen path berupa string path ke folder, dan akan mengembalikan true jika folder tersebut ada, false jika tidak. ``struct stat st`` Membuat variabel st dari struct stat. Struct ini digunakan untuk menyimpan informasi tentang file atau direktori ``stat(path, &st)`` Memanggil fungsi stat() untuk mengisi struct st dengan informasi tentang file/direktori yang ada di path. ``S_ISDIR(st.st_mode)`` Mengecek apakah path tersebut adalah direktori (bukan file biasa). Fungsi makro ini membaca st_mode dan mengonfirmasi apakah itu direktori. ``char *wget_args[]`` Menyusun argumen untuk menjalankan perintah wget, `-q` berarti quiet mode (nggak print output), `-O Clues.zip` berarti output-nya disimpan ke file `Clues.zip`.

B. Filtering the Files

Karena kebanyakan dari file tersebut berawal dengan 1 huruf atau angka, kamu pun mencoba untuk memindahkan file-file yang hanya dinamakan dengan 1 huruf dan 1 angka tanpa special character kedalam folder bernama Filtered. Kamu tidak suka kalau terdapat banyak clue yang tidak berguna jadi disaat melakukan filtering, file yang tidak terfilter dihapus. Karena kamu tidak ingin membuat file kode lagi untuk filtering, maka kamu menggunakan file sebelumnya untuk filtering file-file tadi dengan menambahkan argumen saat ingin menjalankan action.c
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
``regex`` struktur data untuk ekspresi reguler `(regex_t)`. `regcomp()` → meng-compile regex `^[a-zA-Z0-9]\\.txt$`. `regexec` menjalankan regex terhadap filename. `if (!directory_exists(FILTER_DIR))` Cek apakah folder Filtered (dalam FILTER_DIR) sudah ada, Jika belum, buat folder tersebut dengan permission 0755 (rwxr-xr-x). ``struct dirent *entry`` pointer untuk menyimpan setiap file dalam folder. `src_path` path lengkap ke file sumber. `dest_path` path tujuan untuk file yang dipindah. ``while ((entry = readdir(dir)) != NULL) {}`` Baca isi folder satu per satu hingga habis. ``if (entry->d_type == DT_REG){}`` Jika entri yang dibaca dari folder adalah file biasa, maka lanjutkan proses di dalam blok if ini. ``snprintf(src_path, sizeof(src_path), "%s/%s", subdirs[i], fname)`` Ambil nama file dan buat path lengkap ke file sumber (src_path). ``if (is_valid_filename(fname))`` Jika nama file valid (seperti 1.txt, a.txt). `rename(src_path, dest_path);` Pindahkan file dari src_path ke dest_path menggunakan rename().

C. Combine the File Content

Di setiap file .txt yang telah difilter terdapat satu huruf dan agar terdapat progress, Cyrus memberikan clue tambahan untuk meletakan/redirect isi dari setiap `.txt` file tersebut kedalam satu file yaitu Combined.txt dengan menggunakan FILE pointer. Tetapi, terdapat urutan khusus saat redirect isi dari .txt tersebut, yaitu urutannya bergantian dari .txt dengan nama angka lalu huruf lalu angka lagi lalu huruf lagi. Lalu semua file .txt sebelumnya dihapus.
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
`DIR *dir = opendir(FILTER_DIR);` Membuka folder Filtered, jika gagal akan menampilkan pesan "Gagal membuka folder Filtered". `entry` digunakan untuk membaca setiap file dalam folder. `angka_files[]` menyimpan nama file yang diawali dengan angka. `huruf_files[]` menyimpan nama file yang diawali dengan huruf. `while ((entry = readdir(dir)) != NULL)` Melakukan loop pada semua isi folder Filtered. `if (entry->d_type == DT_REG && strstr(entry->d_name, ".txt")) {}` Hanya lanjut jika file biasa (DT_REG) dan punya ekstensi `.txt`. `if (isdigit(entry->d_name[0]))` Jika nama file diawali angka, simpan ke angka_files, begitupun dengan huruf. `strdup` membuat salinan string (karena pointer yang dikembalikan readdir tidak bisa disimpan langsung). `FILE *combined = fopen("Combined.txt", "w")` Membuka (atau membuat) file Combined.txt untuk ditulis. `while (a < angka_count || h < huruf_count)` Loop selama masih ada file angka atau huruf yang belum diproses. `if (a < angka_count)` Mengecek apakah masih ada file yang namanya diawali angka yang belum diproses. `snprintf(filepath, sizeof(filepath), "%s/%s", FILTER_DIR, angka_files[a])` Fungsi snprintf() menulis string ke dalam array filepath dengan aman (tidak melebihi 512 karakter), Format string-nya adalah: "%s/%s" artinya FOLDER/NAMAFILE. `FILE *src = fopen(filepath, "r");` Membuka file yang path-nya tersimpan dalam filepath. `while ((ch = fgetc(src)) != EOF)` Fungsi fgetc(src) membaca 1 karakter dari file yang terbuka, Loop terus berjalan selama belum mencapai EOF (End of File). `fputc(ch, combined)` Menuliskan karakter ch yang dibaca ke file Combined.txt (file yang terbuka dan ditulis sebelumnya). 

D. Decode the file

Karena isi Combined.txt merupakan string yang random, kamu memiliki ide untuk menggunakan Rot13 untuk decode string tersebut dan meletakan hasil dari yang telah di-decode tadi kedalam file bernama Decoded.txt.
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
`FILE *src = fopen("Combined.txt", "r")` Membuka file Combined.txt untuk dibaca ("r" = read), FILE *src adalah pointer ke file sumber (source). `FILE *dest = fopen("Decoded.txt", "w")`  Membuka atau membuat file Decoded.txt untuk ditulis ("w" = write), FILE *dest adalah pointer ke file tujuan (destination). `while ((ch = fgetc(src)) != EOF)` selama belum mencapai akhir file, loop akan terus berjalan.

E. Password Check

nah, berdasarkan decode menggunakan ROT13 didapatkan passwordnya yaitu [password](test/vercel.png)

eh [realpassfr](test/pass.png) lalu kita masukkan ke dalam [sini🐉](https://dragon-pw-checker.vercel.app/).

Done sir ^^

## Soal-2
## Soal-3
A. - Malware ini bekerja secara daemon
```
int main(int argc, char *argv[]) {
    pid_t pid, sid;

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); 

    umask(0);
    sid = setsid();
    if (sid < 0) exit(EXIT_FAILURE);
    if ((chdir(".")) < 0) exit(EXIT_FAILURE); 

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
```
 - Mengganti namanya menjadi /init. 
```
void rename_process(int argc, char *argv[], const char *new_name) {
    strncpy(argv[0], new_name, strlen(argv[0]));
    for (int i = 1; i < argc; i++) {
        memset(argv[i], 0, strlen(argv[i]));
    }
}
```
```
 rename_process(argc, argv, "/init");
```
## Hasil Output 
A. Membuat folder malware

![WhatsApp Image 2025-04-18 at 12 27 40_a5345e70](https://github.com/user-attachments/assets/03191d59-a67a-42e3-b1e3-476a21ed4e7e)

Mengganti namanya menjadi /init
![WhatsApp Image 2025-04-18 at 12 28 00_6a9c5289](https://github.com/user-attachments/assets/8c791bce-939d-4087-a9a9-da576c557a6f)



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
## Hasil Output
A. Menampilkan daftar semua proses yang sedang berjalan pada user 
![WhatsApp Image 2025-04-18 at 11 55 04_23a6153a](https://github.com/user-attachments/assets/4c04e338-ad66-4f38-bbc5-0ba7303575a3)

B. Menjalankan debugmon secara daemon
![WhatsApp Image 2025-04-18 at 11 55 23_9b8c5dc9](https://github.com/user-attachments/assets/78442b89-3899-4b20-bad9-939450b42734)
![WhatsApp Image 2025-04-18 at 11 57 03_55602212](https://github.com/user-attachments/assets/cd89a358-ec8f-41a7-9344-0d02a5c478f6)

C. Menghentikan proses daemon

![WhatsApp Image 2025-04-18 at 11 56 24_de4cffbb](https://github.com/user-attachments/assets/c4981069-37ae-47f2-a9bf-19d0e4de661c)

![WhatsApp Image 2025-04-18 at 11 57 37_81245656](https://github.com/user-attachments/assets/62ecf708-179c-415c-9d28-940eaf6c3d8b)

E. User terblock dan tidak bisa melakukan apa apa
![WhatsApp Image 2025-04-18 at 11 58 23_dacd799e](https://github.com/user-attachments/assets/1a7c440a-0adb-4d23-83f1-12369646c07f)
![WhatsApp Image 2025-04-18 at 11 59 28_8af92f65](https://github.com/user-attachments/assets/96431001-b4ed-483d-a847-054ba2effc8f)

D. Command ini untuk mengembalikan user agar tidak terblock dan bisa menggunakan terminal kembali
![WhatsApp Image 2025-04-18 at 12 02 20_1afee52a](https://github.com/user-attachments/assets/e8c53981-6053-45a7-b33b-a8f0cd202ddb)

 Mencatat ke dalam file log
 
![WhatsApp Image 2025-04-18 at 12 03 24_b7bbb8de](https://github.com/user-attachments/assets/1b59f77d-5e81-49b7-a85e-60521d795046)

![WhatsApp Image 2025-04-18 at 12 03 52_47227615](https://github.com/user-attachments/assets/075f3cce-d133-47b0-ab68-97d709541572)




