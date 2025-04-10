#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void write_log(const char *process_name, const char *status) {
    FILE *log = fopen("debugmon.log", "a");
    if (log == NULL) {
        perror("Gagal membuka debugmon.log");
        return;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(log, "[%02d:%02d:%04d]-[%02d:%02d:%02d]_%s_%s\n",
        tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,
        tm.tm_hour, tm.tm_min, tm.tm_sec,
        process_name, status);
    fclose(log);
}
void list_process(const char *user) {
    char command[128];
    snprintf(command, sizeof(command), "ps -u %s -o pid,comm,pcpu,pmem --no-headers", user);

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Gagal menjalankan ps");
        return;
    }

    printf("Daftar proses untuk user: %s\n", user);
    printf("PID\tCommand\t\tCPU\tMemory\n");
    char pid[10], comm[64], cpu[8], mem[8];
    while (fscanf(fp, "%s %s %s %s", pid, comm, cpu, mem) == 4) {
        printf("%s\t%-15s %s%%\t%s%%\n", pid, comm, cpu, mem);
        write_log(comm, "RUNNING");
    }

    pclose(fp);
}

void run_daemon(const char *user) {
    printf("Menjalankan Debugmon dalam mode daemon untuk user: %s\n", user);
    write_log("daemon_mode", "RUNNING");
}

void stop_monitoring(const char *user) {
    printf("Menghentikan pengawasan untuk user: %s\n", user);
    write_log("stop_monitoring", "RUNNING");
}

void fail_process(const char *user) {
    char command[128];
    snprintf(command, sizeof(command), "ps -u %s -o pid=,comm= --no-headers", user);
    
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Gagal menjalankan ps");
        return;
    }

    char line[256];
    printf("Menggagalkan semua proses untuk user: %s\n", user);
    while (fgets(line, sizeof(line), fp)) {
        int pid;
        char comm[64];

        if (sscanf(line, "%d %63s", &pid, comm) == 2) {
            // Cegah agar program ini sendiri tidak membunuh dirinya
            if (pid == getpid()) continue;

            if (kill(pid, SIGKILL) == 0) {
                printf("Proses %s (PID %d) dihentikan.\n", comm, pid);
                write_log(comm, "FAILED");
            } else {
                perror("Gagal membunuh proses");
            }
        }
    }

    pclose(fp);
}
void revert_process(const char *user) {
    printf("Mengizinkan kembali user %s untuk menjalankan proses.\n", user);
    write_log("revert_mode", "RUNNING");
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: ./debugmon <command> <user>\n");
        return 1;
    }

    char *command = argv[1];
    char *user = argv[2];

    if (strcmp(command, "list") == 0) {
        list_process(user);
    } else if (strcmp(command, "daemon") == 0) {
        run_daemon(user);
    } else if (strcmp(command, "stop") == 0) {
        stop_monitoring(user);
    } else if (strcmp(command, "fail") == 0) {
        fail_process(user);
    } else if (strcmp(command, "revert") == 0) {
        revert_process(user);
    } else {
        printf("Perintah tidak dikenal: %s\n", command);
    }

    return 0;
}
