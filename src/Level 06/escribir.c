#include <stdlib.h>
#include "ficheros.h"

/**
 * Usage: escribir <nombre_dispositivo> <"$(cat fichero)"> <diferentes_inodos>
 * 
 * @param argc
 * @param argv
 * 
 * @return 0 if success, -1 if error
*/
int main (int argc, char *argv[]) {

    // Checking syntax
    if (argc < 3) { 
        fprintf(stderr,"Command syntax should be: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n");
        return FAILURE;
    }

    unsigned int len = strlen(argv[2]);
    int diferentes_inodos = atoi(argv[3]);
    int offsets[] = { 9000, 209000, 30725000, 409605000, 480000000 };

    printf("Longitud texto: %d\n\n", len);

    // Mounting the virtual device
    if (bmount(argv[1]) == FAILURE) {
        fprintf(stderr, "Error mounting the virtual device.\n");
        return FAILURE;
    }

    // Allocating the inode
    int ninodo = reservar_inodo('f', 6);
    if (ninodo == FAILURE) {
        fprintf(stderr, "Error allocating the inode.\n");
        return FAILURE;
    }

    // Buffer to write
    char buffer_texto[len];
    memset(buffer_texto, 0, len);

    for (int i = 0; i < sizeof(offsets) / sizeof(int); i++) {

        printf("Nº inodo reservado: %d\n", ninodo);
        printf("offset: %d\n", offsets[i]);

        // Writing the text
        int bytes_escritos = mi_write_f(ninodo, argv[2], offsets[i], len);
        if (bytes_escritos == FAILURE) {
            fprintf(stderr, "escribir.c: Error mi_write_f().\n");
            return FAILURE;
        }
        printf("Bytes escritos: %d\n\n", bytes_escritos);
       
        // Obtaining the information of the written inode
        struct STAT p_stat;
        if (mi_stat_f(ninodo, &p_stat)) {
            fprintf(stderr, "escribir.c: Error mi_stat_f()\n");
            return FAILURE;
        }

        fprintf(stderr, "stat.tamEnBytesLog = %d\n", p_stat.tamEnBytesLog);
        fprintf(stderr, "stat.numBloquesOcupados = %d\n\n", p_stat.numBloquesOcupados);
        
        // If the <diferentes_indodos> parameter is 0, allocate a new inode
        if (diferentes_inodos != 0) {
            ninodo = reservar_inodo('f', 6);
        
            if (ninodo == FAILURE) {
                return FAILURE;
            }
        }
    }

    // Unmounting the virtual device
    if (bumount() == FAILURE) {
        fprintf(stderr, "Error unmounting the virtual device.\n");
        return FAILURE;
    }
    
    return SUCCESS;
}

