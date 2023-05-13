#include "directorios.h"
#include <string.h>

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
        fprintf(stderr, "Command syntax should be: mi_ls <disco> </ruta directorio> [-l]\n");
        return FAILURE;
    }

    int extended = argc == 4 && strcmp(argv[3], "-l") == 0;

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
    char buffer[TAMBUFFER] = {0};

    // Type of the entry
    char tipo;

    // Reading the directory entries
    int total = mi_dir(argv[2], buffer, &tipo, extended);
    if (total == FAILURE) {
        bumount();

        return FAILURE;
    }

#if DEBUGIMPORTANT
    printf("Total: %d \n", total);
#endif

    if (total != 0 && extended) {
        printf("TIPO\tPERMISOS\tMTIME\t\tTAMAÑO\tNOMBRE\n");
        printf("-----------------------------------------------------\n");
    }

    printf("%s", buffer);

    if (bumount() == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}
