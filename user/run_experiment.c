#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define NUM_ROUNDS 30
#define MAX_LINE_LENGTH 100

typedef struct {
    int throughput;
    int process_justice;
    int fs_efficiency;
    int memory_overhead;
    int system_performance;
} Metrics;

typedef struct {
    int memory_access_time;
    int memory_alloc_time;
    int memory_free_time;
    int io_write_time;
    int io_read_time;
    int io_delete_time;
} MemFS;

MemFS parse_and_calculate_metrics() {
    int fd = open("raw_data.txt", O_RDONLY);
    if (fd < 0) {
        printf("Error: raw_data.txt not found\n");
        exit(0);
    }

    char buf[MAX_LINE_LENGTH];
    int mem_alloc_time = 0, mem_access_time = 0, mem_free_time = 0;
    int io_write_time = 0, io_read_time = 0, io_delete_time = 0;

    while (read(fd, buf, sizeof(buf)) > 0) {
        if (strchr(buf, 'c') && strchr(buf, 'p')) {
            // Extracting time for CPU-bound process
            int alloc, access, free;
            alloc = atoi(buf);
            access = atoi(strchr(buf, ' ') + 1);
            free = atoi(strchr(strchr(buf, ' ') + 1, ' ') + 1);
            mem_alloc_time += alloc;
            mem_access_time += access;
            mem_free_time += free;
        } else if (strchr(buf, 'i') && strchr(buf, 'o')) {
            // Extracting time for IO-bound process
            int write, read, delete;
            write = atoi(buf);
            read = atoi(strchr(buf, ' ') + 1);
            delete = atoi(strchr(strchr(buf, ' ') + 1, ' ') + 1);
            io_write_time += write;
            io_read_time += read;
            io_delete_time += delete;
        }
    }

    close(fd);
    
    const char *filename = "raw_data.txt";
    unlink(filename);

    MemFS time_values;

    time_values.memory_alloc_time = mem_alloc_time;
    time_values.memory_access_time = mem_access_time;
    time_values.memory_free_time = mem_free_time;

    time_values.io_write_time = io_write_time;
    time_values.io_read_time = io_read_time;
    time_values.io_delete_time = io_delete_time;

    return time_values;
}

Metrics collect_metrics(int cpu_count, int io_count) {
    int pid, status;
    int processes_completed = 0;
    int total_exec_time = 0, sum_exec_time_sq = 0;
    int start_time = uptime();

    // Fork CPU-bound processes
    for (int i = 0; i < cpu_count; i++) {
        int proc_start_time = uptime();
        pid = fork();
        if (pid == 0) {
            exec("cpu_bound", 0);
            // printf("cpu ps finished!\n");
            exit(0);
        } else if (pid > 0) {
            wait(&status);
            int proc_end_time = uptime();
            int exec_time = proc_end_time - proc_start_time;
            total_exec_time += exec_time;
            sum_exec_time_sq += exec_time * exec_time;
            processes_completed++;
        }
    }

    // Fork IO-bound processes
    for (int i = 0; i < io_count; i++) {
        int proc_start_time = uptime();
        pid = fork();
        if (pid == 0) {
            exec("io_bound", 0);
            // printf("io ps finished!\n");
            exit(0);
        } else if (pid > 0) {
            wait(&status);
            int proc_end_time = uptime();
            int exec_time = proc_end_time - proc_start_time;
            total_exec_time += exec_time;
            sum_exec_time_sq += exec_time * exec_time;
            processes_completed++;
        }
    }

    int end_time = uptime();
    int total_time = end_time - start_time;

    MemFS t = parse_and_calculate_metrics();
    Metrics metrics = {0};

    // Throughput
    if (total_time > 0) {
        metrics.throughput = (processes_completed / total_time);
        metrics.throughput *= 1000; // check
    }

    // Process Justice
    if (processes_completed > 0 && sum_exec_time_sq > 0) {
        metrics.process_justice = 
            (total_exec_time * total_exec_time) /
            (processes_completed * sum_exec_time_sq);
        metrics.process_justice *= 1000; 
    }

    // Filesystem Efficiency
    int total_fs_time = t.io_write_time + t.io_read_time + t.io_delete_time;
    if (total_fs_time > 0) {
        metrics.fs_efficiency = 1 / total_fs_time;
        metrics.fs_efficiency *= 1000;
    }

    // Memory Overhead
    int total_memory_time = t.memory_access_time + t.memory_alloc_time + t.memory_free_time;
    if (total_memory_time > 0) {
        metrics.memory_overhead = 1 / total_memory_time;
        metrics.memory_overhead *= 1000;
    }

    // System Performance
    metrics.system_performance = (metrics.throughput + metrics.process_justice
     + metrics.fs_efficiency + metrics.memory_overhead) / 4;
    metrics.system_performance *= 1000;

    // printf("Processes Completed: %d\n", processes_completed);
    // printf("Total Execution Time: %d ticks\n", total_time);



    return metrics;
}

// Simple pseudo-random number generator
static unsigned int seed = 3;
int rand() {
    seed = seed * 1664525 + 1013904223;
    return (seed & 0x7FFFFFFF);  // Return a non-negative integer
}

void run_experiment() {
    for (int round = 1; round <= NUM_ROUNDS; round++) {
        int cpu_count = 6 + rand() % 9;
        int io_count = 20 - cpu_count;

        Metrics metrics = collect_metrics(cpu_count, io_count);

        printf("Round %d: CPU-bound=%d, IO-bound=%d\n", round, cpu_count, io_count);
        printf("========= Metrics (x1000) =========\n");
        printf("Throughput: %d\n", metrics.throughput);
        printf("Process Justice: %d\n", metrics.process_justice);
        printf("Filesystem Efficiency: %d\n", metrics.fs_efficiency);
        printf("Memory Overhead: %d\n", metrics.memory_overhead);
        printf("System Performance: %d\n", metrics.system_performance);
        printf("\n");
    }
}

int main() {
    run_experiment();
    exit(0);
}
