#include "directorios.h"

int copy_recursive(char *src, char *dest) {
    // Get file info
    struct STAT stat;
    if (mi_stat(src, &stat) < 0) {
        return FAILURE;
    }

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    // Find directory inode
    int error;
    if ((error = buscar_entrada(src, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0) {
        mostrar_error_buscar_entrada(error);
        return FAILURE;
    }

    inodo_t inodo;
    if (leer_inodo(p_inodo, &inodo) == FAILURE) {
        return FAILURE;
    }

    // Read p_inodo_dir to get the name of the directory p_entrada
    // offset = p_entrada * sizeof(struct entrada), buffer = sizeof(struct entrada)
    struct entrada entrada;
    if (mi_read_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == FAILURE) {
        return FAILURE;
    }

    // Name of the new directory
    char dest_nombre[TAMNOMBRE];
    strcpy(dest_nombre, dest);
    strcat(dest_nombre, entrada.nombre);

    // Create the new directory
    if (mi_creat(dest_nombre, inodo.permisos) == FAILURE) {
        return FAILURE;
    }

    // If it is a file or a directory
    if (stat.tipo == 'f') {
        // Copy the file
        char buffer[BLOCKSIZE];
        int offset = 0;
        while (offset < inodo.tamEnBytesLog) {
            int bytes = mi_read(src, buffer, offset, BLOCKSIZE);
            if (bytes == FAILURE) {
                return FAILURE;
            }

            if (mi_write(dest_nombre, buffer, offset, bytes) == FAILURE) {
                return FAILURE;
            }

            offset += bytes;
        }
    } else {
        // Find all entries
        struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
        int nEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);

        // Copy all entries recursively
        int offset = mi_read_f(p_inodo, entradas, 0, BLOCKSIZE);
        for (int i = 0; i < nEntradas; i++) {
            char dest_nombre[TAMNOMBRE];
            strcpy(dest_nombre, dest);
            strcat(dest_nombre, entradas[i % (BLOCKSIZE / sizeof(struct entrada))].nombre);

            copy_recursive(dest_nombre, dest);

            if (offset % (BLOCKSIZE / sizeof(struct entrada)) == 0) {
                offset += mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);
            }
        }
    }

    return SUCCESS;
}

/**
 * This method copies a file or directory from one src to another.
 *
 * @param argc
 * @param argv
 *
 * @return 0 if success, -1 otherwise.
*/
int main(int argc, char **argv) {

    // Check syntax
    if (argc < 4) {
        fprintf(stderr, "Command syntax should be: ./mi_cp <disco> </origen/dest_nombre> </destino/>\n");
        return FAILURE;
    }

    // Mount virtual device
    if (bmount(argv[1]) == FAILURE) {
        return FAILURE;
    }

    // Check if the source file exists
    struct STAT stat;
    if (mi_stat(argv[2], &stat) < 0) {
        return FAILURE;
    }

    // Check if the destination directory exists
    if (mi_stat(argv[3], &stat) < 0) {
        return FAILURE;
    }

    // If it is a file or a directory
    if (copy_recursive(argv[2], argv[3]) == FAILURE) {
        return FAILURE;
    }

    // Unmount virtual device
    if (bumount() == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}