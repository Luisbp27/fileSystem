#include <stdio.h>
#include "basic_file.h"

int main(int argc, char **argv) {
    // Check the possible errors in params
    if (argc < 3) {
        fprintf(stderr, "Not enough arguments. Usage: %s <device name> <block size>\n", argv[0]);

        return FAILURE;
    }

    char *path = argv[1];
    int n_blocks = atoi(argv[2]);

    // Initialize the buffer to all 0s
    unsigned char buffer[BLOCKSIZE];
    void *memset_result = memset(buffer, 0, BLOCKSIZE);

    // Mount the virtual device
    if (bmount(path) == FAILURE || !memset_result) {
        fprintf(stderr, "An error occurred while mounting the system.\n");

        return FAILURE;
    }

    // Initialize the virtual device to all 0s
    for (int i = 0; i < n_blocks; i++) {
        if (bwrite(i, buffer) == FAILURE) {
            fprintf(stderr, "An error occurred when writing to position %d of the virtual device.\n", i);

            return FAILURE;
        }
    }

    // Unmount the virtual device
    if (bumount() == FAILURE) {
        fprintf(stderr, "An error ocurred while unmounting the system.\n");

        return FAILURE;
    }
}