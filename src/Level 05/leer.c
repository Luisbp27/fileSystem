#include "ficheros.h"

int main(int argc, char *argv[]) {

    inodo_t inodo;

    unsigned int tam_buffer = 1500;
    unsigned char buffer_texto[tam_buffer];
    unsigned int bytes_leidos = 0;
    unsigned int total_bytes_leidos = 0;
    unsigned int ninodo = atoi(argv[2]);
    unsigned int offset = 0;

    // Checking syntax
    if(argv[1] == NULL || argv[2] == NULL ) {  
        fprintf(stderr,"Command syntax should be: leer <nombre_dispositivo> <nº inodo>\n");
        return FAILURE;
    }

    // Mounting the virtual device
    if (bmount(argv[1]) == FAILURE) {
        fprintf(stderr,"Error while mounting the virtual device\n");
        return FAILURE;
    }

    // Cleaning the buffer, filling it with 0s
    if(memset(buffer_texto, 0, tam_buffer) == NULL) {
        fprintf(stderr, "Error while setting memory\n");
        return FAILURE;
    }
    
    bytes_leidos = mi_read_f(ninodo, buffer_texto, offset, tam_buffer);

    while (bytes_leidos > 0) {
        write(1, buffer_texto, bytes_leidos);
        total_bytes_leidos += bytes_leidos;
        offset += tam_buffer;

        if (memset(buffer_texto, 0, tam_buffer) == NULL) {
            fprintf(stderr, "Error while setting memory\n");
            return FAILURE;
        }

        bytes_leidos = mi_read_f(ninodo, buffer_texto, offset, tam_buffer);
    }

    if (leer_inodo(ninodo, &inodo) == FAILURE) {
        fprintf(stderr, "Error while reading the inode\n");
        return FAILURE;
    }

    printf("\n\nTotal bytes leidos: %u\n", bytes_leidos);
    printf("tamEnBytesLog: %u\n", inodo.tamEnBytesLog);

    if (bumount() == FAILURE) {
        fprintf(stderr,"Error while unmounting the virtual device\n");
        return FAILURE;
    }
}