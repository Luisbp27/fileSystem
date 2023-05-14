#include "directorios.h"

int main(int argc, char **argv) {
    
    // Checking syntax
    if (argc < 4) {
        fprintf(stderr, "Command syntax should be: ./mi_link <disco> </ruta_fichero_original> </ruta_enlace> \n");
        return FAILURE;
    }

    // Checking if the path ends in /
    if (argv[2][strlen(argv[2]) - 1] == '/') {
        fprintf(stderr, "Invalid path of the file \n");
        return FAILURE;
    }

    if (argv[3][strlen(argv[3]) - 1] == '/') {
        fprintf(stderr, "Invalid path of the link \n");
        return FAILURE;
    }

    if (bmount(argv[1]) == FAILURE) {
        fprintf(stderr, "Error mounting the disk \n");
        return FAILURE;
    }

    // Linking the file
    if (mi_link(argv[2], argv[3]) < 0) {
        return FAILURE;
    }

    if (bumount() == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}