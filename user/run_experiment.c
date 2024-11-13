// run_experiment.c - Experiment runner with basic metrics for xv6
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NUM_ROUNDS 30

typedef struct {
    int throughput;
    int io_latency;
    int memory_overhead;
    int system_perf;
} Metrics;

Metrics collect_metrics(int cpu_count, int io_count) {
    Metrics metrics;
    int start_time = uptime();
    int processes_completed = 0;
    int pid;
    int status;

    // Fork CPU-bound processes
    for (int i = 0; i < cpu_count; i++) {
        pid = fork();
        if (pid == 0) {
            exec("cpu_bound", 0);
            exit(0);
        }
    }

    // Fork IO-bound processes
    for (int i = 0; i < io_count; i++) {
        pid = fork();
        if (pid == 0) {
            exec("io_bound", 0);
            exit(0);
        }
    }

    // Wait for processes and measure performance
    int total_latency = 0;
    for (int i = 0; i < cpu_count + io_count; i++) {
        wait(&status);
        processes_completed++;
    }
    int end_time = uptime();

    // Metrics calculations
    metrics.throughput = (processes_completed * 100) / (end_time - start_time);
    metrics.io_latency = total_latency / io_count;  // Simulated average latency
    metrics.memory_overhead = (end_time - start_time) / (cpu_count + io_count);
    metrics.system_perf = (metrics.throughput + metrics.io_latency + metrics.memory_overhead) / 3;

    return metrics;
}

// Define the seed for our simple pseudo-random number generator
static unsigned int seed = 1;

// Set the seed for random number generation
void srand(unsigned int s) {
    seed = s;
}

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

        printf("Round %d Results:\n", round + 1);
        printf("  Throughput: %d\n", metrics.throughput);
        printf("  IO Latency: %d\n", metrics.io_latency);
        printf("  Memory Overhead: %d\n", metrics.memory_overhead);
        printf("  System Performance: %d\n", metrics.system_perf);
    }
}

int main() {
    run_experiment();
    exit(0);
}
