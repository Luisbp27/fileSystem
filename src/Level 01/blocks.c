#include "blocks.h"

static int descriptor = 0;

int bmount(const char *path) {

    // Open file descriptor
    descriptor = open(path, O_CREAT, 0666);

    if (descriptor == -1) {
        return EXIT_FAILURE;
    }

    return descriptor;
}

int bumount(){

    if (close(descriptor) == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/**
 * Writes 1 block to the virtual device, in the physical block 
 * specified by n_block.
 * 
*/
int bwrite(unsigned int n_block, const void *buf) {
    // Allocate the pointer
    if (lseek(descriptor, n_block * BLOCKSIZE, SEEK_SET) != -1) {

        // Write the block
        size_t bytes = write(descriptor, buf, BLOCKSIZE);

        // If the writing has gone wrong
        if (bytes < 0) {
            fprintf(stderr, "Error when writing to the block\n");
            return EXIT_FAILURE;
        }

        return bytes;

    } else {
        fprintf(stderr, "Error when positioning the file pointer\n");
        return EXIT_FAILURE;
    }
}

/**
 * Reads 1 block from the virtual device, which corresponds 
 * to the physical block specified by n_block.
 * 
*/
int bread(unsigned int n_block, void *buf) {
    // Allocate the pointer
    if (lseek(descriptor, n_block * BLOCKSIZE, SEEK_SET) != -1) {

        // Read the block
        size_t bytes = read(descriptor, buf, BLOCKSIZE);

        // If the reading has gone wrong
        if (bytes < 0) {
            fprintf(stderr, "Error when reading to the block");
            return EXIT_FAILURE;
        }

        return bytes;

    } else {
        fprintf(stderr, "Error when positioning the file pointer\n");
        return EXIT_FAILURE;
    }
}