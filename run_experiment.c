// run_experiment.c - Experiment runner with basic metrics for xv6
#include "types.h"
#include "stat.h"
#include "user.h"

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

void run_experiment() {
    for (int round = 0; round < NUM_ROUNDS; round++) {
        int cpu_count = 6 + random() % 9;
        int io_count = 20 - cpu_count;

        printf(1, "Starting round %d: CPU-bound=%d, IO-bound=%d\n", round + 1, cpu_count, io_count);

        Metrics metrics = collect_metrics(cpu_count, io_count);

        printf(1, "Round %d Results:\n", round + 1);
        printf(1, "  Throughput: %d\n", metrics.throughput);
        printf(1, "  IO Latency: %d\n", metrics.io_latency);
        printf(1, "  Memory Overhead: %d\n", metrics.memory_overhead);
        printf(1, "  System Performance: %d\n", metrics.system_perf);
    }
}

int main() {
    run_experiment();
    exit(0);
}
