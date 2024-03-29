#include "directorios.h"

int main (int argc, char **argv) {

    // Checking syntax
    if (argc < 3) {
        fprintf(stderr, "Command syntax should be: mi_cat <disco> </ruta_fichero> \n");
        return FAILURE;
    }

    char *disk = argv[1];
    char *path = argv[2];

    if (bmount(disk) == FAILURE) {
        fprintf(stderr, "Error mounting the disk \n");
        return FAILURE;
    }

    unsigned int read_bytes = 0;
    unsigned int read_bytes_total = 0;
    unsigned int offset = 0;
    char buffer[BLOCKSIZE * 4] = {0};
    
    // Reading the file
    while ((read_bytes = mi_read(path, buffer, offset, sizeof(buffer))) > 0) {
        read_bytes_total += read_bytes;

        // Show contents of file
        write(1, buffer, read_bytes);

        // Clean the buffer
        memset(buffer, 0, sizeof(buffer));
        // Prepare offset for next iteration
        offset += sizeof(buffer);
    }

    fprintf(stderr, " \n");
    if (read_bytes_total < 0) {
        mostrar_error_buscar_entrada(read_bytes_total);
        read_bytes_total = 0;
    }
    fprintf(stderr, "\n Total_leidos: %d \n", read_bytes_total);

    if (bumount(disk) == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}