#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define N_DIGRAPHS 1000
#define MAX_VERTICES 200
#define MIN_VERTICES 100
#define MAX_EDGES 400
#define MIN_EDGES 50
#define INFINITY 9999

static unsigned int seed = 1;

int rand() {
    seed = seed * 1664525 + 1013904223;
    return (seed & 0x7FFFFFFF);  // Return a non-negative integer
}

void initialize_graph(int **graph, int vertices, int edges) {
    for (int i = 0; i < vertices; i++) {
        for (int j = 0; j < vertices; j++) {
            graph[i][j] = (i != j) ? INFINITY : 0;
        }
    }
    for (int i = 0; i < edges; i++) {
        int u = rand() % vertices;
        int v = rand() % vertices;
        int weight = rand() % 20 + 1;
        graph[u][v] = weight;
    }
}

void dijkstra(int **graph, int vertices, int start) {
    int dist[vertices];
    int visited[vertices];
    for (int i = 0; i < vertices; i++) {
        dist[i] = INFINITY;
        visited[i] = 0;
    }
    dist[start] = 0;

    for (int i = 0; i < vertices - 1; i++) {
        int min = INFINITY, u = -1;
        for (int j = 0; j < vertices; j++)
            if (!visited[j] && dist[j] < min) min = dist[j], u = j;

        if (u == -1) break;
        visited[u] = 1;

        for (int v = 0; v < vertices; v++) {
            if (!visited[v] && graph[u][v] != INFINITY &&
                dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
            }
        }
    }
}

void log_metrics(int alloc_time, int access_time, int free_time) {
    const char *filename = "raw_data.txt";
    int fd = open(filename, O_RDWR | O_APPEND);  // Use O_APPEND flag to append to the file
    if (fd < 0) {
        fd = open(filename, O_CREATE | O_WRONLY | O_APPEND);  // Create and append if file doesn't exist
    }

    // Use fprintf to log formatted metrics to the file
    fprintf(fd, "%d %d %d\n", alloc_time, access_time, free_time);

    close(fd);
}

int main() {
    int alloc_time, access_time, free_time;
    alloc_time = access_time = free_time = 0;
    int aux = 0;

    for (int j = 0; j < N_DIGRAPHS; j++) {
        int vertices = MIN_VERTICES + rand() % (MAX_VERTICES - MIN_VERTICES + 1);
        int edges = MIN_EDGES + rand() % (MAX_EDGES - MIN_EDGES + 1);

        // Measure allocation time
        aux = uptime();
        int **graph = malloc(vertices * sizeof(int *));
        for (int i = 0; i < vertices; i++) {
            graph[i] = malloc(vertices * sizeof(int));
        }
        alloc_time += uptime() - aux;

        // Start graph
        initialize_graph(graph, vertices, edges);

        // Measure access time for Dijkstra
        aux = uptime();
        dijkstra(graph, vertices, 0);
        access_time += uptime() - aux;

        // Measure free time
        aux = uptime();
        for (int i = 0; i < vertices; i++) {
            free(graph[i]);
        }
        free(graph);
        free_time += uptime() - aux;

    }

    // Log metrics to file
    log_metrics(alloc_time, access_time, free_time);

    exit(0);
}
