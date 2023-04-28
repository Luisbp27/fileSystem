#include "directorios.h"

#define TAMFILA 100
#define TAMBUFFER (TAMFILA * 1000)

/**
 * Program (command) that lists the contents of a directory (names of the entries),
 * calling the function my_dir()
 *
 */
int main(int argc, char **argv) {

    // Checking syntax
    if (argc < 3) {
        fprintf(stderr, "Command syntax should be: mi_ls <disco> </ruta directorio> \n");
        return FAILURE;
    }

    if (bmount(argv[1]) == FAILURE) {
        fprintf(stderr, "Error mounting the disk \n");
        return FAILURE;
    }

    super_bloque_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        fprintf(stderr, "Error reading the superblock \n");
        return FAILURE;
    }

    // Buffer to store the directory entries
    char buffer[TAMBUFFER];
    memset(buffer, 0, TAMBUFFER);

    // Type of the entry
    char tipo;

    // Reading the directory entries
    if (mi_dir(argv[2], buffer, &tipo) == FAILURE) {
        fprintf(stderr, "Error reading the directory entries \n");
        bumount();

        return FAILURE;
    }

    // Checking if the path ends in / and if it is a directory
    if ((argv[2][(strlen(argv[2])) - 1] != '/') && (tipo == 'd')) {
        fprintf(stderr, "The path must end in / to list a directory \n");
        bumount();

        return FAILURE;
    }

    printf("TIPO\tPERMISOS\tMTIME\t\tTAMAÑO\tNOMBRE\n");
    printf("-----------------------------------------------------\n%s", buffer);

    bumount();

    return SUCCESS;
}
