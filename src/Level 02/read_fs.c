#include "basic_files.h"

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

    // Visualization of the inode size
    printf ("Size of superblock type: %lu\n", sizeof(super_block_t));
    printf ("Size of inode type: %lu\n", sizeof(inode_t));
    
    // Visualization of the linked list of free inodes
    printf("LINKED LIST OF FREE INODES\n");
    inode_t inodes[BLOCKSIZE / INODESIZE];
    
    for (unsigned int i = sb.posFirstBlockAI; i <= sb.posLastBlockAI; i++) {
        // Reading the inode block
        if (bwrite(i, &inodes)== FAILURE) {
            return FAILURE;
        }

        for (int j = 0; j < BLOCKSIZE / INODESIZE; j++) {
            if (inodes[j].type != FREE) {
                continue;
            }

            if (inodes[j].direct_pointers[0] != UINT_MAX) {
                printf("%u ", inodes[j].direct_pointers[0]);
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