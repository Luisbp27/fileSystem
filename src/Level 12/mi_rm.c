#include "directorios.h"

/**
 * Method that removes a file from the file system.
 * 
 * @param   camino  Path to the file to remove.
 * 
 * @return  0 if success, -1 otherwise.
 */
int main(int argc, char **argv) {

    // Check syntax
    if (argc < 2) {
        fprintf(stderr, "Command syntax should be: mi_rm <disco> </ruta> \n");
        return FAILURE;
    }

    // Mount disk
    if (bmount(argv[1]) == FAILURE) {
        return FAILURE;
    }

    // Get file info
    struct STAT stat;
    if (mi_stat(argv[2], &stat) == FAILURE) {
        return FAILURE;
    }

    // Check if it is a file
    if (stat.tipo != 'f') {
        return FAILURE;
    }

    // Remove file
    if (mi_unlink(argv[2]) == FAILURE) {
        return FAILURE;
    }

    // Unmount disk
    if (bumount() == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}