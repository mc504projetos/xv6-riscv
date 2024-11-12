// cpu_bound.c - CPU-bound simulation for xv6
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define N_VERTICES 100
#define INFINITY 9999

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

void initialize_graph(int graph[N_VERTICES][N_VERTICES]) {
    for (int i = 0; i < N_VERTICES; i++) {
        for (int j = 0; j < N_VERTICES; j++) {
            graph[i][j] = (i != j) ? (rand() % 20 + 1) : INFINITY;
        }
    }
}

void dijkstra(int graph[N_VERTICES][N_VERTICES], int start) {
    int dist[N_VERTICES];
    int visited[N_VERTICES] = {0};

    for (int i = 0; i < N_VERTICES; i++) dist[i] = INFINITY;
    dist[start] = 0;

    for (int i = 0; i < N_VERTICES - 1; i++) {
        int min = INFINITY, u = -1;
        for (int j = 0; j < N_VERTICES; j++)
            if (!visited[j] && dist[j] < min) min = dist[j], u = j;

        if (u == -1) break;
        visited[u] = 1;

        for (int v = 0; v < N_VERTICES; v++)
            if (!visited[v] && graph[u][v] != INFINITY && dist[u] + graph[u][v] < dist[v])
                dist[v] = dist[u] + graph[u][v];
    }
}

int main() {
    int graph[N_VERTICES][N_VERTICES];
    initialize_graph(graph);
    dijkstra(graph, 0);  // Run Dijkstra's from node 0
    exit(0);
}
