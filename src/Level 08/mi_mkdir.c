#include "directorios.h"

/**
 * Program (command) that creates a file or directory, calling the function my_creat(). 
 * Depending on whether the path ends in / or not will indicate whether to create a directory or a file.
 * 
 * @param argc
 * @param argv
 * 
*/
int main(int argc, char **argv) {

    // Checking syntax
    if (argc < 4) {
        fprintf(stderr, "Command syntax should be: mi_mkdir <disco> <permisos> </ruta> \n");
        return FAILURE;
    }

    int path_len = strlen(argv[3]);

    // Checking if the path ends in /
    if (argv[3][path_len - 1] != '/') {
        fprintf(stderr, "The path must end in / to create a directory \n");
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

    // Creating the directory
    if (mi_creat(argv[3], atoi(argv[2])) == FAILURE) {
        fprintf(stderr, "Error creating the directory \n");
        bumount();
        
        return FAILURE;
    }

    bumount();

    return SUCCESS;
}