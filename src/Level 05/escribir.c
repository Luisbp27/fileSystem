#include "ficheros.h"

int main (int argc, char *argv[]) {

    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL){ // Checkear syntax 
        fprintf(stderr,"Command syntax should be: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n");
        return -1;
    }

    int offsets[5] = { 9000, 209000, 30725000, 409605000, 480000000 };

    if (bmount(argv[1]) == FAILURE) {
        fprintf(stderr, "Error mounting the virtual device.\n");
        return FAILURE;
    }

    unsigned int ninodo = reservar_inodo('f', 6);

    if (ninodo == FAILURE) {
        fprintf(stderr, "Error allocating the inode.\n");
        return FAILURE;
    }

    // Writting on all the offsets of the array
    for (int i = 0; i < (sizeof(offsets) / sizeof(int)); i++) {
        printf("Nº inodo reservado: %d\n", ninodo);
        printf("offset: %d\n", offsets[i]);

        int bytes_escritos = mi_write_f(ninodo, argv[2], offsets[1], strlen(argv[2]));
        if (bytes_escritos == FAILURE) {
            fprintf(stderr, "Error in bytes_escritos.\n");
            return FAILURE;
        }
        printf("Bytes escritos: %d\n", bytes_escritos);

        int len = strlen(argv[2]);
        char *buffer_texto = malloc(len);
        memset(buffer_texto, 0, len);
        unsigned int bytes_leidos = mi_read_f(ninodo, buffer_texto, offsets[1], len);
        printf("Bytes leidos: %d\n", bytes_leidos);

        struct stat p_stat;
        // Taking the information of the written inode
        if (mi_stat_f(ninodo, &p_stat) == FAILURE) {
            fprintf(stderr, "Error in mi_stat_f.\n");
            return FAILURE;
        }

        prinf("stat.tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados: %d\n", p_stat.numBloquesOcupados);

        // If the parameter <diferentes_inodos> is 0, allocate a new inode
        if (strcmp(argv[3], "0")) {
            ninodo = reservar_inodo('f', 6);

            if (ninodo == FAILURE) {
                fprintf(stderr, "Error allocating the inode.\n");
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

