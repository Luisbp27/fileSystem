#include "directorios.h"

int main(int argc, char **argv) {
    // Checking syntax
    if (argc < 3) {
        fprintf(stderr, "Command syntax should be: ./mi_stat <disco> </ruta> \n");
        return FAILURE;
    }

    if (bmount(argv[1]) == FAILURE) {
        fprintf(stderr, "Error mounting the disk \n");
        return FAILURE;
    }

    // Getting the stat
    struct STAT stat;
    int ninodo = mi_stat(argv[2], &stat);

    // Checking the stat
    if (ninodo == FAILURE) {
        fprintf(stderr, "Error getting the stat \n");
        bumount();

        return FAILURE;
    }

    char adate[24], mdate[24], cdate[24];

    // Formatting the dates
    strftime(adate, 24, "%a %d-%m-%Y %H:%M:%S", localtime(&stat.atime));
    strftime(cdate, 24, "%a %d-%m-%Y %H:%M:%S", localtime(&stat.ctime));
    strftime(mdate, 24, "%a %d-%m-%Y %H:%M:%S", localtime(&stat.mtime));

    // Printing the stat
    printf("Nº de inodo: %d \n", ninodo);
    printf("Tipo: %c \n", stat.tipo);
    printf("Permisos: %d \n", stat.permisos);
    printf("atime: %s \n", adate);
    printf("ctime: %s \n", cdate);
    printf("mtime: %s \n", mdate);
    printf("nlinks: %d \n", stat.nlinks);
    printf("tamEnBytesLog: %d \n", stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d \n", stat.numBloquesOcupados);

    if (bumount() == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}
