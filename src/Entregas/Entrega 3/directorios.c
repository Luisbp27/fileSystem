#include "directorios.h"

static struct CacheEntry tabla_cache[MAX_CACHE];
int cache_length = 0;
int cache_start = 0;

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
 * @return
 */
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {
    struct entrada entrada = {
        .nombre = {0},
        .ninodo = 0,
    };

    // Check if it is the root directory
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
    memset(inicial, 0, sizeof(inicial));
    char final[strlen(camino_parcial)];
    memset(final, 0, sizeof(final));
    char tipo;

    if (extraer_camino(camino_parcial, inicial, final, &tipo) == FAILURE) {
        return ERROR_CAMINO_INCORRECTO;
    }

    #if DEBUGENTREGA1
        printf("[buscar_entrada()->inicial: %s, final: %s, reservar: %d]\n", inicial, final, reservar);
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
            if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == FAILURE) {
                return FAILURE;
            }
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

#if DEBUGENTREGA1
                printf("[buscar_entrada()->reservado inodo: %d tipo %c con permisos %d para %s]\n", entrada.ninodo, tipo, permisos, entrada.nombre);
#endif

#if DEBUGENTREGA1
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
            fprintf(stderr, "Error: Desconocido (código: %d) \n", error);
            break;
    }
}

int mi_creat(const char *camino, unsigned char permisos) {
    mi_waitSem();

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error;
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos)) < 0) {
        mostrar_error_buscar_entrada(error);

        mi_signalSem();
        return FAILURE;
    }

    mi_signalSem();
    return SUCCESS;
}

void print_entrada_extended(char *buffer, inodo_t *inodo, struct entrada *entrada) {
    struct tm *tm;
    char time[100];
    // An unsigned int can have 10 digits at max
    char tamEnBytes[10];

    // Type
    if (inodo->tipo == 'd') {
        strcat(buffer, MAGENTA);
        strcat(buffer, "d");
    } else {
        strcat(buffer, CYAN);
        strcat(buffer, "f");
    }
    strcat(buffer, "\t");

    // Perms
    strcat(buffer, BLUE);
    strcat(buffer, ((inodo->permisos & 4) == 4) ? "r" : "-");
    strcat(buffer, ((inodo->permisos & 2) == 2) ? "w" : "-");
    strcat(buffer, ((inodo->permisos & 1) == 1) ? "x" : "-");
    strcat(buffer, "\t");

    // mTime
    strcat(buffer, YELLOW);
    tm = localtime(&inodo->mtime);
    sprintf(time, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    strcat(buffer, time);
    strcat(buffer, "\t");

    // Tamaño
    strcat(buffer, LBLUE);
    sprintf(tamEnBytes, "%d", inodo->tamEnBytesLog);
    strcat(buffer, tamEnBytes);
    strcat(buffer, "\t");

    // Nombre
    strcat(buffer, LRED);
    strcat(buffer, entrada->nombre);
    while ((strlen(buffer) % TAMFILA) != 0) {
        strcat(buffer, " ");
    }

    strcat(buffer, RESET);
    // Preparamos el string para la siguiente entrada
    strcat(buffer, "\n");
}

int mi_dir(const char *camino, char *buffer, char *tipo, int extended) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int nEntradas = 0;

    int error;
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0) {
        mostrar_error_buscar_entrada(error);
        return FAILURE;
    }

    inodo_t inodo;
    if (leer_inodo(p_inodo, &inodo) == FAILURE) {
        return FAILURE;
    }

    if ((inodo.permisos & 4) != 4) {
        return -1;
    }

    if (leer_inodo(p_inodo, &inodo) == FAILURE) {
        return FAILURE;
    }
    *tipo = inodo.tipo;

    if (inodo.tipo == 'd') {
        // Buffer de salida
        struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
        memset(&entradas, 0, sizeof(struct entrada));

        nEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);

        int offset = 0;
        offset = mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);

        // Build each entry
        for (int i = 0; i < nEntradas; i++) {
            if (leer_inodo(entradas[i % (BLOCKSIZE / sizeof(struct entrada))].ninodo, &inodo) == FAILURE) {
                return FAILURE;
            }

            if (extended) {
                print_entrada_extended(buffer, &inodo, &entradas[i % (BLOCKSIZE / sizeof(struct entrada))]);
            } else {
                if (inodo.tipo == 'd') {
                    strcat(buffer, MAGENTA);
                } else {
                    strcat(buffer, CYAN);
                }

                strcat(buffer, entradas[i % (BLOCKSIZE / sizeof(struct entrada))].nombre);
                strcat(buffer, RESET);
                strcat(buffer, "\t");
            }

            if (offset % (BLOCKSIZE / sizeof(struct entrada)) == 0) {
                offset += mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);
            }
        }
    } else {
        nEntradas = 1;

        struct entrada entrada;
        mi_read_f(p_inodo_dir, &entrada, sizeof(struct entrada) * p_entrada, sizeof(struct entrada));
        leer_inodo(entrada.ninodo, &inodo);

        print_entrada_extended(buffer, &inodo, &entrada);
    }

    return nEntradas;
}

