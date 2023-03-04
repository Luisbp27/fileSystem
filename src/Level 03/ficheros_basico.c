#include "ficheros_basico.h"

/**
 * This method calculates the size in blocks required for the bitmap
 *
 * @return  Size in blocks of the bitmap
 */
int tamMB(unsigned int nbloques) {
    int map_size = (nbloques / 8) / BLOCKSIZE;

    if ((nbloques / 8) % BLOCKSIZE > 0) {
        return map_size + 1;
    }

    return map_size;
}

/**
 * This method calculates the block size of the inode array.
 *
 * @return Size of the inode array
 */
int tamAI(unsigned int ninodos) {
    int inode_bytes = (ninodos * INODESIZE);
    int size_ai = inode_bytes / BLOCKSIZE;

    if (inode_bytes % BLOCKSIZE > 0) {
        size_ai++;
    }

    return size_ai;
}

int initSB(unsigned int nbloques, unsigned int ninodos) {
    super_bloque_t sb;

    // Position of the first block in the bitmap
    sb.posPrimerBloqueMB = POS_SB + TAM_SB;
    // Position of the last block in the bitmap
    sb.posUltimoBloqueMB = sb.posPrimerBloqueMB + tamMB(nbloques) - 1;
    // Position of the first block in the inode array
    sb.posPrimerBloqueAI = sb.posUltimoBloqueMB + 1;
    // Position of the last block in the inode array
    sb.posUltimoBloqueAI = sb.posPrimerBloqueAI + tamAI(ninodos) - 1;
    // Position of the first data block
    sb.posPrimerBloqueDatos = sb.posUltimoBloqueAI + 1;
    // Position of the last data block
    sb.posUltimoBloqueDatos = nbloques - 1;
    // Position of the root directory of the inode array
    sb.posInodoRaiz = 0;
    // Position of the first free inode
    sb.posPrimerInodoLibre = 0;
    // Number of free blocks in SF
    sb.cantBloquesLibres = nbloques;
    // Number of free inodes
    sb.cantInodosLibres = ninodos;
    // Total number of blocks
    sb.totBloques = nbloques;
    // Total number of inodes
    sb.totInodos = ninodos;

    // Write de sb on the filesystem
    if (bwrite(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}

int initMB() {
    super_bloque_t sb;
    // Reading the sb
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }
    // Number of blocks occupied by metadata
    unsigned int metadata_blocks = sb.posUltimoBloqueMB + 1;
    // Number of bytes set to all 1s because their 8 bits are used for metadata
    unsigned int all_ones_bytes = metadata_blocks / 8;
    // Number of blocks set to all 1s
    unsigned int full_ones_blocks = all_ones_bytes / BLOCKSIZE;
    // Number of bits corresponding to a metadata block that can't be grouped in a byte
    unsigned int partial_ones = metadata_blocks % 8;

    unsigned char buffer[BLOCKSIZE];

    if (!memset(buffer, UCHAR_MAX, sizeof(buffer))) {
        perror("Bitmap initialization has not been done correctly");
        return FAILURE;
    }

    for (unsigned int i = 0; i < full_ones_blocks; i++) {
        if (bwrite(sb.posPrimerBloqueMB + i * BLOCKSIZE, buffer) == FAILURE) {
            fprintf(stderr, "Could not initialize bitmap");
            return FAILURE;
        }
    }

    // Reuse 1s set in the previous memset, and set the trailing 0s that are
    // needed
    if (!memset(&buffer[all_ones_bytes], 0, sizeof(buffer) - all_ones_bytes)) {
        perror("Bitmap initialization has not been done correctly");
        return FAILURE;
    }

    unsigned char bits_left = UCHAR_MAX << (8 - partial_ones);
    buffer[all_ones_bytes] = bits_left;

    if (bwrite(sb.posPrimerBloqueMB + full_ones_blocks, buffer) == FAILURE) {
        fprintf(stderr, "Could not initialize bitmap");
        return FAILURE;
    }

    return SUCCESS;
}

int initAI() {
    inodo_t inodes[BLOCKSIZE / INODESIZE];

    super_bloque_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    unsigned int inode_counter = sb.posPrimerInodoLibre + 1;

    for (unsigned int i = sb.posPrimerBloqueAI; i <= sb.posUltimoBloqueAI; i++) {
        // Read the inode block from the filesystem
        for (int j = 0; j < BLOCKSIZE / INODESIZE; j++) {
            inodes[j].tipo = FREE;

            if (inode_counter < sb.totInodos) {
                inodes[j].punterosDirectos[0] = inode_counter;
                inode_counter++;
            } else {
                inodes[j].punterosIndirectos[0] = UINT_MAX;
                break;
            }
        }

        // Write the inode block and in the FS
        if (bwrite(i, &inodes) == FAILURE) {
            return FAILURE;
        }
    }

    return SUCCESS;
}

/**
 * Calculates the necessary values to read or write a byte.
 * 
 * @param nbloque Block whose MB we want to read.
 * @param posbyte (return value) Position of the byte inside the MB block
 * @param posbit (return value) Position of the bit inside the byte
 * @param nbloqueabs (return value) Absolute position of the block
 *
 * @return -1 if there was an error, 0 otherwise
*/
int calc_bit(unsigned int nbloque, unsigned int *posbyte, unsigned int *posbit, unsigned int *nbloqueabs) {
    super_bloque_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    *posbyte = nbloque / 8; // posición absoluta del byte que conteiene el bit del MB
    *posbit = nbloque % 8; // posición del bit a modificar dentro del byte
    
    unsigned int nbloqueMB = *posbyte / BLOCKSIZE; // posición del bloque relativa al comienzo del MB
    *nbloqueabs = sb.posPrimerBloqueMB + nbloqueMB; // posición absoluta del bloque

    *posbyte = *posbyte % BLOCKSIZE; // posición del byte dentro del bloque leído

    return SUCCESS;
}

/**
 * Writes `bit` in the MB bit corresponding tho the block `nbloque`
 *
 * @param nbloque
 * @param bit
 *
 * @return -1 if there was an error, 0 otherwise
*/
int escribir_bit(unsigned int nbloque, unsigned int bit) {
    unsigned int posbyte, posbit, nbloqueabs;
    if (calc_bit(nbloque, &posbyte, &posbit, &nbloqueabs) == FAILURE) {
        return FAILURE;
    }
    
    unsigned char buffer[BLOCKSIZE];
    if (bread(nbloqueabs, &buffer) == FAILURE) {
        return FAILURE;
    }
    
    unsigned char mascara = 0b10000000 >> posbit;

    if (bit == 0) {
        buffer[posbyte] &= ~mascara;
    } else if (bit == 0) {
        buffer[posbyte] |= mascara;
    } else {
        return FAILURE;
    }

    if (bwrite(nbloqueabs, &buffer) == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}


/**
 * Reads the bit in the MB bit corresponding tho the block `nbloque`
 *
 * @param nbloque
 * @param bit
 *
 * @return -1 if there was an error, the value of the bit otherwise
*/
char leer_bit(unsigned int nbloque) {
    unsigned int posbyte, posbit, nbloqueabs;
    if (calc_bit(nbloque, &posbyte, &posbit, &nbloqueabs) == FAILURE) {
        return FAILURE;
    }
    
    unsigned char buffer[BLOCKSIZE];
    if (bread(nbloqueabs, &buffer) == FAILURE) {
        return FAILURE;
    }

    unsigned char mascara = 0b10000000 >> posbit;
    unsigned char result = mascara & buffer[posbyte];
    result >>= (7 - posbit);
    
    return result;
}

/**
 * This method finds the first free block, queries the MB, 
 * occupies it and returns its position.
 * 
 * @return The position of block, or -1 if there was an error.
*/
int reservar_bloque() {
    // Reading the superblock of the virtual device
    super_bloque_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    // Check free blocks
    if (sb.cantBloquesLibres == 0) {
        return FAILURE;
    }

    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    unsigned int posBloqueMB = sb.posPrimerBloqueMB;

    // Set 1s the bufferAux
    if (!memset(bufferAux, UCHAR_MAX, BLOCKSIZE)) {
        return FAILURE;
    }

    // Locate the first free block
    int found = 0;
    while(found != 1) {
        if (bread(posBloqueMB, bufferMB) == FAILURE) {
            return FAILURE;
        }

        if (memcmp(bufferMB, bufferAux, BLOCKSIZE) != 0) {
            found = 1;
            break;
        }
        
        posBloqueMB++;
    }

    // Locate the 0 byte inside the founded free block
    unsigned int posbyte = 0;
    while (bufferMB[posbyte] == UCHAR_MAX) {
        posbyte++;
    }

    // Locate the 0 bit inside the founded byte
    unsigned char mascara = 0b10000000; // 128
    unsigned int posbit = 0;
    while (bufferMB[posbyte] & mascara) {
        bufferMB[posbyte] <<= 1;
        posbit++;
    }

    // Absolute position of the bit inside the virtual device
    unsigned int nbloque = ((posBloqueMB - sb.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;

    // Allocate the bit
    if (escribir_bit(nbloque, 1) == FAILURE) {
        return FAILURE;
    }

    // Decrement the number of free blocks
    sb.cantBloquesLibres--;

    // Fill the buffer with 0s to prevent trash on memory
    if (!memset(bufferAux, 0, BLOCKSIZE)) {
        return FAILURE;
    }

    // Save the superblock
    if (bwrite(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    return nbloque;
}

int liberar_bloque(unsigned int nbloque) {
    // Reading the superblock
    super_bloque_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    escribir_bit(nbloque, 0);
    sb.cantBloquesLibres++;
    
    // Save the superblock
    if (bwrite(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    return nbloque;
}

int escribir_inodo(unsigned int ninodo, inodo_t *inodo) {
    super_bloque_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    // Find the inode inside the AI
    unsigned int posBloqueAI = sb.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODESIZE));
    
    // Reading inode
    inodo_t inodos_buffer[BLOCKSIZE / BLOCKSIZE];
    if (bread(posBloqueAI, inodos_buffer) == FAILURE) {
        return FAILURE;
    }

    inodos_buffer[ninodo % (BLOCKSIZE / BLOCKSIZE)] = *inodo;

    inodo_t inodos_buffer[BLOCKSIZE / BLOCKSIZE];
    if (bwrite(posBloqueAI, inodos_buffer) == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}

int leer_inodo(unsigned int ninodo, inodo_t *inodo) {
    // Reading the superblock
    super_bloque_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }
    
    // Find the inode inside the AI
    unsigned int posBloqueAI = sb.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODESIZE));

    // Reading inode
    inodo_t inodos_buffer[BLOCKSIZE / BLOCKSIZE];
    if (bread(posBloqueAI, inodos_buffer) == FAILURE) {
        return FAILURE;
    }

    // Writing inode in it's position in the array
    *inodo = inodos_buffer[ninodo % (BLOCKSIZE / BLOCKSIZE)];

    return SUCCESS;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos) {
    
}