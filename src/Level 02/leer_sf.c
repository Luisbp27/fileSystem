#include "ficheros_basico.h"

// Usage: ./my_mkfs <path to virtual device>
int main(int argc, char const *argv[]) {
    // Check the possible errors in params
    if (argc < 2) {
        fprintf(stderr,
                "Not enough arguments. Usage: %s <device name> <block size>\n",
                argv[0]);

        return FAILURE;
    }

    const char *path = argv[1];

    // Mount the virtual device
    if (bmount(path) == FAILURE) {
        fprintf(stderr, "An error occurred while mounting the system.\n");

        return FAILURE;
    }

    // Reading the superblock from the virtual devide
    super_block_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        fprintf(stderr, "Error reading the superblock.\n");
        return FAILURE;
    }

    // Visualization of the superblock content
    printf("SUPERBLOCK DATA\n");
    printf("posPrimerBloqueMB = %u\n", sb.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %u\n", sb.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %u\n", sb.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %u\n", sb.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %u\n", sb.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %u\n", sb.posUltimoBloqueDatos);
    printf("posInodoRaiz = %u\n", sb.posInodoRaiz);
    printf("posPrimerInodoLibre = %u\n", sb.posPrimerInodoLibre);
    printf("cantBloquesLibres = %u\n", sb.cantBloquesLibres);
    printf("cantInodosLibres = %u\n", sb.cantInodosLibres);
    printf("totBloques = %u\n", sb.totBloques);
    printf("totInodos = %u\n", sb.totInodos);

    // Visualization of the inode size
    printf("Size of superblock type: %lu\n", sizeof(super_block_t));
    printf("Size of inode type: %lu\n", sizeof(inode_t));

    // Visualization of the linked list of free inodes
    printf("LINKED LIST OF FREE INODES\n");

    inode_t inodes[BLOCKSIZE / INODESIZE];
    for (unsigned int i = sb.posPrimerBloqueAI; i <= sb.posUltimoBloqueAI; i++) {
        // Reading the inode block
        if (bread(i, &inodes) == FAILURE) {
            return FAILURE;
        }

        for (int j = 0; j < BLOCKSIZE / INODESIZE; j++) {
            if (inodes[j].tipo != FREE) {
                continue;
            }

            if (inodes[j].punterosDirectos[0] != UINT_MAX) {
                printf("%u ", inodes[j].punterosDirectos[0]);
            } else {
                printf("END\n");
                break;
            }
        }
    }

    // Unmount the virtual device
    if (bumount() == FAILURE) {
        fprintf(stderr, "An error ocurred while unmounting the system.\n");

        return FAILURE;
    }
}