/**
 * This method changes the permisions of a file or directory
 *
 * @param camino The path to the file or directory
 * @param permisos The new permisions
 *
 * @return 0 if success, -1 if error
 */
int mi_chmod(const char *camino, unsigned char permisos) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error;
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos)) < 0) {
        mostrar_error_buscar_entrada(error);
        return error;
    }

    if (mi_chmod_f(p_inodo, permisos) == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}

/**
 * This method allow us to visualize the information of the inode of the path given
 *
 * @param camino The path to the file or directory
 * @param p_stat The struct where we will save the information
 *
 * @return 0 if success, -1 if error
 */
int mi_stat(const char *camino, struct STAT *p_stat) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error;
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, p_stat->permisos)) < 0) {
        mostrar_error_buscar_entrada(error);
        return error;
    }

    if (mi_stat_f(p_inodo, p_stat) < 0) {
        return FAILURE;
    }

    return p_inodo;
}

int timeval_cmp(struct timeval t1, struct timeval t2) {
    if (t1.tv_sec > t2.tv_sec) {
        return 1;  // t1 is greater
    } else if (t1.tv_sec < t2.tv_sec) {
        return -1; // t2 is greater
    } else {
        if (t1.tv_usec > t2.tv_usec) {
            return 1;  // t1 is greater
        } else if (t1.tv_usec < t2.tv_usec) {
            return -1; // t2 is greater
        } else {
            return 0;  // t1 and t2 are equal
        }
    }
}

int cache_read(const char *camino, unsigned int *p_inodo) {
    // Cache traversal to check if the write is on a previous inode
    for (int i_raw = cache_start; i_raw < cache_start + cache_length; i_raw++) {
        // Table is treated like a circular vector, with a moving start position
        // This is made this way for FIFO, but it doesnt interfere with LRU
        int i = i_raw % cache_length;
        if (strcmp(camino, tabla_cache[i].camino) == 0) {
#if DEBUG9
    fprintf(stderr, "[cache_read() -> Utilizamos cache %d: %s]\n", i, camino);
#endif
            *p_inodo = tabla_cache[i].p_inodo;
#if LRU_CACHE
            gettimeofday(&tabla_cache[i].time, NULL);
#endif
            return 1;
        }
    }

    return 0;
}

