#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define FILENAME "random_file.txt"
#define LINE_COUNT 100
#define LINE_LENGTH 100
#define PERMUTATIONS 50

// Simple linear congruential generator for pseudo-random numbers
unsigned int next = 1;
int rand() {
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed) {
    next = seed;
}

// Generate a random uppercase character
char random_char() {
    return 'A' + (rand() % 26);
}

// Write 100 lines of 100 random characters to a file
void write_random_lines(int fd) {
    char line[LINE_LENGTH + 1];
    for (int i = 0; i < LINE_COUNT; i++) {
        for (int j = 0; j < LINE_LENGTH; j++) {
            line[j] = random_char();
        }
        line[LINE_LENGTH] = '\n';
        write(fd, line, LINE_LENGTH + 1);
    }
}

// Swap two lines in the file
void swap_lines(int fd, int line1, int line2) {
    char buffer1[LINE_LENGTH + 1], buffer2[LINE_LENGTH + 1];

    // Read first line
    int offset1 = line1 * (LINE_LENGTH + 1);
    seek(fd, offset1);
    read(fd, buffer1, LINE_LENGTH);

    // Read second line
    int offset2 = line2 * (LINE_LENGTH + 1);
    seek(fd, offset2);
    read(fd, buffer2, LINE_LENGTH);

    // Swap lines by writing back
    seek(fd, offset1);
    write(fd, buffer2, LINE_LENGTH);
    seek(fd, offset2);
    write(fd, buffer1, LINE_LENGTH);
}

int main() {
    int fd = open(FILENAME, O_CREATE | O_RDWR);
    if (fd < 0) {
        printf(1, "Error: cannot open file\n");
        exit(1);
    }

    srand(uptime()); // Initialize random seed
    write_random_lines(fd); // Write initial random lines

    // Perform 50 random swaps between lines
    for (int i = 0; i < PERMUTATIONS; i++) {
        int line1 = rand() % LINE_COUNT;
        int line2 = rand() % LINE_COUNT;
        if (line1 != line2) {
            swap_lines(fd, line1, line2);
        }
    }

    close(fd);
    unlink(FILENAME); // Delete the file

    printf(1, "File operations completed.\n");
    exit(0);
}
