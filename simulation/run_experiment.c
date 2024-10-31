#include "metrics.h"
#include "types.h"
#include "user.h"
#include "fcntl.h"

void perform_experiment(char *filename) {
    printf(1, "I/O Latency: %ld ticks\n", measure_io_latency(filename));
    printf(1, "Throughput: %.2f bytes/tick\n", measure_throughput(filename));
    printf(1, "Process Justice: %ld ticks\n", measure_justice());
    printf(1, "Filesystem Efficiency: %ld operations\n", measure_fs_efficiency(filename));
    printf(1, "Memory Management Overhead: %ld ticks\n", measure_mem_overhead());
    printf(1, "System Performance: %ld ticks\n", measure_system_perf());
}

int main() {
    perform_experiment("testfile.txt");
    exit(0);
}
