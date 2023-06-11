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
 * This method renames an entry in a directory
 *
 * @param argc
 * @param argv
 *
 * @return 0 if success, -1 otherwise.
 */
int main(int argc, char **argv) {
    // Check syntax
    if (argc < 4) {
        fprintf(stderr, "Command syntax should be: ./mi_rn <disco> </origen/nombre> <nuevo nombre>\n");
        return FAILURE;
    }

    char *disk = argv[1];
    char *src = argv[2];
    char *new_name = argv[3];

    // Mount virtual device
    if (bmount(disk) == FAILURE) {
        return FAILURE;
    }

    unsigned int p_inodo_dir_src = 0;
    unsigned int p_inodo_src = 0;
    unsigned int p_entrada_src = 0;

    // Find the entry
    int error;
    if ((error = buscar_entrada(src, &p_inodo_dir_src, &p_inodo_src, &p_entrada_src, 0, 0)) < 0) {
        mostrar_error_buscar_entrada(error);
        return FAILURE;
    }

    // Get dest path (to check if it already exists)
    char dest[TAMNOMBRE * PROFUNDIDAD];
    parent_dir(src, dest);
    strcat(dest, new_name);

    unsigned int p_inodo_dir_dest = 0;
    unsigned int p_inodo_dest = 0;
    unsigned int p_entrada_dest = 0;

    if (buscar_entrada(dest, &p_inodo_dir_dest, &p_inodo_dest, &p_entrada_dest, 0, 0) != ERROR_NO_EXISTE_ENTRADA_CONSULTA) {
        fprintf(stderr, "Error: Entry already exists\n");
        return FAILURE;
    } 

    // Read p_inodo_dir to get the name of the directory p_entrada
    // offset = p_entrada * sizeof(struct entrada), buffer = sizeof(struct entrada)
    struct entrada entrada;
    if (mi_read_f(p_inodo_dir_src, &entrada, p_entrada_src * sizeof(struct entrada), sizeof(struct entrada)) == FAILURE) {
        return FAILURE;
    }

    strcpy(entrada.nombre, new_name);

    if (mi_write_f(p_inodo_dir_src, &entrada, p_entrada_src * sizeof(struct entrada), sizeof(struct entrada)) == FAILURE) {
        return FAILURE;
    }

    // Unmount virtual device
    if (bumount() == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}