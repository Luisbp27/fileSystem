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

    char *rest = strchr(camino + 1, '/');

    if (rest) {
        strncpy(inicial, camino + 1, strlen(camino) - strlen(rest) - 1);
        strcpy(final, rest);

        *tipo = final[0] == '/' ? 'd' : 'f';
    } else {
        strcpy(inicial, camino + 1);
        strcpy(final, "");
        *tipo = 'f';
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
    struct entrada entrada = {
        .nombre = {0},
        .ninodo = 0,
    };

    if (strcmp(camino_parcial, "/") == 0) {
        super_bloque_t sb;
        if (bread(POS_SB, &sb) == FAILURE) {
            return FAILURE;
        }

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

#if DEBUGIMPORTANT
    printf("[buscar_entrada()->inicial: %s, final: %s, reservar: %d]\n", inicial,
           final, reservar);
#endif

    // Search for the entry in the root directory
    inodo_t inodo_dir;
    if (leer_inodo(*p_inodo_dir, &inodo_dir) == FAILURE) {
        return FAILURE;
    }

    if ((inodo_dir.permisos & 4) != 4) {
        return ERROR_PERMISO_LECTURA;
    }

    // Initialize the buffer with 0
    struct entrada read_buffer[BLOCKSIZE / sizeof(struct entrada)];
    if (!memset(read_buffer, 0, (BLOCKSIZE / sizeof(struct entrada)) * sizeof(struct entrada))) {
        return FAILURE;
    }

    // Calculate the number of entries in the inode
    int cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    int num_entrada_inodo = 0;

    // Read the entries in the inode
    if (cant_entradas_inodo > 0) {
        if ((inodo_dir.permisos & 4) != 4) {
            return ERROR_PERMISO_LECTURA;
        }

        if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == FAILURE) {
            return FAILURE;
        }

        while (num_entrada_inodo < cant_entradas_inodo && strcmp(inicial, entrada.nombre) != 0) {
            num_entrada_inodo++;
            if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == FAILURE)
                return FAILURE;
        }
    }

    // If the entry is not found
    if (num_entrada_inodo == cant_entradas_inodo && (inicial != read_buffer[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].nombre)) {
        switch (reservar) {
            case 0:
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            case 1:
                if (inodo_dir.tipo == 'f') {
                    return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
                }

                if ((inodo_dir.permisos & 2) != 2) {
                    return ERROR_PERMISO_ESCRITURA;
                }

                strcpy(entrada.nombre, inicial);

                if (tipo == 'd' && strcmp(final, "/") != 0) {
                    return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                }

                entrada.ninodo = reservar_inodo(tipo, permisos);

#if DEBUGIMPORTANT
                printf("[buscar_entrada()->reservado inodo: %d tipo %c con permisos %d para '%s']\n", entrada.ninodo, tipo, permisos, entrada.nombre);
#endif

#if DEBUGIMPORTANT
                fprintf(stderr, "[buscar_entrada()->creada entrada: %s, %d] \n", inicial, entrada.ninodo);
#endif

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
    if ((strcmp(final, "/")) == 0 || strcmp(final, "") == 0) {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1)) {
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
}

/**
 * This method shows an error message depending of the error.
 *
 * @param error
 */
void mostrar_error_buscar_entrada(int error) {
    switch (error) {
        case ERROR_CAMINO_INCORRECTO:
            fprintf(stderr, "Error: Camino incorrecto.\n");
            break;
        case ERROR_PERMISO_LECTURA:
            fprintf(stderr, "Error: Permiso denegado de lectura.\n");
            break;
        case ERROR_NO_EXISTE_ENTRADA_CONSULTA:
            fprintf(stderr, "Error: No existe el archivo o el directorio.\n");
            break;
        case ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO:
            fprintf(stderr, "Error: No existe algún directorio intermedio.\n");
            break;
        case ERROR_PERMISO_ESCRITURA:
            fprintf(stderr, "Error: Permiso denegado de escritura.\n");
            break;
        case ERROR_ENTRADA_YA_EXISTENTE:
            fprintf(stderr, "Error: El archivo ya existe.\n");
            break;
        case ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO:
            fprintf(stderr, "Error: No es un directorio.\n");
            break;
        default:
            fprintf(stderr, "Error: Desconocido (código: %d)", error);
            break;
    }
}
