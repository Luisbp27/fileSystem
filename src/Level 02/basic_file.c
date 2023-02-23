#include "basic_file.h"

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
        return size_ai + 1;
    }

    return size_ai;
}

int initSB(unsigned int n_blocks, unsigned int n_inodes) {
    struct super_block SB;

    // Position of the first block in the bitmap
    SB.posFirstBlockMB = posSB + sizeSB;
    // Position of the last block in the bitmap
    SB.posLastBlockMB = SB.posFirstBlockMB + sizeMB(n_blocks) + 1;
    // Position of the first block in the inode array
    SB.posFirstBlockAI = SB.posLastBlockMB + 1;
    // Position of the last block in the inode array
    SB.posLastBlockAI = SB.posFirstBlockAI + sizeAI(n_blocks) - 1;
    // Position of the first data block
    SB.posFirstBlockData = SB.posLastBlockAI + 1;
    // Position of the last data block
    SB.posFirstBlockData = n_blocks - 1;
    // Position of the root directory of the inode array
    SB.posInodeRoot = 0;
    // Position of the first free inode
    SB.posFirstInodeFree = 0;
    // Number of free blocks in SF
    SB.numBlocksFree = n_blocks;
    // Number of free inodes
    SB.numInodesFree = n_inodes;
    // Total number of blocks
    SB.allBlocks = n_blocks;
    // Total number of inodes
    SB.allInodes = n_inodes;

    // Write de SB on the filesystem
    if (bwrite(posSB, &SB) == FAILURE) {
        return FAILURE;
    }

    return SUCCESS;
}

int initMB() {}

int initAI() {
    struct inode i_nodes [BLOCKSIZE / INODESIZE];
}
