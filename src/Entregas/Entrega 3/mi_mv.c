#include "directorios.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <string.h>

// Function to find the parent directory
void parent_dir(const char* entry, char* parent) {
    size_t len = strlen(entry);
    
    // Check if the entry is a directory and remove trailing slashes
    if (len > 0 && entry[len - 1] == '/') {
        len--;
    }
    
    // Find the last occurrence of '/'
    const char* last_slash = strrchr(entry, '/');
    
    if (last_slash != NULL) {
        // Calculate the length of the parent directory
        size_t parent_len = last_slash - entry + 1;
        
        // Copy the parent directory into the buffer
        strncpy(parent, entry, parent_len);
        parent[parent_len] = '\0'; // Null-terminate the string
    } else {
        // If there is no '/' in the entry, set parent directory as empty string
        parent[0] = '\0';
    }
}

/**
 * This method moves an entry to another directory
 *
 * @param argc
 * @param argv
 *
 * @return 0 if success, -1 otherwise.
 */
int main(int argc, char **argv) {
    // Check syntax
    if (argc < 4) {
        fprintf(stderr, "Command syntax should be: ./mi_rn <disco> </origen/nombre> </destino/>\n");
        return FAILURE;
    }

    char *disk = argv[1];
    char *src = argv[2];
    char *dest_parent = argv[3];

    // Mount virtual device
    if (bmount(disk) == FAILURE) {
        return FAILURE;
    }

    unsigned int p_inodo_src_parent = 0;
    unsigned int p_inodo_src = 0;
    unsigned int p_entrada_src = 0;

    // Find the entry
    int error;
    if ((error = buscar_entrada(src, &p_inodo_src_parent, &p_inodo_src, &p_entrada_src, 0, 0)) < 0) {
        mostrar_error_buscar_entrada(error);
        return FAILURE;
    }

    struct entrada entrada_src;
    if (mi_read_f(p_inodo_src_parent, &entrada_src, p_entrada_src * sizeof(struct entrada), sizeof(struct entrada)) == FAILURE) {
        return FAILURE;
    }

    struct STAT dest_dir_stat;
    if (mi_stat(dest_parent, &dest_dir_stat) < 0) {
        return FAILURE;
    }

    if (dest_dir_stat.tipo != 'd') {
        fprintf(stderr, "Error: Destination is not a directory\n");
        return FAILURE;
    }

    // Get dest path (to check if it already exists)
    char dest[TAMNOMBRE * PROFUNDIDAD];
    strcpy(dest, dest_parent);
    strcat(dest, entrada_src.nombre);

    unsigned int p_inodo_dest_parent = 0;
    unsigned int p_inodo_dest = 0;
    unsigned int p_entrada_dest = 0;

    if (buscar_entrada(dest, &p_inodo_dest_parent, &p_inodo_dest, &p_entrada_dest, 0, 0) != ERROR_NO_EXISTE_ENTRADA_CONSULTA) {
        fprintf(stderr, "Error: Entry already exists\n");
        return FAILURE;
    }  

    inodo_t inodo_src_parent;
    if (leer_inodo(p_inodo_src_parent, &inodo_src_parent) == FAILURE) {
        return FAILURE;
    }

    inodo_t inodo_dest_parent;
    if (leer_inodo(p_inodo_dest_parent, &inodo_dest_parent) == FAILURE)  {
        return FAILURE;
    } 

    // Delete src entry
    int num_entradas_inodo = inodo_src_parent.tamEnBytesLog / sizeof(struct entrada);
    // If the entry is not the last one, we need to move the last entry to the deleted entry
    if (p_entrada_src != num_entradas_inodo - 1) {
        struct entrada entry_to_move;
        // Read the last entry
        if (mi_read_f(p_inodo_src_parent, &entry_to_move, sizeof(struct entrada) * (num_entradas_inodo - 1), sizeof(struct entrada)) < 0) {
            return FAILURE;
        }

        // Write the last entry in the deleted entry
        if (mi_write_f(p_inodo_src_parent, &entry_to_move, sizeof(struct entrada) * p_entrada_src, sizeof(struct entrada)) < 0) {
            return FAILURE;
        }
    }

    // Delete the last entry
    if (mi_truncar_f(p_inodo_src_parent, sizeof(struct entrada) * (num_entradas_inodo - 1)) == FAILURE) {
        return FAILURE;
    } 

    if (mi_write_f(p_inodo_dest_parent, &entrada_src, inodo_dest_parent.tamEnBytesLog, sizeof(struct entrada)) == FAILURE) {
        return FAILURE;
    }

    // Unmount virtual device
    if (bumount() == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}