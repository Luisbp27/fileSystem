#include "verificacion.h"

int main(int argc, char **argv) {

    // Check syntax
    if (argc < 2 ) {
        fprintf(stderr, "Command syntax should be: ./verificacion <disco> <directorio_simulacion>\n");
        return FAILURE;
    }

    // Mount virtual device
    if (bmount(argv[1]) == FAILURE) {
        return FAILURE;
    }

    struct STAT stat;
    struct entrada buff_entr [NUMPROCESOS];
    memset(buff_entr, 0, sizeof(buff_entr));

    // Check if the directory exists
    if (mi_stat(argv[2], &stat) == FAILURE) {
        return FAILURE;
    }

    // Calculate the number of entries of the inode
    int num_entr = stat.tamEnBytesLog / sizeof(struct entrada);
    if (num_entr != NUMPROCESOS) {
        fprintf(stderr, "The number of entries in the directory is not correct\n");
        bumount();

        return FAILURE;
    }

    // Create a report 
    char report[100];
    memset(report, 0, sizeof(report));

    sprintf(report, "%s%s", argv[2], "informe.txt");
    if (mi_creat(report, 7) == FAILURE) {
        bumount();
        exit(0);
    }

    // Upload the entries
    struct entrada entrada[num_entr];
    int error;
    if (error = mi_read(argv[2], entrada, 0, sizeof(entrada)) == FAILURE) {
        mostrar_error_buscar_entrada(error);
        return FAILURE;
    }    
}