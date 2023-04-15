#include "ficheros.h"
#include <stdlib.h>

#define DEBUG 1

int main(int argc, char *argv[]) {

    // Checking syntax
    if (argc < 4) {
        fprintf(stderr, "Command syntax should be: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n");
        return FAILURE;
    }

    unsigned int len = strlen(argv[2]);
    int diferentes_inodos = atoi(argv[3]);
    int offsets[] = {9000, 209000, 30725000, 409605000, 480000000};

    printf("Longitud texto: %d\n", len);
    printf("Texto: %s\n\n", argv[2]);

    if (bmount(argv[1]) == FAILURE) {
        fprintf(stderr, "Error mounting the virtual device.\n");
        return FAILURE;
    }

    int ninodo = reservar_inodo('f', 6);
    if (ninodo == FAILURE) {
        fprintf(stderr, "Error allocating the inode.\n");
        return FAILURE;
    }

    // Bucle de escritura en todos los offsets del array.
    for (int i = 0; i < sizeof(offsets) / sizeof(int); i++) {

        printf("Nº inodo reservado: %d\n", ninodo);
        printf("offset: %d\n", offsets[i]);

        int bytes_escritos = mi_write_f(ninodo, argv[2], offsets[i], len);
        if (bytes_escritos == FAILURE) {
            fprintf(stderr, "escribir.c: Error mi_write_f().\n");
            return FAILURE;
        }
        printf("Bytes escritos: %d\n", bytes_escritos);

#if DEBUG
        char buffer_texto[len + 1];
        if (!memset(buffer_texto, 0, sizeof(buffer_texto))) {
            fprintf(stderr, "Error allocating the text buffer\n");
            return FAILURE;
        }

        int bytes_leidos = mi_read_f(ninodo, buffer_texto, offsets[i], len);
        printf("Texto leído: %s\n", buffer_texto);
        printf("Bytes leídos: %d\n", bytes_leidos);
#endif

        printf("\n");

        // Obtencion de la información del inodo escrito
        struct STAT p_stat;
        if (mi_stat_f(ninodo, &p_stat)) {
            fprintf(stderr, "escribir.c: Error mi_stat_f()\n");
            return FAILURE;
        }

        fprintf(stderr, "stat.tamEnBytesLog = %d\n", p_stat.tamEnBytesLog);
        fprintf(stderr, "stat.numBloquesOcupados = %d\n\n", p_stat.numBloquesOcupados);

        // Si el parámetro <diferentes_indodos> es 0, reserva un nuevo inodo.
        if (diferentes_inodos != 0) {
            ninodo = reservar_inodo('f', 6);

            if (ninodo == FAILURE) {
                return FAILURE;
            }
        }
    }

    if (bumount() == FAILURE) {
        fprintf(stderr, "Error unmounting the virtual device.\n");
        return FAILURE;
    }

    return SUCCESS;
}
