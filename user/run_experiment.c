#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NUM_ROUNDS 30

typedef struct {
    float throughput;
    float process_justice;
    float fs_efficiency;
    float memory_overhead;
    float system_performance;
} Metrics;

Metrics collect_metrics(int cpu_count, int io_count) {
    Metrics metrics = {0};
    int start_time = uptime();
    int end_time;
    int processes_completed = 0;

    int total_exec_time = 0, sum_exec_time_sq = 0;
    int io_write_time = 0, io_read_time = 0, io_delete_time = 0;
    int memory_access_time = 0, memory_alloc_time = 0, memory_free_time = 0;

    for (int i = 0; i < cpu_count + io_count; i++) {
        if (fork() == 0) {
            char *argv[] = {i < cpu_count ? "cpu_bound" : "io_bound", 0};
            exec(argv[0], argv);
            printf("Error: exec(%s) failed\n", argv[0]);
            exit(1);
        }
    }

    for (int i = 0; i < cpu_count + io_count; i++) {
        int proc_start_time = uptime();
        wait(0);
        int proc_end_time = uptime();
        int exec_time = proc_end_time - proc_start_time;

        total_exec_time += exec_time;
        sum_exec_time_sq += exec_time * exec_time;
        processes_completed++;

        if (i >= cpu_count) {
            io_write_time += 2;  // Simulated times
            io_read_time += 1;
            io_delete_time += 1;
        }
    }

    end_time = uptime();
    int total_time = end_time - start_time;

    // Throughput
    if (total_time > 0) {
        metrics.throughput = (float)processes_completed / total_time;
    }

    // Process Justice
    if (processes_completed > 0 && sum_exec_time_sq > 0) {
        metrics.process_justice = 
            ((float)(total_exec_time * total_exec_time)) /
            (processes_completed * sum_exec_time_sq);
    }

    // Filesystem Efficiency
    int total_fs_time = io_write_time + io_read_time + io_delete_time;
    if (total_fs_time > 0) {
        metrics.fs_efficiency = 1.0 / total_fs_time;
    }

    // Memory Overhead
    int total_memory_time = memory_access_time + memory_alloc_time + memory_free_time;
    if (total_memory_time > 0) {
        metrics.memory_overhead = 1.0 / total_memory_time;
    }

    // System Performance
    metrics.system_performance = 
        0.25 * metrics.throughput + 
        0.25 * metrics.process_justice + 
        0.25 * metrics.fs_efficiency + 
        0.25 * metrics.memory_overhead;

    return metrics;
}

static unsigned int seed = 2;

// Simple pseudo-random number generator
int rand() {
    seed = seed * 1664525 + 1013904223;
    return (seed & 0x7FFFFFFF);  // Return a non-negative integer
}

void run_experiment() {
    for (int round = 0; round < NUM_ROUNDS; round++) {
        int cpu_count = 6 + rand() % 9;
        int io_count = 20 - cpu_count;

        Metrics metrics = collect_metrics(cpu_count, io_count);

        printf("Round %d: CPU-bound=%d, IO-bound=%d\n", round + 1, cpu_count, io_count);
        printf("========= Metrics ========\n");
        printf("Throughput: %.2f\n", metrics.throughput);
        printf("Process Justice: %.2f\n", metrics.process_justice);
        printf("Filesystem Efficiency: %.2f\n", metrics.fs_efficiency);
        printf("Memory Overhead: %.2f\n", metrics.memory_overhead);
        printf("System Performance: %.2f\n", metrics.system_performance);
        printf("\n");
    }
}

int main() {
    run_experiment();
    exit(0);
}
