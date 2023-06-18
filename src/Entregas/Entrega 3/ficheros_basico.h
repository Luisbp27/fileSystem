#include "bloques.h"
#include <limits.h>
#include <time.h>
#include <math.h>

#define DEBUG3 0
#define DEBUG4 0
#define DEBUG5 0
#define DEBUG6 0
#define DEBUG7 0
#define DEBUG8 0
#define DEBUG9 0
#define DEBUG12 0
#define DEBUGENTREGA1 1
#define DEBUGIMPORTANT 0

#define POS_SB 0      // The SB is written in the first block of our file system
#define TAM_SB 1      // Unit: Blocks
#define INODESIZE 128 // Size in bytes
#define FREE 'l'

#define NPUNTEROS (BLOCKSIZE / sizeof(unsigned int)) // 256
#define DIRECTOS 12
#define INDIRECTOS0 (NPUNTEROS + DIRECTOS)                            // 1 nivel (268)
#define INDIRECTOS1 (NPUNTEROS * NPUNTEROS + INDIRECTOS0)             // 2 niveles (65804)
#define INDIRECTOS2 (NPUNTEROS * NPUNTEROS * NPUNTEROS + INDIRECTOS1) // 3 niveles (16843020)
#define LIBERAR_BLOQUES_INODO_FREE_NONE 0 // No blocks were modified, no need to rewrite
#define LIBERAR_BLOQUES_INODO_FREE_SOME 1 // Some blocks were freed, but not all, so the pointer block has to be rewritten
#define LIBERAR_BLOQUES_INODO_FREE_ALL 2 // All blocks were freed, no need to rewrite, but the pointer block has to be freed


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
} super_bloque_t;

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
} inodo_t;

int tamMB(unsigned int nbloques);
int tamAI(unsigned int ninodos);
int initSB(unsigned int nbloques, unsigned int ninodos);
int initMB();
int initAI();

int escribir_bit(unsigned int nbloque, unsigned int bit);
char leer_bit(unsigned int nbloque);
int reservar_bloque();
int liberar_bloque(unsigned int nbloque);
int escribir_inodo(unsigned int ninodo, inodo_t *inodo);
int leer_inodo(unsigned int ninodo, inodo_t *inodo);
int reservar_inodo(unsigned char tipo, unsigned char permisos);
int traducir_bloque_inodo(inodo_t *inodo, unsigned int nblogico, unsigned char reservar);
int liberar_inodo(unsigned int ninodo);
int liberar_bloques_inodo(unsigned int primerBL, inodo_t *inodo);
