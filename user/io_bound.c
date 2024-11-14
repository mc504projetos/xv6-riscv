// io_bound.c - IO-bound simulation for xv6
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define LINE_COUNT 100
#define LINE_LENGTH 100

// Define the seed for our simple pseudo-random number generator
static unsigned int seed = 2;

// Generate a pseudo-random number
int rand() {
    seed = seed * 1664525 + 1013904223;
    return (seed & 0x7FFFFFFF);  // Return a non-negative integer
}

void generate_random_line(char *line) {
    for (int i = 0; i < LINE_LENGTH - 1; i++) {
        line[i] = 'A' + (rand() % 26);
    }
    line[LINE_LENGTH - 1] = '\n';
}

int main() {
    int fd = open("tempfile.txt", O_CREATE | O_WRONLY);
    if (fd < 0) {
        printf("Error: Could not open file for writing\n");
        exit(0);
    }

    // Write random lines
    char line[LINE_LENGTH];
    for (int i = 0; i < LINE_COUNT; i++) {
        generate_random_line(line);
        write(fd, line, LINE_LENGTH);
    }
    close(fd);

    // Delete file
    unlink("tempfile.txt");
    exit(0);
}
