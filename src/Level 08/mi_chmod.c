#include "directorios.h"

/**
 * Change the permissions of a file or directory by calling 
 * the my_chmod() function of the directory layer
 * 
 * @param argc
 * @param argv
 * 
*/
int main (int argc, char **argv) {

    // Checking syntax
    if (argc < 3) {
        fprintf(stderr, "Command syntax should be: mi_chmod <disco> <permisos> </ruta> \n");
        return FAILURE;
    }

    // Check perms
    if ((atoi(argv[2]) < 0) || (atoi(argv[2]) > 7)) { 
        fprintf(stderr, "Invalid perms \n");
        return FAILURE;
    }

    if (bmount(argv[1]) == FAILURE) {
        fprintf(stderr, "Error mounting the disk \n");
        return FAILURE;
    }

    // Changing the permissions
    if (mi_chmod(argv[3], atoi(argv[2])) == FAILURE) {
        fprintf(stderr, "Error changing the permissions \n");
        bumount();
        
        return FAILURE;
    }
}