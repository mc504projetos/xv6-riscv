#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define LINE_COUNT 100
#define LINE_LENGTH 100

static unsigned int seed = 2;

// Simple pseudo-random number generator
int rand() {
    seed = seed * 1664525 + 1013904223;
    return (seed & 0x7FFFFFFF);  // Return a non-negative integer
}

// Generate a line of random characters
void generate_random_line(char *line) {
    for (int i = 0; i < LINE_LENGTH - 1; i++) {
        line[i] = 'A' + (rand() % 26);
    }
    line[LINE_LENGTH - 1] = '\n';
}

// Simulate reading a line at a specific position
void read_line(int fd, int index, char *buffer) {
    char temp[LINE_LENGTH];
    int target = index * LINE_LENGTH;
    int bytes_read = 0;

    while (bytes_read < target) {
        read(fd, temp, LINE_LENGTH);  // Skip lines until reaching the target
        bytes_read += LINE_LENGTH;
    }

    read(fd, buffer, LINE_LENGTH);  // Read the target line
}

// Simulate writing a line at a specific position
void write_line(int fd, int index, const char *buffer) {
    char temp[LINE_LENGTH];
    int target = index * LINE_LENGTH;
    int bytes_written = 0;

    while (bytes_written < target) {
        write(fd, temp, LINE_LENGTH);  // Skip lines until reaching the target
        bytes_written += LINE_LENGTH;
    }

    write(fd, buffer, LINE_LENGTH);  // Write the target line
}

// Shuffle lines in the file
void shuffle_lines(const char *filename) {
    int fd_read = open(filename, O_RDONLY);
    int fd_write = open(filename, O_RDWR);
    if (fd_read < 0 || fd_write < 0) {
        printf("Error: Could not open file for shuffling\n");
        return;
    }

    char buffer1[LINE_LENGTH], buffer2[LINE_LENGTH];
    int indices[LINE_COUNT];
    for (int i = 0; i < LINE_COUNT; i++) indices[i] = i;

    // Shuffle indices using a simple shuffle algorithm
    for (int i = LINE_COUNT - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    // Swap pairs of lines based on shuffled indices
    for (int i = 0; i < LINE_COUNT / 2; i++) {
        int pos1 = indices[i];
        int pos2 = indices[LINE_COUNT - i - 1];

        read_line(fd_read, pos1, buffer1);
        read_line(fd_read, pos2, buffer2);

        write_line(fd_write, pos1, buffer2);
        write_line(fd_write, pos2, buffer1);
    }

    close(fd_read);
    close(fd_write);
}

int main() {
    const char *filename = "tempfile.txt";
    int fd = open(filename, O_CREATE | O_WRONLY);
    if (fd < 0) {
        printf("Error: Could not open file for writing\n");
        exit(0);
    }

    // Generate and write random lines
    char line[LINE_LENGTH];
    for (int i = 0; i < LINE_COUNT; i++) {
        generate_random_line(line);
        write(fd, line, LINE_LENGTH);
    }
    close(fd);

    // Shuffle lines and delete the file
    shuffle_lines(filename);
    unlink(filename);

    exit(0);
}
