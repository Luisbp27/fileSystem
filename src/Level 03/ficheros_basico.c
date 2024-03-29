#include "ficheros_basico.h"

/**
 * This method calculates the size in blocks required for the bitmap
 *
 * @param nbloques
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
 * @param ninodos
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

/**
 * This method initialize the superblock data
 *
 * @param nbloques
 * @param ninodos
 *
 * @return -1 if there was an error, 0 otherwise
*/
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

    return bwrite(POS_SB, &sb);
}

/**
 * This method initialize the bitmap
 *
 * @return -1 if there was an error, 0 otherwise
*/
int initMB() {
    super_bloque_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    unsigned char buffer[BLOCKSIZE];

    if (!memset(buffer, 0, sizeof(buffer))) {
        return FAILURE;
    }

    // Initialize the MB to all 0s (nothing is reserved)
    for (int i = sb.posPrimerBloqueMB; i <= sb.posUltimoBloqueAI; i++) {
        if (bwrite(i, buffer) == FAILURE) {
            return FAILURE;
        }
    }
    
    // Set the metadata blocks as reserved in the MB
    for (int i = 0; i < sb.posPrimerBloqueDatos; i++) {
        if (reservar_bloque() == FAILURE) {
            return FAILURE;
        }
    }

    return SUCCESS;
}

/**
 * This method initialize the inode array
 *
 * @return -1 if there was an error, 0 otherwise
*/
int initAI() {
    inodo_t inodes[BLOCKSIZE / INODESIZE];

    super_bloque_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    unsigned int inode_counter = sb.posPrimerInodoLibre + 1;
    int end = 0;
    
    for (unsigned int i = sb.posPrimerBloqueAI; i <= sb.posUltimoBloqueAI && !end; i++) {
        // Read the inode block from the filesystem
        for (int j = 0; j < BLOCKSIZE / INODESIZE; j++) {
            inodes[j].tipo = FREE;

            if (inode_counter < sb.totInodos) {
                inodes[j].punterosDirectos[0] = inode_counter;
                inode_counter++;
            } else {
                inodes[j].punterosIndirectos[0] = UINT_MAX;
                end = 1;
                break;
            }
        }

        // Write the inode block in the virtual device
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

    // Mask and set the bit scrolling
    unsigned char mascara = 0b10000000 >> posbit;

    if (bit == 0) {
        // AND and NOT operators
        buffer[posbyte] &= ~mascara;
    } else if (bit == 1) {
        // OR operator
        buffer[posbyte] |= mascara;
    } else {
        return FAILURE;
    }

    // Writting buffer inside the virtual device
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
    while(!found) {
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

    // Save the superblock
    if (bwrite(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    return nbloque;
}

/**
 * This method frees a given block (with the help of the function write_bit())
 *
 * @param nbloque
 *
 * @return Position of the free block
*/
int liberar_bloque(unsigned int nbloque) {
    // Reading the superblock
    super_bloque_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    if (escribir_bit(nbloque, 0) == FAILURE) {
        return FAILURE;
    }

    sb.cantBloquesLibres++;

    // Save the superblock
    if (bwrite(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    return nbloque;
}

/**
 * This method writes the contents of a struct variable of type struct inode to a given
 * inode in the array of inodes, inodes. particular inode of the array of inodes, inodes.
 *
 * @param ninodo
 * @param inodo
 *
 * @return -1 if there was an error, 0 otherwise
*/
int escribir_inodo(unsigned int ninodo, inodo_t *inodo) {
    super_bloque_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    // Find the inode inside the AI
    unsigned int posBloqueAI = sb.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODESIZE));

    // Reading inode array
    inodo_t inodos_buffer[BLOCKSIZE / INODESIZE];
    if (bread(posBloqueAI, inodos_buffer) == FAILURE) {
        return FAILURE;
    }

    // Writing the inode in the corresponding position
    inodos_buffer[ninodo % (BLOCKSIZE / INODESIZE)] = *inodo;

    // Modifying the virtual device
    if (bwrite(posBloqueAI, inodos_buffer) == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}

/**
 * This method reads a given inode from the array of inodes to dump it into a struct
 * inode variable passed by reference.
 *
 * @param ninodo
 * @param inodo
 *
 * @return -1 if there was an error, 0 otherwise
*/
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

/**
 * This method finds the first free inode (data stored in the superblock),
 * reserves it (with the help of the function write_inodo())
 *
 * @param tipo
 * @param permisos
 *
 * @return Returns the position of the reserved inode
*/
int reservar_inodo(unsigned char tipo, unsigned char permisos) {
    // Reading the superblock
    super_bloque_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    // Checking the free inodes available
    if (sb.cantInodosLibres == 0) {
        return FAILURE;
    }

    unsigned int posInodoReservado = sb.posPrimerInodoLibre;

    // Initialize inode with default values
    inodo_t inodo = {
      .tipo = tipo,
      .permisos = permisos,
      .nlinks = 1,
      .tamEnBytesLog = 0,
      .atime = time(NULL),
      .mtime = time(NULL),
      .ctime = time(NULL),
      .numBloquesOcupados = 0,
      .punterosDirectos = {0},
      .punterosIndirectos = {0},
    };

    if (escribir_inodo(posInodoReservado, &inodo) == FAILURE) {
        return FAILURE;
    }

    // Mark the next inode as free and decrease the amount of free enodes
    sb.posPrimerInodoLibre++;
    sb.cantInodosLibres--;

    if (bwrite(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    return posInodoReservado;
}