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
    *posbit = nbloque % 8;  // posición del bit a modificar dentro del byte

    unsigned int nbloqueMB = *posbyte / BLOCKSIZE;  // posición del bloque relativa al comienzo del MB
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
    while (!found) {
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
    *inodo = inodos_buffer[ninodo % (BLOCKSIZE / INODESIZE)];

    return SUCCESS;
}

/**
 * This method finds the first free inode (data stored in the superblock),
 * reserves it (with the help of the function write_inodo())
 *
 * @param tipo
 * @param permisos
 *
 * @return Position of the reserved inode
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

/**
 * This method obtains the range of pointers in which the logical block
 * we are looking for is located. and we also obtain the address stored
 * in the corresponding pointer of the inode.
 *
 * @param inodo
 * @param nblogico
 * @param ptr
 *
 * @return -1 if there was an error, 0, 1, 2 or 3 otherwise
 */
int obtener_nRangoBL(inodo_t *inodo, unsigned int nblogico, unsigned int *ptr) {
    if (nblogico < DIRECTOS) {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }

    if (nblogico < INDIRECTOS0) {
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }

    if (nblogico < INDIRECTOS1) {
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }

    if (nblogico < INDIRECTOS2) {
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }

    *ptr = 0;

    return FAILURE;
}

/**
 * Function to generalise the fetching of block indices pointer blocks.
 *
 * @param nblogico
 * @param nivel_punteros
 *
 * @return -1 if there was an error or the index of the pointer block
 */
