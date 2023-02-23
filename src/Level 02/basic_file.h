#include "blocks.h"

#define posSB 0       // The SB is written in the first block of our file system
#define sizeSB 1      // Unit: Blocks
#define INODESIZE 128 // Size in bytes

typedef union {
    struct super_block {
        unsigned int posFirstBlockMB;   // Absolute position of the first block of the bitmap
        unsigned int posLastBlockMB;    // Absolute position of the last block of the bitmap
        unsigned int posFirstBlockAI;   // Absolute position of the first block of the inode array
        unsigned int posLastBlockAI;    // Absolute position of the last block of the inode array
        unsigned int posFirstBlockData; // Absolute position of the first data block
        unsigned int posLastBlockData;  // Absolute position of the last data block
        unsigned int posInodeRoot;      // Root directory inode position (relative to the IM)
        unsigned int posFirstInodeFree; // Posición del primer inodo libre (relativa al AI)
        unsigned int numBlocksFree;     // Number of free blocks (on the whole disc)
        unsigned int numInodesFree;     // Number of free inodes (in the IM)
        unsigned int allBlocks;         // Total number of disc blocks
        unsigned int allInodes;         // Total number of inodes (heuristic)
    };
    char padding[BLOCKSIZE];
} super_block_t;

typedef union {
    struct inode {
        unsigned char type;  // ('l':free, 'd':directory o 'f':file)
        unsigned char perms; // Permisos (read, write or execution)

        /* For internal structure alignment reasons, if a 4-byte word size
        (32-bit microprocessors) is being used a 4-byte word size
        (32-bit microprocessors): unsigned char reserved_lineup1 [2]; in case
        the word used is 8-byte word size (64-bit microprocessors): unsigned
        char reserved_lineup1 [6]; */
        unsigned char reserved_lineup1[6];

        time_t a_time; // Date and time of last data access
        time_t m_time; // Date and time of last data modification
        time_t c_time; // Date and time of last modification of the inode

        unsigned int n_links;         // Number of directory entry links
        unsigned int log_size_bytes;  // Size in logical bytes (EOF)
        unsigned int num_blocks_busy; // Number of occupied blocks data area

        unsigned int direct_pointers[12];  // 12 direct block pointers
        unsigned int undirect_pointers[3]; // 1 single indirect, 1 double indirect, 1 triple indirect
    };
    char padding[INODESIZE];
} inode_t;
