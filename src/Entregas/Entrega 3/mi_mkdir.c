#include "directorios.h"

int main(int argc, char **argv) {
    // Checking syntax
    if (argc < 4) {
        fprintf(stderr, "Command syntax should be: mi_mkdir <disco> <permisos> </ruta> \n");
        return FAILURE;
    }

    // Checking if the path ends in /
    int path_len = strlen(argv[3]);
    if (argv[3][path_len - 1] != '/') {
        fprintf(stderr, "Invalid path \n");
        return FAILURE;
    }

    // Check perms
    int perms = atoi(argv[2]);
    if ((perms < 0) || (perms > 7)) {
        fprintf(stderr, "Invalid perms <<%d>> \n", perms);
        return FAILURE;
    }

    if (bmount(argv[1]) == FAILURE) {
        fprintf(stderr, "Error mounting the disk \n");
        return FAILURE;
    }

    // Creating the directory
    if (mi_creat(argv[3], atoi(argv[2])) == FAILURE) {
        bumount();

        return FAILURE;
    }

    if (bumount() == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}