void cache_add(const char *camino, unsigned int p_inodo) {
    // Table is treated like a circular vector, with a moving start position
    int replace_index;
    if (cache_length < MAX_CACHE) {
        replace_index = cache_length;
        cache_length++;
    } else {
#if LRU_CACHE
        // Entry with the minimum timeval will be the oldest one
        struct CacheEntry *min_entry = &tabla_cache[0];
        replace_index = 0;

        for (int i = 1; i < cache_length; i++) {
            // fprintf(stderr, "[i: %d] > max? %d\n", i, tiemval_cmp(tabla_cache[i].time, min_entry->time));
            if (timeval_cmp(tabla_cache[i].time, min_entry->time) < 0) {
                min_entry = &tabla_cache[i];
                replace_index = i;
            }
        }
#else
        replace_index = cache_start;
        cache_start = (cache_start + 1) % cache_length;
#endif
    }

#if DEBUG9
    fprintf(stderr, "[cache_add() -> Reemplazamos cache %d: %s]\n", replace_index, camino);
#endif

    strcpy(tabla_cache[replace_index].camino, camino);
    tabla_cache[replace_index].p_inodo = p_inodo;
#if LRU_CACHE
    gettimeofday(&tabla_cache[replace_index].time, NULL);
#endif
}

/**
 * This method allow us to write in a file
 *
 * @param camino The path to the file or directory
 * @param buf The buffer where we will save the information
 * @param offset The offset where we will start writing
 * @param nbytes The number of bytes to write
 *
 * @return 0 if success, -1 if error
 */
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error = 0;
    int is_in_cache = cache_read(camino, &p_inodo);

    // Check if the last entry is the same as the current one
    if (!is_in_cache) {
        // Get the inode
        error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (error < 0) {
            return error;
        }

        cache_add(camino, p_inodo);
    }


    // Write the data
    int bytes = mi_write_f(p_inodo, buf, offset, nbytes);
    if (bytes == FAILURE) {
        return ERROR_PERMISO_ESCRITURA;
    }

    return bytes;
}

/**
 * This method allow us to read from a file
 *
 * @param camino The path to the file or directory
 * @param buf The buffer where we will save the information
 * @param offset The offset where we will start writing
 * @param nbytes The number of bytes to write
 *
 * @return 0 if success, -1 if error
 */
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error = 0;
    int is_in_cache = cache_read(camino, &p_inodo);

    if (!is_in_cache) {
        // Get the inode
        error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (error < 0) {
            return error;
        }

        cache_add(camino, p_inodo);
    }

    // Read the data
    int bytes = mi_read_f(p_inodo, buf, offset, nbytes);
    if (bytes == FAILURE) {
        bytes = ERROR_PERMISO_LECTURA;
    }

    return bytes;
}

/**
 * This method create a link between input path directory and the respective inode specified in the other path directory
 *
 * @param camino1 The path to the file or directory
 * @param camino2 The path to the file or directory
 *
 * @return 0 if success, -1 if error
 */
int mi_link(const char *camino1, const char *camino2) {
    mi_waitSem();

    unsigned int p_inodo_dir1 = 0;
    unsigned int p_inodo1 = 0;
    unsigned int p_entrada1 = 0;

    // Check path 1
    int error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 6);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return FAILURE;
    }

    // Read the inode
    inodo_t inodo;
    if (leer_inodo(p_inodo1, &inodo) == FAILURE) {
        mi_signalSem();
        return FAILURE;
    }

    // Check if the input path is a file
    if (inodo.tipo != 'f') {
        fprintf(stderr, "Error: The path camino1 is not a file.\n");

        mi_signalSem();
        return FAILURE;
    }

    // Check if the input path has read permissions
    if ((inodo.permisos & 4) != 4) {
        fprintf(stderr, "Error: The path camino1 hasn't read perms.\n");

        mi_signalSem();
        return FAILURE;
    }

    unsigned int p_inodo_dir2 = 0;
    unsigned int p_inodo2 = 0;
    unsigned int p_entrada2 = 0;

    // Check path 2
    error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);

        mi_signalSem();
        return FAILURE;
    }

    struct entrada entrada2;
    if (mi_read_f(p_inodo_dir2, &entrada2, sizeof(struct entrada) * (p_entrada2), sizeof(struct entrada)) < 0) {
        mi_signalSem();
        return FAILURE;
    }

    // Create the link
    entrada2.ninodo = p_inodo1;

    // Write the link
    if (mi_write_f(p_inodo_dir2, &entrada2, sizeof(struct entrada) * (p_entrada2), sizeof(struct entrada)) < 0) {
        mi_signalSem();
        return FAILURE;
    }

    // Frees the inode
    if (liberar_inodo(p_inodo2) == FAILURE) {
        mi_signalSem();
        return FAILURE;
    }

    // Update the inode
    inodo.nlinks++;
    inodo.ctime = time(NULL);

    // Write the inode
    if (escribir_inodo(p_inodo1, &inodo) == FAILURE) {
        mi_signalSem();
        return FAILURE;
    }

    mi_signalSem();
    return SUCCESS;
}

