// run_experiment.c - Experiment runner with basic metrics for xv6
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NUM_ROUNDS 30

// Define a simplified Metrics struct for essential data
typedef struct {
    float throughput;
    float process_justice;
    float fs_efficiency;
    float memory_overhead;
    float system_performance;
} Metrics;

Metrics collect_metrics(int cpu_count, int io_count) {
    Metrics metrics;
    int start_time = uptime();
    int end_time;
    int processes_completed = 0;
    int pid;
    int status;

    // Simulated values for operations
    int proc_start_time = 0;
    int proc_end_time = 0;
    int exec_time = 0;
    int total_exec_time = 0;
    int sum_exec_time_sq = 0;
    int io_write_time = 0, io_read_time = 0, io_delete_time = 0;
    int memory_access_time = 0, memory_alloc_time = 0, memory_free_time = 0;
    int min_throughput = 1, max_throughput = 100;

    // Fork CPU-bound processes
    for (int i = 0; i < cpu_count; i++) {
        proc_start_time = uptime();
        pid = fork();
        if (pid == 0) {
            char *argv[] = {"cpu_bound", 0};
            exec("cpu_bound", argv);
            exit(0);
        } else if (pid > 0) {
            wait(&status);
            proc_end_time = uptime();
            exec_time = proc_end_time - proc_start_time;
            total_exec_time += exec_time;
            sum_exec_time_sq += exec_time * exec_time;
            processes_completed++;
        }
    }

    // Fork IO-bound processes
    for (int i = 0; i < io_count; i++) {
        proc_start_time = uptime();
        pid = fork();
        if (pid == 0) {
            char *argv[] = {"io_bound", 0};
            exec("io_bound", argv);
            exit(0);
        } else if (pid > 0) {
            wait(&status);
            proc_end_time = uptime();
            exec_time = proc_end_time - proc_start_time;
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

    // Throughput calculation based on normalized throughput formula
    if (total_time > 0) {
        float avg_throughput = (float)processes_completed / total_time;
        metrics.throughput = 1 - (avg_throughput - min_throughput) / (max_throughput - min_throughput);
    } else 
        metrics.throughput = 0;

    // Process Justice (J_cpu) calculation
    int N = processes_completed;
    if (N > 0 && sum_exec_time_sq > 0) {
        int total_exec_time_sq = total_exec_time * total_exec_time;
        metrics.process_justice = (float)total_exec_time_sq / (N * sum_exec_time_sq);
    } else
        metrics.process_justice = 0;

    // Filesystem Efficiency (E_fs) calculation
    int total_fs_time = io_write_time + io_read_time + io_delete_time;
    if (total_fs_time > 0) {
        metrics.fs_efficiency = 1.0 / total_fs_time;
    } else
        metrics.fs_efficiency = 0;

    // Memory Overhead (M_over) calculation
    int total_memory_time = memory_access_time + memory_alloc_time + memory_free_time;
    if (total_memory_time > 0) {
        metrics.memory_overhead = 1.0 / total_memory_time;
    } else
        metrics.memory_overhead = 0;

    // System Performance (S_perfom) as the average of all four normalized metrics
    metrics.system_performance = (metrics.throughput + metrics.process_justice +
                                  metrics.fs_efficiency + metrics.memory_overhead) / 4;

    return metrics;
}

// Simple pseudo-random number generator
static unsigned int seed = 3;
int rand() {
    seed = seed * 1103515245 + 12345;
    return (seed & 0x7FFFFFFF);  // Return a non-negative integer
}

void run_experiment() {
    for (int round = 0; round < NUM_ROUNDS; round++) {
        int cpu_count = 6 + rand() % 9;
        int io_count = 20 - cpu_count;

        Metrics metrics = collect_metrics(cpu_count, io_count);

        // Display metrics for current round
        printf("Starting round %d: CPU-bound=%d, IO-bound=%d\n", round + 1, cpu_count, io_count);
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
