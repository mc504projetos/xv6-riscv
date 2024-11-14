// run_experiment.c - Experiment runner with basic metrics for xv6
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NUM_ROUNDS 30

// Define a simplified Metrics struct for essential data
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
    int pid;
    int status;
    int total_exec_time = 0; // For fairness calculation
    int io_operations = 0;   // Counter for filesystem efficiency
    int memory_usage = 0;    // Simple counter for memory overhead

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
            total_exec_time += (proc_end_time - proc_start_time);
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
            total_exec_time += (proc_end_time - proc_start_time);
            io_operations += 2;  // Assume two I/O actions per IO-bound process
            processes_completed++;
        }
    }

    end_time = uptime();

    // Throughput: completed processes per unit time
    metrics.throughput = (processes_completed * 100) / (end_time - start_time);

    // Process Justice: simple variance based on execution time
    metrics.process_justice = total_exec_time / processes_completed;

    // Filesystem Efficiency: proxy based on number of I/O operations
    metrics.fs_efficiency = io_operations;

    // Memory Overhead: use total runtime as a simple approximation
    memory_usage = (end_time - start_time) / (cpu_count + io_count);
    metrics.memory_overhead = memory_usage;

    // Overall System Performance: simple average of normalized metrics
    metrics.system_performance = (metrics.throughput + metrics.process_justice + metrics.fs_efficiency + metrics.memory_overhead) / 4;

    return metrics;
}

// Define the seed for our simple pseudo-random number generator
static unsigned int seed = 3;

// Generate a pseudo-random number
int rand() {
    seed = seed * 1664525 + 1013904223;
    return (seed & 0x7FFFFFFF);  // Return a non-negative integer
}

void run_experiment() {
    for (int round = 0; round < NUM_ROUNDS; round++) {
        int cpu_count = 6 + rand() % 9;
        int io_count = 20 - cpu_count;

        printf("Starting round %d: CPU-bound=%d, IO-bound=%d\n", round + 1, cpu_count, io_count);

        Metrics metrics = collect_metrics(cpu_count, io_count);

        // Display metrics for current round
        printf("Metrics:\n");
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
