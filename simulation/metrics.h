#ifndef METRICS_H
#define METRICS_H

long measure_io_latency(char *filename);
double measure_throughput(char *filename);
long measure_justice();
long measure_fs_efficiency(char *filename);
long measure_mem_overhead();
long measure_system_perf();

#endif
