#include <stdlib.h>
#include "ficheros.h"

int main (int argc, char *argv[]) {

    struct STAT p_stat;
    unsigned int len = strlen(argv[2]);
    int diferentes_inodos = atoi(argv[3]);
    char *buffer_texto = malloc(len);
    int offsets[5] = { 9000, 209000, 30725000, 409605000, 480000000 };
    int bytes_escritos = 0;
    int bytes_leidos = 0;
    int ninodo;

    printf("Longitud texto: %d\n\n", len);

    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL){ // Checkear syntax 
        fprintf(stderr,"Command syntax should be: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n");
        return FAILURE;
    }

    if (bmount(argv[1]) == FAILURE) {
        fprintf(stderr, "Error mounting the virtual device.\n");
        return FAILURE;
    }

    ninodo = reservar_inodo('f', 6);
    if (ninodo == FAILURE) {
        fprintf(stderr, "Error allocating the inode.\n");
        return FAILURE;
    }

    // Bucle de escritura en todos los offsets del array.
    for (int i = 0; i < (sizeof(offsets) / sizeof(int)); i++) {

        printf("Nº inodo reservado: %d\n", ninodo);
        printf("offset: %d\n", offsets[i]);

        bytes_escritos = mi_write_f(ninodo, argv[2], offsets[i], len);
        if (bytes_escritos == FAILURE) {
            fprintf(stderr, "escribir.c: Error mi_write_f().\n");
            return FAILURE;
        }
        printf("Bytes escritos: %d\n\n", bytes_escritos);

        memset(buffer_texto, 0, len);
        bytes_leidos = mi_read_f(ninodo, buffer_texto, offsets[i], len);
        printf("Bytes leídos: %d\n", bytes_leidos);

        // Obtencion de la información del inodo escrito
        if (mi_stat_f(ninodo, &p_stat)) {
            fprintf(stderr, "escribir.c: Error mi_stat_f()\n");
            return FAILURE;
        }

        printf("stat.tamEnBytesLog = %d\n", p_stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados = %d\n\n", p_stat.numBloquesOcupados);
        
        // Si el parámetro <diferentes_indodos> es 0, reserva un nuevo inodo.
        if (diferentes_inodos == 0) {
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

