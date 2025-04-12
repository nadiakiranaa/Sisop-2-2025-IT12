#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>


#define PROC_DIR "/proc"
#define LOG_FILE "debugmon.log"

int fail_mode = 0;

uid_t get_uid_by_name(const char *username) {
    struct passwd *pw = getpwnam(username);
    if (!pw) {
        fprintf(stderr, "User '%s' not found.\n", username);
        exit(EXIT_FAILURE);
    }
    return pw->pw_uid;
}

int is_numeric(const char *str) {
    while (*str) {
        if (!isdigit(*str)) return 0;
        str++;
    }
    return 1;
}

void log_failed_process(const char *process_name) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "[%d:%m:%Y]-[%H:%M:%S]", t);

    fprintf(log, "%s_%s_FAILED\n", timestamp, process_name);
    fclose(log);
}

void log_process(const char *process_name) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "[%d:%m:%Y]-[%H:%M:%S]", t);

    fprintf(log, "%s_%s_RUNNING\n", timestamp, process_name);
    fclose(log);
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); 

    
    umask(0);
    setsid();

    if (chdir("/home/nadia/Sisop-2-2025-IT12/soal_4") < 0) exit(EXIT_FAILURE);

    fclose(stdin);
    fclose(stdout);
    fclose(stderr);

    open("/dev/null", O_RDONLY);  
    open("/dev/null", O_WRONLY); 
    open("/dev/null", O_RDWR);   
}


void monitor_user_processes_daemon(const char *username, int fail_mode) {
    uid_t uid = get_uid_by_name(username);

    while (1) {
        DIR *proc = opendir(PROC_DIR);
        if (!proc) {
            sleep(5);
            continue;
        }

        struct dirent *entry;
        while ((entry = readdir(proc)) != NULL) {
            if (!is_numeric(entry->d_name)) continue;

            int pid = atoi(entry->d_name);
            char path[256], line[256];
            FILE *fp;
            uid_t proc_uid;
            char comm[256] = "-";

            
            snprintf(path, sizeof(path), PROC_DIR"/%d/status", pid);
            fp = fopen(path, "r");
            if (!fp) continue;

            while (fgets(line, sizeof(line), fp)) {
                if (sscanf(line, "Uid: %d", &proc_uid) == 1) break;
            }
            fclose(fp);
            if (proc_uid != uid) continue;

            
            snprintf(path, sizeof(path), PROC_DIR"/%d/comm", pid);
            fp = fopen(path, "r");
            if (fp) {
                fgets(comm, sizeof(comm), fp);
                comm[strcspn(comm, "\n")] = 0;
                fclose(fp);
            }

            if (strcmp(comm, "debugmon") == 0) {
                log_process(comm);
            } else if (fail_mode) {
                
                if (kill(pid, SIGKILL) == 0) {
                    log_failed_process(comm);
                }
            }
        }

        closedir(proc);
        sleep(5);
    }
}


