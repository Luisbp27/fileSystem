#include "ficheros.h"

/**
 * This method writes the contents of a memory buffer, original_buf, of size nbytes, to a file/directory.
 *
 * @param ninodo
 * @param buf_original
 * @param offset
 * @param nbytes
 *
 * @return bytes written
 */
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes) {
    inodo_t inodo;
    if (leer_inodo(ninodo, &inodo) == FAILURE) {
        return FAILURE;
    }

    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    unsigned int nb_fisico = traducir_bloque_inodo(&inodo, primerBL, 1);
    if (escribir_inodo(ninodo, &inodo) == FAILURE) {
        return FAILURE;
    }

    unsigned char buf_bloque[BLOCKSIZE];
    unsigned int bytes_escritos = 0;

    if ((inodo.permisos & 2) != 2) {
        fprintf(stderr, "The inode %d does not have write permissions", ninodo);
        return FAILURE;
    }

    // Can write in one block
    if (primerBL == ultimoBL) {
        if (bread(nb_fisico, buf_bloque) == FAILURE) {
            return FAILURE;
        }

        // Write nbytes
        memcpy(buf_bloque + desp1, buf_original, nbytes);

        if (bwrite(nb_fisico, buf_bloque) == FAILURE) {
            return FAILURE;
        }

        bytes_escritos += nbytes;

        // We need more than one block
    } else {
        // First logic block (8)
        if (bread(nb_fisico, buf_bloque) == FAILURE) {
            return FAILURE;
        }

        // memcpy(buf_bloque + 808, buf_original, 216);
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);

        if (bwrite(nb_fisico, buf_bloque) == FAILURE) {
            return FAILURE;
        }

        bytes_escritos += BLOCKSIZE - desp1;

        // Intermediate logic blocks (9, 10, 11)
        for (int i = primerBL + 1; i < ultimoBL; i++) {
            if (bwrite(nb_fisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE) == FAILURE) {
                return FAILURE;
            }

            bytes_escritos += BLOCKSIZE;
        }

        // Last logic block (12)
        if (bread(nb_fisico, buf_bloque) == FAILURE) {
            return FAILURE;
        }

        // memcpy (buf_bloque, buf_original + (3751 – 462 - 1), 462 + 1)
        memcpy(buf_bloque, buf_original + (nbytes - (desp2 + 1)), desp2 + 1);

        if (bwrite(nb_fisico, buf_bloque) == FAILURE) {
            return FAILURE;
        }

        bytes_escritos += desp2 + 1;
    }

    // Update inode
    leer_inodo(ninodo, &inodo);

    if (inodo.tamEnBytesLog < (bytes_escritos + offset)) {
        inodo.tamEnBytesLog = bytes_escritos + offset;
        inodo.ctime = time(NULL);
    }

    inodo.mtime = time(NULL);

    escribir_inodo(ninodo, &inodo);

    return bytes_escritos;
}

