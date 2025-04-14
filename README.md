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
## Soal-2
## Soal-3
## Soal-4
A. Menampilkan daftar semua proses yang sedang berjalan pada user tersebut beserta PID, command, CPU usage, dan memory usage
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
C. Menghentikan pengawasan
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
F. Mencatat ke dalam file log
```
void log_process(...) { fprintf(... "_RUNNING"); }
void log_failed_process(...) { fprintf(... "_FAILED"); }
```





