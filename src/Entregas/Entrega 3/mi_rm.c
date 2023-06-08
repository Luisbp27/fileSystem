#include "directorios.h"

int remove_f(char *path, int recursive) {
    // Get file info
    struct STAT stat;
    if (mi_stat(path, &stat) < 0) {
        return FAILURE;
    }      
   
    if (!recursive || stat.tipo == 'f') {
        if (mi_unlink(path) == FAILURE) {
            return FAILURE;
        }
    } else {
        unsigned int p_inodo_dir = 0;
        unsigned int p_inodo = 0;
        unsigned int p_entrada = 0;

        // Find directory inode
        int error;
        if ((error = buscar_entrada(path, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0) {
            mostrar_error_buscar_entrada(error);
            return FAILURE;
        }

        inodo_t inodo;
        if (leer_inodo(p_inodo, &inodo) == FAILURE) {
            return FAILURE;
        }

        // Find all entries
        struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
        int nEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);

        // Remove all entries recursively
        int offset = mi_read_f(p_inodo, entradas, 0, BLOCKSIZE);
        for (int i = 0; i < nEntradas; i++) {
            char nombre[TAMNOMBRE];
            strcpy(nombre, path);
            if (nombre[strlen(nombre) - 1] != '/') {
                strcat(nombre, "/");
            }
            strcat(nombre, entradas[i % (BLOCKSIZE / sizeof(struct entrada))].nombre);

            remove_f(nombre, recursive);

            if (offset % (BLOCKSIZE / sizeof(struct entrada)) == 0) {
                offset += mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);
            }
        }

        // Remove original directory, which is now empty
        if (mi_unlink(path) == FAILURE) {
            return FAILURE;
        }
    }

    return SUCCESS;
}

/**
 * Method that removes a file from the file system.
 *
 * @param   camino  Path to the file to remove.
 *
 * @return  0 if success, -1 otherwise.
 */
int main(int argc, char **argv) {
    // Check syntax
    if (argc < 3) {
        fprintf(stderr, "Command syntax should be: mi_rm <disco> </ruta> [-r]\n");
        return FAILURE;
    }
    char *disk = argv[1];
    char *path = argv[2];
    int recursive = argc == 4 && strcmp(argv[3], "-r") == 0;

    // Mount disk
    if (bmount(disk) == FAILURE) {
        return FAILURE;
    }

    if (remove_f(path, recursive) == FAILURE) {
        return FAILURE;
    }

    // Unmount disk
    if (bumount() == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}
