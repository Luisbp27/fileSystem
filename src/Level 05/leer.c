#include "ficheros.h"

#define BUFFER_SIZE 1500

int main(int argc, char *argv[]) {

    // Checking syntax
    if (argc < 3) {
        fprintf(stderr, "Command syntax should be: leer <nombre_dispositivo> <nº inodo>\n");
        return FAILURE;
    }

    unsigned int ninodo = atoi(argv[2]);

    // Mounting the virtual device
    if (bmount(argv[1]) == FAILURE) {
        fprintf(stderr, "Error while mounting the virtual device\n");
        return FAILURE;
    }

    // Cleaning the buffer, filling it with 0s
    unsigned char buffer_texto[BUFFER_SIZE];
    if (!memset(buffer_texto, 0, BUFFER_SIZE)) {
        fprintf(stderr, "Error while setting memory\n");
        return FAILURE;
    }

    unsigned int offset = 0;
    unsigned int total_bytes_leidos = 0;
    unsigned int bytes_leidos = mi_read_f(ninodo, buffer_texto, offset, BUFFER_SIZE);
    if (bytes_leidos == FAILURE) {
        fprintf(stderr, "Could not read the file. Does it have read perms?\n");
        return FAILURE;
    }

    while (bytes_leidos > 0) {
        total_bytes_leidos += bytes_leidos;
        write(1, buffer_texto, bytes_leidos);

        // Cleaning the buffer
        if (!memset(buffer_texto, 0, BUFFER_SIZE)) {
            fprintf(stderr, "Error while setting memory\n");
            return FAILURE;
        }

        offset += BUFFER_SIZE;
        bytes_leidos = mi_read_f(ninodo, buffer_texto, offset, BUFFER_SIZE);
        // fprintf(stderr, "bytes leidos: %d\n", bytes_leidos);
    }

    inodo_t inodo;
    if (leer_inodo(ninodo, &inodo) == FAILURE) {
        fprintf(stderr, "Error while reading the inode\n");
        return FAILURE;
    }

    fprintf(stderr, "\n\nTotal bytes leidos: %u\n", total_bytes_leidos);
    fprintf(stderr, "tamEnBytesLog: %u\n", inodo.tamEnBytesLog);

    if (bumount() == FAILURE) {
        fprintf(stderr, "Error while unmounting the virtual device\n");
        return FAILURE;
    }

    return SUCCESS;
}
