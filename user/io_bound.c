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

// Shuffle lines in the file
void shuffle_lines(const char *filename) {
    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        printf("Error: Could not open file for shuffling\n");
        return;
    }

    char line1[LINE_LENGTH], line2[LINE_LENGTH];
    int indices[LINE_COUNT];

    // Initialize indices array
    for (int i = 0; i < LINE_COUNT; i++) {
        indices[i] = i;
    }

    // Shuffle indices using Fisher-Yates shuffle
    for (int i = LINE_COUNT - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    // Swap lines based on shuffled indices
    for (int i = 0; i < LINE_COUNT / 2; i++) {
        int pos1 = indices[i];
        int pos2 = indices[LINE_COUNT - i - 1];

        // Read first line
        read(fd, line1, LINE_LENGTH);
        // Skip to the next line (assuming lines are of fixed length)
        for (int j = 1; j < pos1; j++) read(fd, line1, LINE_LENGTH);

        // Read second line
        read(fd, line2, LINE_LENGTH);
        // Skip to the next line
        for (int j = 1; j < pos2; j++) read(fd, line2, LINE_LENGTH);

        // Write second line to the first line's position
        write(fd, line2, LINE_LENGTH);

        // Write first line to the second line's position
        write(fd, line1, LINE_LENGTH);
    }

    close(fd);
}

// void print_file_content(const char *filename) {
//     int fd = open(filename, O_RDONLY);
//     if (fd < 0) {
//         printf("Error: Could not open file %s\n", filename);
//         return;
//     }

//     char buf[512]; // Buffer size for reading chunks
//     int n;

//     while ((n = read(fd, buf, sizeof(buf))) > 0) {
//         write(1, buf, n); // Write to stdout (file descriptor 1)
//     }

//     if (n < 0) {
//         printf("Error: Failed to read file %s\n", filename);
//     }

//     printf("raw_data.txt foi impresso!\n");

//     close(fd);
// }


// Log metrics into raw_data.txt
void log_metrics(int write_time, int read_time, int delete_time) {
    printf("%d %d %d\n", write_time, read_time, delete_time);
}

int main() {
    const char *filename = "tempfile.txt";
    char line[LINE_LENGTH];

    // Write lines to file
    int write_time = uptime();
    int fd = open(filename, O_CREATE | O_WRONLY | O_APPEND);  // Open with O_APPEND to append to the end of the file
    for (int i = 0; i < LINE_COUNT; i++) {
        generate_random_line(line);
        write(fd, line, LINE_LENGTH);
    }
    close(fd);
    write_time = uptime() - write_time;

    // Read lines from file
    int read_time = uptime();
    fd = open(filename, O_RDONLY);
    for (int i = 0; i < LINE_COUNT; i++) {
        read(fd, line, LINE_LENGTH);
    }
    close(fd);
    read_time = uptime() - read_time;

    // Shuffle lines in the file
    int shuffle_time = uptime();
    shuffle_lines(filename);
    shuffle_time = uptime() - shuffle_time;

    // Delete the file
    int delete_time = uptime();
    unlink(filename);
    delete_time = uptime() - delete_time;

    // Log the metrics
    log_metrics(write_time, read_time + shuffle_time, delete_time);

    exit(0);
}