int obtener_indice(unsigned int nblogico, int nivel_punteros) {
    if (nblogico < DIRECTOS) {
        return nblogico;
    }

    if (nblogico < INDIRECTOS0) {
        return (nblogico - DIRECTOS);
    }

    if (nblogico < INDIRECTOS1) {
        if (nivel_punteros == 2) {
            return ((nblogico - INDIRECTOS0) / NPUNTEROS);
        }
        if (nivel_punteros == 1) {
            return ((nblogico - INDIRECTOS0) % NPUNTEROS);
        }
    }

    if (nblogico < INDIRECTOS2) {
        if (nivel_punteros == 3) {
            return ((nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS));
        }
        if (nivel_punteros == 2) {
            return (((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS);
        }
        if (nivel_punteros == 1) {
            return (((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS);
        }
    }

    // If nblogico > INDIRECTOS2, its an error and return FAILURE
    return FAILURE;
}

/**
 * This method is responsible for obtaining the physical block number corresponding to a
 * given logical block from the indicated inode.
 *
 * @param inodo
 * @param nblogico
 * @param reservar
 *
 * @return -1 if there was an error or the physical block number
 */
int traducir_bloque_inodo(inodo_t *inodo, unsigned int nblogico, unsigned char reservar) {
    unsigned int ptr, ptr_ant, buffer[NPUNTEROS];
    int nRangoBL, nivel_punteros, indice;

    ptr = 0;
    ptr_ant = 0;
    nRangoBL = obtener_nRangoBL(inodo, nblogico, &ptr);
    nivel_punteros = nRangoBL;

    // Iterate for every level of direct pointers
    while (nivel_punteros > 0) {
        // Check if ptr is not set to anything
        if (ptr == 0) {
            if (reservar == 0) {
                return FAILURE;
            } else {
                // Reserve pointer blocks and create links from inode to data block
                ptr = reservar_bloque();
                inodo->numBloquesOcupados++;
                inodo->ctime = time(NULL);

                // Check if the block hangs directly from the inode
                if (nivel_punteros == nRangoBL) {
                    inodo->punterosIndirectos[nRangoBL - 1] = ptr;

                    #if DEBUG4
                        fprintf(stderr,
                            "[traducir_bloque_inodo()→ inodo.punterosIndirectos[%i] = %i (reservado BF %i para punteros_nivel%i)]\n",
                            nRangoBL - 1, ptr, ptr, nivel_punteros + 1);
                    #endif
                } else {
                    buffer[indice] = ptr;

                    #if DEBUG4
                        fprintf(stderr,
                            "[traducir_bloque_inodo()→ inodo.punteros_nivel%i[%i] = %i (reservado BF %i para punteros_nivel%i)]\n",
                            nivel_punteros + 1, indice, ptr, ptr, nivel_punteros + 1);
                    #endif

                    if (bwrite(ptr_ant, buffer) == FAILURE) {
                        return FAILURE;
                    }
                }
                // Set to 0s all the buffer pointers
                memset(buffer, 0, BLOCKSIZE);
            }
        } else {
            // Read from the virtual device the existing block of pointers
            if (bread(ptr, buffer) == FAILURE) {
                return FAILURE;
            }
        }

        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;
        ptr = buffer[indice];
        nivel_punteros--;
    }

    // If does not exist any data block
    if (ptr == 0) {
        if (reservar == 0) {
            return FAILURE;
        } else {
            ptr = reservar_bloque();
            inodo->numBloquesOcupados++;
            inodo->ctime = time(NULL);

            // Check if is a direct pointer
            if (nRangoBL == 0) {
                // Assign the address of the data block at the inode
                inodo->punterosDirectos[nblogico] = ptr;

                #if DEBUG4
                    fprintf(stderr,
                        "[traducir_bloque_inodo()→ inodo.punterosDirectos[%i] = %i (reservado BF %i para BL %i)]\n\n",
                        nblogico, ptr, ptr, nblogico);
                #endif
            } else {
                // Allocate the address of the data block in the buffer
                buffer[indice] = ptr;

                #if DEBUG4
                    fprintf(stderr,
                        "[traducir_bloque_inodo()→ inodo.punteros_nivel%i[%i] = %i (reservado BF %i para BL %i)]\n\n",
                        nivel_punteros + 1, indice, ptr, ptr, nblogico);
                #endif

                if (bwrite(ptr_ant, buffer) == FAILURE) {
                    return FAILURE;
                }
            }
        }
    }

    return ptr;
}

/**
 * This method is responsible for freeing all the blocks of an inode.
 * 
 * @param ninodo 
 * 
 * @return Number of freed blocks
 */
int libearar_inodo(unsigned int ninodo) {
    inodo_t inodo;
    if (leer_inodo(ninodo, &inodo) == FAILURE) {
        return FAILURE;
    }

    // Releasing all the logical blocks
    inodo.numBloquesOcupados -= liberar_bloques_inodo(0, &inodo);
    if (inodo.numBloquesOcupados != 0) {
        return FAILURE;
    }

    // Updating the inode values
    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;

    // Reading the superblock
    super_bloque_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    // We include the freed inode in the linked list as the first free inode, and we link
    // this one with the previous one so as not to lose order
    inodo.punterosDirectos[0] = sb.posPrimerInodoLibre;
    sb.posPrimerInodoLibre = ninodo;
    sb.cantInodosLibres++;

    // Writing the updates on the superblock
    if (bwrite(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }

    inodo.ctime = time(NULL);
    // Writing the updates on the inode
    if (escribir_inodo(ninodo, &inodo) == FAILURE) {
        return FAILURE;
    }

    return ninodo;
}

/**
 * This method is responsible for freeing all the blocks of an inode.
 * 
 * @param primerBL 
 * @param inodo
 * 
 * @return Number of freed blocks
*/
int liberar_bloques_inodo(unsigned int primerBL, inodo_t *inodo) {

    unsigned int nivel_punteros, indice, nBL, ultimoBL;
    unsigned int ptr = 0;
    unsigned int bloque_punteros[3][NPUNTEROS];
    unsigned char bufAux_punteros[BLOCKSIZE]; 
    int ptr_nivel[3], nRangoBL, indices[3];
    int liberados = 0;

    // Check if the file is empty
    if (inodo->tamEnBytesLog == 0) {
        return 0;
    }

    // Obtain the number of the last block
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0) {
        ultimoBL = (inodo->tamEnBytesLog / BLOCKSIZE) - 1;
    } else {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }

    memset (bufAux_punteros, 0, BLOCKSIZE);

    for (nBL = primerBL; nBL <= ultimoBL; nBL++) {
        // Check if the block is a direct pointer
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);
        if (nRangoBL < 0) {
            return FAILURE;
        }
        nivel_punteros = nRangoBL;

        // While are hanging pointer blocks
        while (ptr > 0 && nivel_punteros > 0) {
            indice = obtener_indice(nBL, nivel_punteros);
            if (indice == 0 || nBL == primerBL) {
                // Read from the device if it is not already previously loaded into a buffer
                if (bread(ptr, bloque_punteros[nivel_punteros - 1]) == FAILURE) {
                    return FAILURE;
                }
            }

            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloque_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }

        // If the data block exists
        if (ptr > 0) {
            liberar_bloque(ptr);
            liberados++;

            // Check if is a direct pointer and set it to 0
            if (nRangoBL == 0) {
                inodo->punterosDirectos[nBL] = 0;
            
            } else {
                nivel_punteros = 1;

                // While are hanging pointer blocks
                while (nivel_punteros <= nRangoBL) {
                    indice = indices[nivel_punteros - 1];
                    bloque_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros - 1];
                    
                    if (memcmp(bloque_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0) {
                        // No more occupied blocks will hang, the pointer block must be freed.
                        liberar_bloque(ptr);
                        liberados++;
                        
                        if (nivel_punteros == nRangoBL) {
                            inodo->punterosIndirectos[nRangoBL - 1] = 0;
                        } 
                            
                        nivel_punteros++;
                        
                    } else {
                        // There are still occupied blocks hanging, the pointer block must be updated.
                        if (bwrite(ptr, bloque_punteros[nivel_punteros - 1]) == FAILURE) {
                            return FAILURE;
                        }

                        nivel_punteros = nRangoBL + 1;
                    }

                }
            }
        }
    }

    return liberados;
}
