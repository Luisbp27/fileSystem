#include "directorios.h"

int main(int argc, char **argv) {

    // Check syntax
    if (argc < 3) {
        fprintf(stderr, "Command syntax should be: mi_rmdir <disco> </ruta> \n");
        return FAILURE;
    }

    // Mount disk
    if (bmount(argv[1]) == FAILURE) {
        return FAILURE;
    }

    // Get file info
    struct STAT stat;
    if (mi_stat(argv[2], &stat) < 0) {
        return FAILURE;
    }

    // Check if it is a directory
    if (stat.tipo != 'd') {
        fprintf(stderr, "The path is not a directory \n");
        return FAILURE;
    }

    // Remove directory
    if (mi_unlink(argv[2]) == FAILURE) {
        return FAILURE;
    }

    // Unmount disk
    if (bumount() == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}