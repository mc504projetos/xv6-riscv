#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NUM_ROUNDS 30

typedef struct {
    int throughput;
    int process_justice;
    int fs_efficiency;
    int memory_overhead;
    int system_performance;
} Metrics;

Metrics collect_metrics(int cpu_count, int io_count) {
    Metrics metrics;
    int start_time = uptime();
    int end_time;
    int processes_completed = 0;
    int pid; int status;

    int total_exec_time = 0, sum_exec_time_sq = 0;
    int io_write_time = 0, io_read_time = 0, io_delete_time = 0;
    int memory_access_time = 0, memory_alloc_time = 0, memory_free_time = 0;

    // Fork CPU-bound processes
    for (int i = 0; i < cpu_count; i++) {
        int proc_start_time = uptime();
        pid = fork();
        if (pid == 0) {
            exec("cpu_bound", 0);
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
            exit(0);
        } else if (pid > 0) {
            wait(&status);
            int proc_end_time = uptime();
            int exec_time = proc_end_time - proc_start_time;
            total_exec_time += exec_time;
            sum_exec_time_sq += exec_time * exec_time;

            // Simulated IO times for filesystem efficiency
            io_write_time += 2;
            io_read_time += 1;
            io_delete_time += 1;

            processes_completed++;
        }
    }

    end_time = uptime();
    int total_time = end_time - start_time;

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
    int total_fs_time = io_write_time + io_read_time + io_delete_time;
    if (total_fs_time > 0) {
        metrics.fs_efficiency = 1 / total_fs_time;
        metrics.fs_efficiency *= 1000;
    }

    // Memory Overhead
    int total_memory_time = memory_access_time + memory_alloc_time + memory_free_time;
    if (total_memory_time > 0) {
        metrics.memory_overhead = 1 / total_memory_time;
        metrics.memory_overhead *= 1000;
    }

    // System Performance
    metrics.system_performance = 
        0.25 * metrics.throughput + 
        0.25 * metrics.process_justice + 
        0.25 * metrics.fs_efficiency + 
        0.25 * metrics.memory_overhead;
    metrics.system_performance *= 1000;

    printf("%d\n", metrics.memory_overhead);

    return metrics;
}

// Simple pseudo-random number generator
static unsigned int seed = 2;
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
