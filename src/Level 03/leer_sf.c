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
    super_bloque_t sb;
    if (bread(POS_SB, &sb) == FAILURE) {
        fprintf(stderr, "Error reading the superblock.\n");
        return FAILURE;
    }

    // Visualization of the superblock content
    printf("SUPERBLOQUE\n");
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
    printf("Size of superblock type: %lu\n", sizeof(super_bloque_t));
    printf("Size of inode type: %lu\n", sizeof(inodo_t));

    // Visualization of the linked list of free inodes
    printf("\nINODOS LIBRES\n");

    inodo_t inodes[BLOCKSIZE / INODESIZE];
    for (unsigned int i = sb.posPrimerBloqueAI; i <= sb.posPrimerBloqueAI + 10; i++) {
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

    printf("\n\nRESERVA Y LIBERACIÓN DE BLOQUES\n");

    int pos = reservar_bloque();
    printf("Se ha reservado el bloque físico nº %d que era el 1º libre indicado por el MB\n", pos);
    
    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }
    printf("Cantidad de bloques libres: %u\n", sb.cantBloquesLibres);
    printf("Liberamos ese bloque y después sb.cantidadBloquesLibres: %u\n", sb.cantBloquesLibres);

    printf("\nMAPA DE BITS\n");

    if (bread(POS_SB, &sb) == FAILURE) {
        return FAILURE;
    }
    printf("| %u | %u ... %u | %u ... %u | %u ... %u |\n",
        leer_bit(POS_SB),
        leer_bit(sb.posPrimerBloqueMB),
        leer_bit(sb.posUltimoBloqueMB),
        leer_bit(sb.posPrimerBloqueAI),
        leer_bit(sb.posUltimoBloqueAI),
        leer_bit(sb.posPrimerBloqueDatos),
        leer_bit(sb.posUltimoBloqueDatos)
    );
    printf("| S |    MB   |    AI   |    D     |\n");

    printf("\nDATOS DEL DIRECTORIO RAIZ\n");

    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];

    inodo_t inodo;
    if (leer_inodo(0, &inodo) == FAILURE) {
        return FAILURE;
    }

    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);

    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %u\n", inodo.permisos);
    printf("atime: %s\n", atime);
    printf("ctime: %s\n", ctime);
    printf("mtime: %s\n", mtime);
    printf("nlinks: %u\n", inodo.nlinks);
    printf("Tamaño en bytes lógicos: %u\n", inodo.tamEnBytesLog);
    printf("Bloques ocupados: %u\n", inodo.numBloquesOcupados);

    // Unmount the virtual device
    if (bumount() == FAILURE) {
        fprintf(stderr, "An error ocurred while unmounting the system.\n");
        return FAILURE;
    }

    return SUCCESS;
}
