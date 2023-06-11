#include "directorios.h"

int copy_recursive(char *src, char *dest) {
    // Get file info
    struct STAT src_stat;
    if (mi_stat(src, &src_stat) < 0) {
        return FAILURE;
    }
    
    if (src_stat.tipo == 'd' && src[strlen(src) - 1] != '/') {
        strcat(src, "/");
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
    struct entrada src_entrada;
    if (mi_read_f(p_inodo_dir, &src_entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == FAILURE) {
        return FAILURE;
    }

    // Name of the new entry
    char dest_new_entry[TAMNOMBRE * PROFUNDIDAD];
    strcpy(dest_new_entry, dest);
    strcat(dest_new_entry, src_entrada.nombre);
    if (src_stat.tipo == 'd') {
        strcat(dest_new_entry, "/");
    }

    // Create the new directory
    if (mi_creat(dest_new_entry, inodo.permisos) == FAILURE) {
        return FAILURE;
    }

    // If it is a file or a directory
    if (src_stat.tipo == 'f') {
        // Copy the file
        char buffer[BLOCKSIZE] = {0};

        for (int logic_block_index = 0; logic_block_index * BLOCKSIZE < inodo.tamEnBytesLog; logic_block_index++) {
            int phis_block_index = traducir_bloque_inodo(&inodo, logic_block_index, 0);
            if (phis_block_index == FAILURE) {
                // Block is not assigned, it has to be skipped
                continue;
            }

            int bytes = bread(phis_block_index, buffer);
            if (bytes == FAILURE) {
                return FAILURE;
            }

            if (inodo.tamEnBytesLog - logic_block_index * BLOCKSIZE < BLOCKSIZE) {
                bytes = inodo.tamEnBytesLog - logic_block_index * BLOCKSIZE;
            }

            if (mi_write(dest_new_entry, buffer, logic_block_index * BLOCKSIZE, bytes) == FAILURE) {
                return FAILURE;
            }
        }
    } else {
        // Find all entries
        struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
        int nEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);

        // Copy all entries recursively
        int offset = mi_read_f(p_inodo, entradas, 0, BLOCKSIZE);
        for (int i = 0; i < nEntradas; i++) {
            struct entrada *entrada_i = &entradas[i % (BLOCKSIZE / sizeof(struct entrada))];

            char src_next[TAMNOMBRE * PROFUNDIDAD];
            char dest_next[TAMNOMBRE * PROFUNDIDAD];
            strcpy(src_next, src);
            strcat(src_next, entrada_i->nombre);
            strcpy(dest_next, dest);
            strcat(dest_next, src_entrada.nombre);
            strcat(dest_next, "/");

            copy_recursive(src_next, dest_next);

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
        fprintf(stderr, "Command syntax should be: ./mi_cp <disco> </origen/nombre> </destino/>\n");
        return FAILURE;
    }

    // Mount virtual device
    if (bmount(argv[1]) == FAILURE) {
        return FAILURE;
    }

    // Check if the source exists
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