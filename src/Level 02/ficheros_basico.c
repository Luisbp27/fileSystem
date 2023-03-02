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
    super_block_t sb;

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
    super_block_t sb;
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
    inode_t inodes[BLOCKSIZE / INODESIZE];

    super_block_t sb;
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
