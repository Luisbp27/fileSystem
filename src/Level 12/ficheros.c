#include "ficheros.h"

/**
 * This method writes the contents of a memory buffer, original_buf, of size nbytes, to a file/directory.
 *
 * @param ninodo
 * @param buf_original
 * @param offset
 * @param nbytes
 *
 * @return
 */
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes) {
    inodo_t inodo;

    if (leer_inodo(ninodo, &inodo) == FAILURE) {
        mi_signalSem();
        return FAILURE;
    }

    // Check writing perms
    if ((inodo.permisos & 2) != 2) {
        return FAILURE;
    }

    int bytes_escritos = 0;
    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    unsigned char buf_bloque[BLOCKSIZE];

    mi_waitSem();
    int nb_fisico = traducir_bloque_inodo(&inodo, primerBL, 1);
    mi_signalSem();
    // Fits in a single block
    if (primerBL == ultimoBL) {
        if (bread(nb_fisico, buf_bloque) == FAILURE) {
            return FAILURE;
        }

        memcpy(buf_bloque + desp1, buf_original, nbytes);

        if (bwrite(nb_fisico, buf_bloque) == FAILURE) {
            return FAILURE;
        }

        bytes_escritos += nbytes;

    } else { // We need more than one block
        // First logical block
        if (bread(nb_fisico, buf_bloque) == FAILURE) {
            return FAILURE;
        }

        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);

        if (bwrite(nb_fisico, buf_bloque) == FAILURE) {
            return FAILURE;
        }

        bytes_escritos += BLOCKSIZE - desp1;

        // Intermediate logical blocks
        for (int i = primerBL + 1; i < ultimoBL; i++) {
            mi_waitSem();
            nb_fisico = traducir_bloque_inodo(&inodo, i, 1);
            mi_signalSem();

            if (bwrite(nb_fisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE) == -1) {
                return FAILURE;
            }

            bytes_escritos += BLOCKSIZE;
        }

        // Last logical block
        mi_waitSem();
        nb_fisico = traducir_bloque_inodo(&inodo, ultimoBL, 1);
        mi_signalSem();

        if (bread(nb_fisico, buf_bloque) == FAILURE) {
            return FAILURE;
        }

        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);

        if (bwrite(nb_fisico, buf_bloque) == FAILURE) {
            return FAILURE;
        }

        bytes_escritos += desp2 + 1;
    }

    if (inodo.tamEnBytesLog < bytes_escritos + offset) {
        inodo.tamEnBytesLog = bytes_escritos + offset;
        inodo.ctime = time(NULL);
    }

    inodo.mtime = time(NULL);

    mi_waitSem();
    if (escribir_inodo(ninodo, &inodo) == FAILURE) {
        return FAILURE;
    }
    mi_signalSem();

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
    int bytes_leidos = 0;

    inodo_t inodo;
    if (leer_inodo(ninodo, &inodo) == FAILURE) {
        return FAILURE;
    }

    // Check read perms
    if ((inodo.permisos & 4) != 4) {
        return FAILURE;
    }

    // Update atime
    inodo.atime = time(NULL);
    mi_waitSem();
    if (escribir_inodo(ninodo, &inodo) == FAILURE) {
        return FAILURE;
    }
    mi_signalSem();

    // Nothing else to write
    if (offset >= inodo.tamEnBytesLog) {
        return bytes_leidos;
    }

    // If offset overshoots, cap it
    if (offset + nbytes >= inodo.tamEnBytesLog) {
        nbytes = inodo.tamEnBytesLog - offset;
    }

    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    int desp1 = offset % BLOCKSIZE;

    unsigned char buf_bloque[BLOCKSIZE];

    int nb_fisico = traducir_bloque_inodo(&inodo, primerBL, 0);

    // Fits in a single block
    if (primerBL == ultimoBL) {
        if (nb_fisico != FAILURE) {
            if (bread(nb_fisico, buf_bloque) == FAILURE) {
                return FAILURE;
            }

            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }

        bytes_leidos = nbytes;

    } else { // We need more than one block

        // First block
        if (nb_fisico != FAILURE) {
            if (bread(nb_fisico, buf_bloque) == FAILURE) {
                return FAILURE;
            }

            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1); // Leemos el primer bloque
        }

        bytes_leidos = BLOCKSIZE - desp1;

        // Bloques intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++) {
            nb_fisico = traducir_bloque_inodo(&inodo, i, 0);

            if (nb_fisico != FAILURE) {
                if (bread(nb_fisico, buf_bloque) == FAILURE) {
                    return FAILURE;
                }

                memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE); // leemos todo el bloque
            }

            bytes_leidos += BLOCKSIZE;
        }

        // Ultimo bloque
        int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

        nb_fisico = traducir_bloque_inodo(&inodo, ultimoBL, 0);

        if (nb_fisico != FAILURE) {
            if (bread(nb_fisico, buf_bloque) == FAILURE) {
                return FAILURE;
            }

            memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1);
        }

        bytes_leidos += desp2 + 1;
    }

    return bytes_leidos;
}

/**
 * This method
 *
 * @param ninodo
 * @param p_stat
 *
 * @return Meta-information of a file/directory (corresponding to the inode number passed as argument)
 */
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat) {
    inodo_t inodo;
    if (leer_inodo(ninodo, &inodo)) {
        return FAILURE;
    }

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
 * @return
 */
int mi_chmod_f(unsigned int ninodo, unsigned char permisos) {
    mi_waitSem();

    inodo_t inodo;
    if (leer_inodo(ninodo, &inodo) == FAILURE) {
        return FAILURE;
    }

    inodo.permisos = permisos;
    inodo.ctime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FAILURE) {
        mi_signalSem();
        return FAILURE;
    }

    mi_signalSem();
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
