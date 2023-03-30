#include "ficheros.h"

/**
 * Usage: permitir <nombre_dispositivo> <ninodo> <permisos>
 * 
 * @param argc
 * @param argv
 * 
 * @return 0 if success, -1 if error
*/
int main(int argc, char *argv[]) {

    // argv[1] = dir, argv[2] = ninodo, argv[3] = permisos
    if (argc < 3) { 
        fprintf(stderr, "Command syntax should be: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        return FAILURE;

    } else {
        if (bmount(argv[1]) == FAILURE) {
            fprintf(stderr, "Error while mounting the virtual device");
            return FAILURE;
        }

        mi_chmod_f(atoi(argv[2]), atoi(argv[3]));

        if (bumount() == FAILURE) {
            fprintf(stderr, "Error while unmounting the virtual device");
            return FAILURE;
        }
    }

    return SUCCESS;
}