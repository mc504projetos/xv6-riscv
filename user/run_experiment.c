// run_experiment.c
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

Metrics parse_and_calculate_metrics() {
    int fd = open("raw_data.txt", O_RDONLY);
    if (fd < 0) {
        printf("Error: raw_data.txt not found\n");
        exit(0);
    }

    char buf[MAX_LINE_LENGTH];
    int cpu_alloc_time = 0, cpu_access_time = 0, cpu_free_time = 0;
    int io_write_time = 0, io_read_time = 0, io_delete_time = 0;
    int cpu_processes = 0, io_processes = 0;

    while (read(fd, buf, sizeof(buf)) > 0) {
        if (strstr(buf, "cpu_bound")) {
            int alloc, access, free;
            sscanf(buf, "%d ticks %d ticks %d ticks (cpu_bound)", &alloc, &access, &free);
            cpu_alloc_time += alloc;
            cpu_access_time += access;
            cpu_free_time += free;
        } else if (strstr(buf, "io_bound")) {
            int write, read, delete;
            sscanf(buf, "%d ticks %d ticks %d ticks (io_bound)", &write, &read, &delete);
            io_write_time += write;
            io_read_time += read;
            io_delete_time += delete;
        }
    }

    close(fd);

    int total_processes = 20;
    int total_cpu_time = cpu_alloc_time + cpu_access_time + cpu_free_time;
    int total_io_time = io_write_time + io_read_time + io_delete_time;
    int total_time = total_cpu_time + total_io_time;

    Metrics metrics;

    // Throughput
    if (total_time > 0) {
        metrics.throughput = (total_processes * 1000) / total_time;
    }

    // Process Justice
    if (cpu_processes > 0) {
        metrics.process_justice =
            (total_cpu_time * total_cpu_time * 1000) /
            (cpu_processes * (cpu_alloc_time + cpu_access_time + cpu_free_time));
    }

    // Filesystem Efficiency
    if (total_io_time > 0) {
        metrics.fs_efficiency = 1000 / total_io_time;
    }

    // Memory Overhead
    if (total_cpu_time > 0) {
        metrics.memory_overhead = 1000 / total_cpu_time;
    }

    // System Performance
    metrics.system_performance =
        (metrics.throughput + metrics.process_justice +
         metrics.fs_efficiency + metrics.memory_overhead) / 4;

    return metrics;
}

Metrics collect_metrics(int cpu_count, int io_count) {
    int start_time = uptime();
    int processes_completed = 0;
    int pid, status;

    // Fork CPU-bound processes
    for (int i = 0; i < cpu_count; i++) {
        pid = fork();
        if (pid == 0) {
            exec("cpu_bound", 0);
            exit(0);
        } else if (pid > 0) {
            wait(&status);
            processes_completed++;
        }
    }

    // Fork IO-bound processes
    for (int i = 0; i < io_count; i++) {
        pid = fork();
        if (pid == 0) {
            exec("io_bound", 0);
            exit(0);
        } else if (pid > 0) {
            wait(&status);
            processes_completed++;
        }
    }

    int end_time = uptime();
    int total_time = end_time - start_time;

    Metrics metrics = parse_and_calculate_metrics();

    printf("Processes Completed: %d\n", processes_completed);
    printf("Total Execution Time: %d ticks\n", total_time);

    return metrics;
}

void run_experiment() {
    for (int round = 1; round <= NUM_ROUNDS; round++) {
        int cpu_count = 6 + rand() % 9;
        int io_count = 20 - cpu_count;

        printf("Round %d: CPU-bound=%d, IO-bound=%d\n", round, cpu_count, io_count);

        Metrics metrics = collect_metrics(cpu_count, io_count);

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
