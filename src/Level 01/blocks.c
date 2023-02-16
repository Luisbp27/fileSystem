#include "blocks.h"

static int descriptor = 0;

int bmount(const char *path) {

    // Open file descriptor

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

int bwrite(unsigned int block, const void *buf) {

}

int bread(unsigned int n_block, void *buf) {

}