/**
 * This method remove the link between input path directory and the respective inode specified in the other path directory
 *
 * @param camino1 The path to the file or directory
 *
 * @return 0 if success, -1 if error
 */
int mi_unlink(const char *camino) {
    mi_waitSem();

    super_bloque_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    unsigned int p_inodo_dir = sb.posInodoRaiz;
    unsigned int p_inodo = sb.posInodoRaiz;
    unsigned int p_entrada = 0;

    // Search the file to unlink
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);

        mi_signalSem();
        return FAILURE;
    }

    // Read the inode
    inodo_t inodo;
    if (leer_inodo(p_inodo, &inodo) == FAILURE) {
        mi_signalSem();
        return FAILURE;
    }

    // Check if the file is a directory and has content
    if ((inodo.tipo == 'd') && (inodo.tamEnBytesLog > 0)) {
        fprintf(stderr, "Error: The file is a directory and has content.\n");

        mi_signalSem();
        return FAILURE;

    } else {
        // Read the directory inode
        inodo_t inodo_dir;
        if (leer_inodo(p_inodo_dir, &inodo_dir) == FAILURE) {
            mi_signalSem();
            return FAILURE;
        }

        int num_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
        // If the entry is not the last one, we need to move the last entry to the deleted entry
        if (p_entrada != num_entradas_inodo - 1) {

            struct entrada entrada;
            // Read the last entry
            if (mi_read_f(p_inodo_dir, &entrada, sizeof(struct entrada) * (num_entradas_inodo - 1), sizeof(struct entrada)) < 0) {
                mi_signalSem();
                return FAILURE;
            }

            // Write the last entry in the deleted entry
            if (mi_write_f(p_inodo_dir, &entrada, sizeof(struct entrada) * p_entrada, sizeof(struct entrada)) < 0) {
                mi_signalSem();
                return FAILURE;
            }
        }

        // Delete the last entry
        if (mi_truncar_f(p_inodo_dir, sizeof(struct entrada) * (num_entradas_inodo - 1)) == FAILURE) {
            mi_signalSem();
            return FAILURE;
        }

        // Update the inode
        inodo.nlinks--;
        inodo.ctime = time(NULL);

        // If the inode has no links, we need to free it
        if (inodo.nlinks == 0) {
            if (liberar_inodo(p_inodo) == FAILURE) {
                mi_signalSem();
                return FAILURE;
            }
        } else {
            // Write the inode
            if (escribir_inodo(p_inodo, &inodo) == FAILURE) {
                mi_signalSem();
                return FAILURE;
            }
        }
    }

    mi_signalSem();
    return SUCCESS;
}

int tiemval_cmp(struct timeval t1, struct timeval t2) {
    if (t1.tv_sec > t2.tv_sec) {
        return 1;  // t1 is greater
    } else if (t1.tv_sec < t2.tv_sec) {
        return -1; // t2 is greater
    } else {
        if (t1.tv_usec > t2.tv_usec) {
            return 1;  // t1 is greater
        } else if (t1.tv_usec < t2.tv_usec) {
            return -1; // t2 is greater
        } else {
            return 0;  // t1 and t2 are equal
        }
    }
}