void stop_debugmon_daemon_only(const char *username) {
    DIR *proc = opendir(PROC_DIR);
    if (!proc) {
        perror("opendir /proc");
        return;
    }

    struct dirent *entry;
    uid_t uid = get_uid_by_name(username);

    while ((entry = readdir(proc)) != NULL) {
        if (!is_numeric(entry->d_name)) continue;

        int pid = atoi(entry->d_name);
        char path[256], line[1024], exe_path[256];
        uid_t proc_uid;

        
        snprintf(path, sizeof(path), PROC_DIR"/%d/status", pid);
        FILE *fp = fopen(path, "r");
        if (!fp) continue;

        while (fgets(line, sizeof(line), fp)) {
            if (sscanf(line, "Uid: %d", &proc_uid) == 1) break;
        }
        fclose(fp);
        if (proc_uid != uid) continue;

        
        snprintf(exe_path, sizeof(exe_path), PROC_DIR"/%d/exe", pid);
        char actual_path[256];
        ssize_t len = readlink(exe_path, actual_path, sizeof(actual_path) - 1);
        if (len == -1) continue;
        actual_path[len] = '\0';

        if (!strstr(actual_path, "debugmon")) continue;

        
        snprintf(path, sizeof(path), PROC_DIR"/%d/cmdline", pid);
        fp = fopen(path, "r");
        if (!fp) continue;
        size_t read_bytes = fread(line, 1, sizeof(line) - 1, fp);
        fclose(fp);

        line[read_bytes] = '\0';

        
        char *args[10];
        int argc = 0;
        char *ptr = line;
        while (ptr < line + read_bytes && argc < 10) {
            args[argc++] = ptr;
            ptr += strlen(ptr) + 1;
        }

       
        if (argc >= 3 &&
            strstr(args[0], "debugmon") &&
           
            strcmp(args[2], username) == 0) {

            if (kill(pid, SIGTERM) == 0) {
                printf("Berhasil menghentikan daemon debugmon (PID %d) untuk user '%s'\n", pid, username);
            } else {
                perror("Gagal menghentikan proses");
            }
        }
    }

    closedir(proc);
}


int main(int argc, char *argv[]) {
    if (argc == 3 && strcmp(argv[1], "list") == 0) {
        uid_t uid = get_uid_by_name(argv[2]);
        DIR *proc = opendir(PROC_DIR);
        if (!proc) {
            perror("opendir /proc");
            return EXIT_FAILURE;
        }

        struct dirent *entry;
        while ((entry = readdir(proc)) != NULL) {
            if (is_numeric(entry->d_name)) {
                int pid = atoi(entry->d_name);
                char path[256], line[256];
                FILE *fp;
                uid_t proc_uid;
                char comm[256] = "-";
                double mem_usage = 0;

                snprintf(path, sizeof(path), PROC_DIR"/%d/status", pid);
                fp = fopen(path, "r");
                if (!fp) continue;

                while (fgets(line, sizeof(line), fp)) {
                    if (sscanf(line, "Uid: %d", &proc_uid) == 1) {
                        if (proc_uid != uid) {
                            fclose(fp);
                            goto next;
                        }
                    }
                }
                fclose(fp);

                snprintf(path, sizeof(path), PROC_DIR"/%d/comm", pid);
                fp = fopen(path, "r");
                if (fp) {
                    fgets(comm, sizeof(comm), fp);
                    comm[strcspn(comm, "\n")] = 0;
                    fclose(fp);
                }

                snprintf(path, sizeof(path), PROC_DIR"/%d/statm", pid);
                fp = fopen(path, "r");
                if (fp) {
                    long pages;
                    if (fscanf(fp, "%ld", &pages) == 1) {
                        long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024;
                        mem_usage = pages * page_size_kb;
                    }
                    fclose(fp);
                }

                printf("PID: %-6d CMD: %-20s CPU: %.2f%% MEM: %.2f KB\n", pid, comm, 0.0, mem_usage);
            }
        next:;
        }

        closedir(proc);

    } else if (argc == 3 && strcmp(argv[1], "daemon") == 0) {
        daemonize();
        monitor_user_processes_daemon(argv[2],0);
    } else if (argc == 3 && strcmp(argv[1], "stop") == 0) {
        stop_debugmon_daemon_only(argv[2]);
    } else if (argc == 3 && strcmp(argv[1], "fail") == 0) {
        daemonize();
        monitor_user_processes_daemon(argv[2], 1); 

    } else if (argc == 3 && strcmp(argv[1], "revert") == 0) {
        stop_debugmon_daemon_only(argv[2]);  
    } else {
        printf("Usage:\n");
        printf("  %s list <user>\n", argv[0]);
        printf("  %s daemon <user>\n", argv[0]);
        printf("  %s stop <user>\n", argv[0]);
        printf("  %s fail <user>\n", argv[0]);
        printf("  %s revert <user>\n", argv[0]);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


