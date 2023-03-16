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
    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    
    unsigned int nb_fisico = traducir_bloque_inodo(ninodo, primerBL, 1);
    unsigned char buf_bloque[BLOCKSIZE];
    unsigned int bytes_escritos = 0;

    if ((inodo.permisos & 2) != 2) {
        
        // Can write in one block
        if (primerBL == ultimoBL) {
            if (bread(nb_fisico, buf_bloque) == FAILURE) {
                return FAILURE;
            }

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

    } else {
        fprintf(stderr, "The inode %d does not have write permissions", ninodo);
        return FAILURE;
    }
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

}   

/**
 * This method
 * 
 * @param ninodo
 * @param p_stat
 * 
 * @return Meta-information of a file/directory (corresponding to the inode number passed as argument)
*/
int mi_stat_f(unsigned int ninodo, struct stat_t *p_stat) {

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

}