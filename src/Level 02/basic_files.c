#include "basic_files.h"

/**
 * This method calculates the size in blocks required for the bitmap
 *
 * @return  Size in blocks of the bitmap
 */
int sizeMB(unsigned int n_blocks) {
    int map_size = (n_blocks / 8) / BLOCKSIZE;

    if ((n_blocks / 8) % BLOCKSIZE > 0) {
        return map_size + 1;
    }

    return map_size;
}

/**
 * This method calculates the block size of the inode array.
 *
 * @return Size of the inode array
 */
int sizeAI(unsigned int n_inodes) {
    int size_ai = (n_inodes * INODESIZE) / BLOCKSIZE;

    if ((n_inodes * INODESIZE) % BLOCKSIZE > 0) {
        size_ai++;
    }

    return size_ai;
}

int initSB(unsigned int n_blocks, unsigned int n_inodes) {
    super_block_t sb;
    
    // Position of the first block in the bitmap
    sb.posFirstBlockMB = POS_SB + SIZE_SB;
    // Position of the last block in the bitmap
    sb.posLastBlockMB = sb.posFirstBlockMB + sizeMB(n_blocks) - 1;
    // Position of the first block in the inode array
    sb.posFirstBlockAI = sb.posLastBlockMB + 1;
    // Position of the last block in the inode array
    sb.posLastBlockAI = sb.posFirstBlockAI + sizeAI(n_blocks) - 1;
    // Position of the first data block
    sb.posFirstBlockData = sb.posLastBlockAI + 1;
    // Position of the last data block
    sb.posLastBlockData = n_blocks - 1;
    // Position of the root directory of the inode array
    sb.posInodeRoot = 0;
    // Position of the first free inode
    sb.posFirstInodeFree = 0;
    // Number of free blocks in SF
    sb.numBlocksFree = n_blocks;
    // Number of free inodes
    sb.numInodesFree = n_inodes;
    // Total number of blocks
    sb.allBlocks = n_blocks;
    // Total number of inodes
    sb.allInodes = n_inodes;

    printf("SUPERBLOCK DATA\n");
    printf("posFirstBlockMB = %u\n", sb.posFirstBlockMB);
    printf("posLastBlockMB = %u\n", sb.posLastBlockMB);
    printf("posFirstBlockAI = %u\n", sb.posFirstBlockAI);
    printf("posLastBlockAI = %u\n", sb.posLastBlockAI);
    printf("posFirstBlockData = %u\n", sb.posFirstBlockData);
    printf("posLastBlockData = %u\n", sb.posLastBlockData);
    printf("posInodeRoot = %u\n", sb.posInodeRoot);
    printf("posFirstInodeFree = %u\n", sb.posFirstInodeFree);
    printf("numBlocksFree = %u\n", sb.numBlocksFree);
    printf("numInodesFree = %u\n", sb.numInodesFree);
    printf("allBlocks = %u\n", sb.allBlocks);
    printf("allInodes = %u\n", sb.allInodes);

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

    unsigned int metadata_blocks = sb.posLastBlockMB + 1;       // Number of blocks occupied by metadata
    unsigned int all_ones_bytes = metadata_blocks / 8;          // Number of bytes set to all 1s because their 8 blocks are used for metadata
    unsigned int full_ones_blocks = all_ones_bytes / BLOCKSIZE; // Number of blocks set to all 1s
    unsigned int partial_ones = metadata_blocks % 8;            // Number of bits corresponding to a metadata block that can't be grouped in a byte

    unsigned char buffer[BLOCKSIZE];

    if (!memset(buffer, UCHAR_MAX, sizeof(buffer))) {
        perror("Bitmap initialization has not been done correctly");
        return FAILURE;
    }

    for (unsigned int i = 0; i < full_ones_blocks; i++) {
        if (bwrite(sb.posFirstBlockMB + i * BLOCKSIZE, buffer) == FAILURE) {
            fprintf(stderr, "Could not initialize bitmap");
            return FAILURE;
        }
    }

    // Reuse 1s set in the previous memset, and set the trailing 0s that are needed
    if (!memset(&buffer[all_ones_bytes], 0, sizeof(buffer) - all_ones_bytes)) {
        perror("Bitmap initialization has not been done correctly");
        return FAILURE;
    }

    unsigned char bits_left = UCHAR_MAX << (8 - partial_ones);
    buffer[all_ones_bytes] = bits_left;

    if (bwrite(sb.posFirstBlockMB + full_ones_blocks, buffer) == FAILURE) {
        fprintf(stderr, "Could not initialize bitmap");
        return FAILURE;
    }

    return SUCCESS;
}

int initAI() {
    inode_t inodes [BLOCKSIZE / INODESIZE];

    super_block_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }
    
    unsigned int inode_counter = sb.posFirstInodeFree + 1; 

    for (unsigned int i = sb.posFirstBlockAI; i <= sb.posLastBlockAI; i++) {
        // Read the inode block from the filesystem
        for (int j = 0; j < BLOCKSIZE / INODESIZE; j++) {
            inodes[j].type = FREE;

            if (inode_counter < sb.allInodes) {
                inodes[j].direct_pointers[0] = inode_counter;
                inode_counter++;
            } else {
                inodes[j].direct_pointers[0] = UINT_MAX;
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
