#include "directorios.h"

/**
 * This method extracts the path of a file or directory from a given path.
 * 
 * @param camino 
 * @param inicial 
 * @param final 
 * 
 * @return 0 if the path is correct, -1 otherwise
*/
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {

    if (camino[0] != '/') {
        return FAILURE;
    }

    if (!strchr(camino + 1, '/')) {
        strcpy(inicial, camino + 1);
        *tipo = 'f';
        strcpy(final, "");
    } else {
        strncpy(inicial, camino + 1, strchr(camino + 1, '/') - camino - 1);
        *tipo = 'd';
        strcpy(final, strchr(camino + 1, '/') + 1);
    }

    return SUCCESS;    
}

/**
 * This method searches for an entry in a given path.
 * 
 * @param camino_parcial
 * @param p_inodo_dir
 * @param p_inodo
 * @param p_entrada
 * @param reservar
 * @param permisos
 * 
 * @return 0 if the entry is found, -1 otherwise
*/
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {

    struct entrada entrada;

    super_bloque_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    if (camino_parcial[0] != '/') {
        // Root always be associated with the inode 0
        *p_inodo = sb.posInodoRaiz;
        *p_entrada = 0;

        return SUCCESS;
    }

    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;

    if (extraer_camino(camino_parcial, inicial, final, &tipo) == FAILURE) {
        return ERROR_CAMINO_INCORRECTO;
    }

    // Search for the entry in the root directory
    inodo_t inodo_dir;
    leer_inodo(*p_inodo_dir, &inodo_dir);
    if ( (inodo_dir.permisos & 4) != 4 ) {
        return ERROR_PERMISO_LECTURA;
    }

    // Initialize the buffer with 0
    struct entrada read_buffer[BLOCKSIZE / sizeof(struct entrada)];
    memset(read_buffer, 0, (BLOCKSIZE / sizeof(struct entrada)) * sizeof(struct entrada));

    // Calculate the number of entries in the inode
    int cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    int num_entrada_inodo = 0;

    // Read the entries in the inode
    if (cant_entradas_inodo > 0) {
        // Improvement: read the entries in blocks
        int bytes_leidos = mi_read_f(*p_inodo_dir, read_buffer, 0, BLOCKSIZE);

        // Search for the entry
        while ( (num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, read_buffer[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].nombre) != 0)) {
            num_entrada_inodo++;

            // Read the next block
            if (num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada)) == 0) {
                memset(read_buffer, 0, sizeof(read_buffer));
                bytes_leidos += mi_read_f(*p_inodo_dir, read_buffer, bytes_leidos, BLOCKSIZE);
            }
        }
    }

    // If the entry is not found
    if ( (inicial != entrada.nombre) && (num_entrada_inodo = cant_entradas_inodo)) {
        switch (reservar) {
            case 0: 
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            case 1:
                if (inodo_dir.tipo = 'f') {
                    return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
                }
                
                if ( (inodo_dir.permisos & 2) != 2) {
                    return ERROR_PERMISO_ESCRITURA;
                } else {
                    strcpy(entrada.nombre, inicial);

                    if (tipo == 'd') {
                        if (strcmp(inicial, "/") != 0) {
                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }

                        entrada.ninodo = reservar_inodo('d', permisos);

                    } else {
                        entrada.ninodo = reservar_inodo('f', permisos);
                    }
                }

                // Write the entry in the inode
                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == FAILURE) {
                    if (entrada.ninodo != -1) {
                        liberar_inodo(entrada.ninodo);
                    }
                
                    return FAILURE;
                }
            
        }
    }

    // We arrive at the end of the path
    if ((strcmp(final, "/")) == 0) {
        if ( (num_entrada_inodo < cant_entradas_inodo) && (reservar = 1) ) {
            return ERROR_ENTRADA_YA_EXISTENTE;
        }

        // Cut the recursive call
        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;

        return SUCCESS;
    } else {
        *p_inodo_dir = entrada.ninodo;

        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }

    return SUCCESS;
}

/**
 * This method shows an error message depending of the error.
 * 
 * @param error
*/
void mostrar_error_buscar_entrada(int error) {
    switch (error) {
        case -2: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
        case -3: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
        case -4: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
        case -5: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
        case -6: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
        case -7: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
        case -8: fprintf(stderr, "Error: No es un directorio.\n"); break;
    }
}