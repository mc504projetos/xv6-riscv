#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define NUM_ROUNDS 30
#define MAX_LINE_LENGTH 100
#define LINE_BUFFER_SIZE 128

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

MemFS parse_and_calculate_metrics(int fd, int cpu_count, int io_count) {
    char line[LINE_BUFFER_SIZE];
    int line_index = 0;

    int mem_alloc_time = 0, mem_access_time = 0, mem_free_time = 0;
    int io_write_time = 0, io_read_time = 0, io_delete_time = 0;

    // Read file line by line
    int line_pos = 0;
    char c;

    while (read(fd, &c, 1) > 0) {
        if (c == '\n' || line_pos == LINE_BUFFER_SIZE - 1) {
            line[line_pos] = '\0'; // Null-terminate the line

            // Parse integers from the line
            int values[3] = {0};
            char *token = line;
            for (int i = 0; i < 3; i++) {
                values[i] = atoi(token); // Convert token to integer
                token = strchr(token, ' '); // Find next token
                if (token) token++; // Move past the space
                else break;
            }

            // Categorize and accumulate metrics
            if (line_index < cpu_count) {
                mem_alloc_time += values[0];
                mem_access_time += values[1];
                mem_free_time += values[2];
            } else if (line_index < cpu_count + io_count) {
                io_write_time += values[0];
                io_read_time += values[1];
                io_delete_time += values[2];
            }

            // Move to the next line
            line_index++;
            line_pos = 0; // Reset for the next line
        } else {
            line[line_pos++] = c; // Append character to the current line buffer
        }
    }

    // Populate the MemFS struct with collected metrics
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

    // Create file
    int fd = open("raw_data.txt", O_RDWR | O_CREATE | O_APPEND);

    // Fork CPU-bound processes
    for (int i = 0; i < cpu_count; i++) {
        int proc_start_time = uptime();
        pid = fork();
        if (pid == 0) {
            // Pass the file descriptor to the child process
            close(1);  // Close the standard output if not used
            dup(fd);   // Duplicate the file descriptor for writing
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
            // Pass the file descriptor to the child process
            close(1);  // Close the standard output if not used
            dup(fd);   // Duplicate the file descriptor for writing
            exec("io_bound", 0);
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

    MemFS t = parse_and_calculate_metrics(fd, cpu_count, io_count);

    // Close and delete file
    close(fd);
    unlink("raw_data.txt");

    Metrics metrics = {0};

    // Throughput
    if (total_time > 0) {
        metrics.throughput = (processes_completed / total_time);
        metrics.throughput *= 1000; 
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
