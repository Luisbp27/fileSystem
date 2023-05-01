#include "ficheros.h"

/**
 * Usage: truncar <nombre_dispositivo> <nº inodo> <nuevo tamaño>
 *
 * @param argc
 * @param argv
 *
 * @return 0 if success, -1 if error
 */
int main(int argc, char *argv[]) {
    // Checking syntax
    if (argc < 3) {
        fprintf(stderr, "Command syntax should be: truncar <nombre_dispositivo> <nº inodo> <nuevo tamaño>\n");
        return FAILURE;
    }

    // Mounting the virtual device
    if (bmount(argv[1]) == FAILURE) {
        fprintf(stderr, "Error while mounting the virtual device\n");
        return FAILURE;
    }

    unsigned int ninodo = atoi(argv[2]);
    unsigned int nbytes = atoi(argv[3]);
    // Check if the inode exists
    if (nbytes == 0) {
        liberar_inodo(ninodo);
    } else {
        mi_truncar_f(ninodo, nbytes);
    }

    struct STAT p_stat;
    // Getting the inode's information
    if (mi_stat_f(ninodo, &p_stat) == FAILURE) {
        fprintf(stderr, "Error while getting the inode's information\n");
        return FAILURE;
    }

    // Formatting the dates
    struct tm *info;
    char adate[24];
    char cdate[24];
    char mdate[24];
    strftime(adate, 24, "%a %d-%m-%Y %H:%M:%S", info = localtime(&p_stat.atime));
    strftime(cdate, 24, "%a %d-%m-%Y %H:%M:%S", info = localtime(&p_stat.ctime));
    strftime(mdate, 24, "%a %d-%m-%Y %H:%M:%S", info = localtime(&p_stat.mtime));

    // Printing the inode's information
    printf("\nDATOS INODO [%i]\n", ninodo);
    printf("tipo: %c\n", p_stat.tipo);
    printf("permisos: %i\n", p_stat.permisos);
    printf("atime: %s\n", adate);
    printf("mtime: %s\n", mdate);
    printf("Ctime: %s\n", cdate);
    printf("nlinks: %u\n", p_stat.nlinks);
    printf("tamEnBytesLog: %u\n", p_stat.tamEnBytesLog);
    printf("numBloquesOcupados: %u\n", p_stat.numBloquesOcupados);

    // Unmounting the virtual device
    if (bumount() == FAILURE) {
        fprintf(stderr, "Error while unmounting the virtual device\n");
        return FAILURE;
    }

    return SUCCESS;
}
