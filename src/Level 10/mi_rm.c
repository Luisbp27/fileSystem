#include "directorios.h"

int main(int argc, char **argv) {

    // Check syntax
    if (argc < 2) {
        fprintf(stderr, "Command syntax should be: mi_rm <disco> </ruta> \n");
        return FAILURE;
    }

    // Mount disk
    if (bmount(argv[1]) == FAILURE) {
        fprintf(stderr, "Couldn't mount disk\n");
        return FAILURE;
    }

    // Remove file
    if (mi_unlink(argv[2]) == FAILURE) {
        fprintf(stderr, "Couldn't remove file\n");
        return FAILURE;
    }

    // Unmount disk
    if (bumount() == FAILURE) {
        fprintf(stderr, "Couldn't unmount disk\n");
        return FAILURE;
    }

    return SUCCESS;
}