#include "bloques.h"
#include <limits.h>

#define POS_SB 0      // The SB is written in the first block of our file system
#define TAM_SB 1      // Unit: Blocks
#define INODESIZE 128 // Size in bytes
#define FREE 'l'

typedef union {
    struct
    {
        unsigned int posPrimerBloqueMB;    // Absolute position of the first block of the bitmap
        unsigned int posUltimoBloqueMB;    // Absolute position of the last block of the bitmap
        unsigned int posPrimerBloqueAI;    // Absolute position of the first block of the inode array
        unsigned int posUltimoBloqueAI;    // Absolute position of the last block of the inode array
        unsigned int posPrimerBloqueDatos; // Absolute position of the first data block
        unsigned int posUltimoBloqueDatos; // Absolute position of the last data block
        unsigned int posInodoRaiz;         // Root directory inode position (relative to the IM)
        unsigned int posPrimerInodoLibre;  // Posición del primer inodo libre (relativa al AI)
        unsigned int cantBloquesLibres;    // Number of free blocks (on the whole disc)
        unsigned int cantInodosLibres;     // Number of free inodes (in the IM)
        unsigned int totBloques;           // Total number of disc blocks
        unsigned int totInodos;            // Total number of inodes (heuristic)
    };
    char padding[BLOCKSIZE];
} super_block_t;

typedef union {
    struct
    {
        unsigned char tipo;     // ('l':free, 'd':directory o 'f':file)
        unsigned char permisos; // Permisos (read, write or execution)

        /* For internal structure alignment reasons, if a 4-byte word size
        (32-bit microprocessors) is being used a 4-byte word size
        (32-bit microprocessors): unsigned char reserved_lineup1 [2]; in case
        the word used is 8-byte word size (64-bit microprocessors): unsigned
        char reserved_lineup1 [6]; */
        unsigned char reservado_alineacion1[6];

        time_t atime; // Date and time of last data access
        time_t mtime; // Date and time of last data modification
        time_t ctime; // Date and time of last modification of the inode

        unsigned int nlinks;             // Number of directory entry links
        unsigned int tamEnBytesLog;      // Size in logical bytes (EOF)
        unsigned int numBloquesOcupados; // Number of occupied blocks data area

        unsigned int punterosDirectos[12];  // 12 direct block pointers
        unsigned int punterosIndirectos[3]; // 1 single indirect, 1 double indirect, 1 triple indirect
    };
    char padding[INODESIZE];
} inode_t;

int tamMB(unsigned int nbloques);
int tamAI(unsigned int ninodos);
int initSB(unsigned int nbloques, unsigned int ninodos);
int initMB();
int initAI();
