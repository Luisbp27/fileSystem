#include "blocks.h"

#define posSB 0   // The SB is written in the first block of our file system
#define sizeSB 1
#define INODESIZE 128 // Size in bytes

struct super_block {
   unsigned int posPrimerBloqueMB;     // Absolute position of the first block of the bitmap
   unsigned int posUltimoBloqueMB;     // Absolute position of the last block of the bitmap
   unsigned int posPrimerBloqueAI;     // Absolute position of the first block of the inode array
   unsigned int posUltimoBloqueAI;     // Absolute position of the last block of the inode array
   unsigned int posPrimerBloqueDatos;  // Absolute position of the first data block
   unsigned int posUltimoBloqueDatos;  // Absolute position of the last data block
   unsigned int posInodoRaiz;          // Root directory inode position (relative to the IM)
   unsigned int posPrimerInodoLibre;   // Posición del primer inodo libre (relativa al AI)
   unsigned int cantBloquesLibres;     // Number of free blocks (on the whole disc)
   unsigned int cantInodosLibres;      // Number of free inodes (in the IM)
   unsigned int totBloques;            // Total number of disc blocks
   unsigned int totInodos;             // Total number of inodes (heuristic)
   char padding[BLOCKSIZE - 12 * sizeof(unsigned int)];  // Filling to fill the entire block
};

struct inode {    
   unsigned char type;  // ('l':free, 'd':directory o 'f':file)
   unsigned char perms; // Permisos (read, write or execution)

   /* For internal structure alignment reasons, if a 4-byte word size 
   (32-bit microprocessors) is being used a 4-byte word size 
   (32-bit microprocessors): unsigned char reserved_lineup1 [2]; in case the word 
   used is 8-byte word size (64-bit microprocessors): unsigned char reserved_lineup1 [6]; */
   unsigned char reserved_lineup1[6];

   time_t a_time; // Date and time of last data access
   time_t m_time; // Date and time of last data modification
   time_t c_time; // Date and time of last modification of the inode

   unsigned int n_links;          // Number of directory entry links
   unsigned int log_size_bytes;    // Size in logical bytes (EOF)
   unsigned int num_blocks_busy;   // Number of occupied blocks data area

   unsigned int direct_pointers[12];    // 12 direct block pointers
   unsigned int undirect_pointers[3];   // 1 single indirect, 1 double indirect, 1 triple indirect

   char padding[INODESIZE - 2 * sizeof(unsigned char) - 3 * sizeof(time_t) - 18 * sizeof(unsigned int) - 6 * sizeof(unsigned char)];
};

