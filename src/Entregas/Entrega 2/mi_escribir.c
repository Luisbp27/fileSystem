#include "directorios.h"

int main(int argc, char **argv) {
    // Checking syntax
    if (argc < 5) {
        fprintf(stderr, "Command syntax should be: mi_chmod <disco> <permisos> </ruta> \n");
        return FAILURE;
    }

    // Checking if the path ends in /
    if (argv[2][strlen(argv[2]) - 1] == '/') {
        fprintf(stderr, "Invalid path \n");
        return FAILURE;
    }

    if (bmount(argv[1]) == FAILURE) {
        fprintf(stderr, "Error mounting the disk \n");
        return FAILURE;
    }

    #if DEBUGIMPORTANT
        fprintf(stderr, "Longitud texto: %ld \n", strlen(argv[3]));
    #endif

    // Writing the file
    int bytes = mi_write(argv[2], argv[3], atoi(argv[4]), strlen(argv[3]));
    if (bytes < 0) {
        mostrar_error_buscar_entrada(bytes);
        bytes = 0;
    }

    if (bumount() == FAILURE) {
        return FAILURE;
    }

    #if DEBUGIMPORTANT
        fprintf(stderr, "Bytes escritos: %d \n", bytes);
    #endif

    return SUCCESS;
}