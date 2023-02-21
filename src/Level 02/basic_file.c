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

}

int initMB() {

}

int initAI() {
    
}