/**
 * This method reads information from a file/directory (corresponding to the inode number, node, passed as argument)
 * and stores it in a memory buffer.
 *
 * @param ninodo
 * @param buf_original
 * @param offset
 * @param nbytes
 *
 * @return Number of bytes actually written
 */
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes) {
    unsigned int bytes_leidos = 0;

    inodo_t inodo;
    if (leer_inodo(ninodo, &inodo) == FAILURE) {
        return bytes_leidos;
    }
    inodo.atime = time(NULL);

    if ((inodo.permisos & 4) != 4) {
        return bytes_leidos;
    }

    printf("\n\n offset: %d y tamEnBytesLog: %d \n\n", offset, inodo.tamEnBytesLog);
    if (offset >= inodo.tamEnBytesLog) {
        return bytes_leidos;
    } 

    if ((offset + nbytes) >= inodo.tamEnBytesLog) {
        nbytes = inodo.tamEnBytesLog - offset;
    }

    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int nb_fisico = traducir_bloque_inodo(&inodo, primerBL, 0);

    unsigned int desp1 = offset & BLOCKSIZE;
    unsigned char buf_bloque[BLOCKSIZE];

    // Can write in one block
    if (primerBL == ultimoBL) {
        if (nb_fisico != FAILURE) {
            if (bread(nb_fisico, buf_bloque) == FAILURE) {
                return FAILURE;
            }

            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }

        bytes_leidos = nbytes;

    // We need more than one block
    } else {
        // First logic block (8)
        if (nb_fisico != FAILURE) {
            if (bread(nb_fisico, buf_bloque) == FAILURE) {
                return FAILURE;
            }

            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }

        bytes_leidos = BLOCKSIZE - desp1;

        // Intermediate logic blocks (9, 10, 11)
        for (int i = primerBL + 1; i < ultimoBL; i++) {
            nb_fisico = traducir_bloque_inodo(&inodo, i, 0);

            if (nb_fisico != FAILURE) {
                if (bread(nb_fisico, buf_bloque) == FAILURE) {
                    return FAILURE;
                }

                memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }

            bytes_leidos += BLOCKSIZE;
        }

        // Last logic block (12)
        unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
        nb_fisico = traducir_bloque_inodo(&inodo, ultimoBL, 0);

        if (nb_fisico != -1) {
            if (bread(nb_fisico, buf_bloque) == FAILURE) {
                return FAILURE;
            }

            memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1);
        }

        bytes_leidos += desp2 + 1;
    } 

    inodo.atime = time(NULL);

    return bytes_leidos;
}

/**
 * This method returns the meta-information of a file/directory (corresponding to the inode number passed as argument)
 *
 * @param ninodo
 * @param p_stat
 *
 * @return Meta-information of a file/directory
 */
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat) {

    inodo_t inodo;
    leer_inodo(ninodo, &inodo);

    // Fill the structure
    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;

    p_stat->atime = inodo.atime;
    p_stat->ctime = inodo.ctime;
    p_stat->mtime = inodo.mtime;

    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;
    p_stat->nlinks = inodo.nlinks;

    return SUCCESS;
}

/**
 * This method changes the permissions of a file/directory (corresponding to the inode number passed as argument, ninodo)
 * with the value given by the argument permisos.
 *
 * @param ninodo
 * @param permisos
 *
 * @return -1 if there is an error, 0 otherwise
 */
int mi_chmod_f(unsigned int ninodo, unsigned char permisos) {
    inodo_t inodo;

    leer_inodo(ninodo, &inodo);
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);
    escribir_inodo(ninodo, &inodo);

    return SUCCESS;
}

/**
 * This method truncates a file/directory (corresponding to the inode number passed as argument, ninodo)
 * 
 * @param ninodo
 * @param nbytes
 * 
 * @return -1 if there is an error or free blocks
 */
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes) {

    inodo_t inodo;
    if (leer_inodo(ninodo, &inodo) == FAILURE) {
        return FAILURE;
    }

    // Checking inode permissions
    if ((inodo.permisos & 2) != 2) {
        fprintf(stderr, "The inode %d does not have write permissions", ninodo);
        return FAILURE;
    }

    // Checking the size 
    if (nbytes > inodo.tamEnBytesLog) {
        fprintf(stderr, "The new size is greater than the current size");
        return FAILURE;
    }
    
    // Obtaining the number of logical block
    int primerBL;
    if (nbytes % BLOCKSIZE == 0) {
        primerBL = nbytes / BLOCKSIZE;
    } else {
        primerBL = nbytes / BLOCKSIZE + 1;
    }

    // Freeing the blocks
    int liberados = 0;
    liberados = liberar_bloques_inodo(primerBL, &inodo);

    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);

    // Updating the inode
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados -= liberados;

    if (escribir_inodo(ninodo, &inodo) == FAILURE) {
        return FAILURE;
    }
        
    return liberados;
}
