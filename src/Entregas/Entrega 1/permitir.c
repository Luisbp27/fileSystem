#include "ficheros.h"

int main(int argc, char *argv[]) {
    // argv[1] = dir, argv[2] = ninodo, argv[3] = permisos
    if (argc < 4) {
        fprintf(stderr, "Command syntax should be: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        return FAILURE;
    }

    char *disk = argv[1];
    int ninode = atoi(argv[2]);
    int perms = atoi(argv[3]);

    if (bmount(disk) == FAILURE) {
        fprintf(stderr, "Error while mounting the virtual device\n");
        return FAILURE;
    }

    if (mi_chmod_f(ninode, perms) == FAILURE) {
        fprintf(stderr, "Error while changing permissions\n");
        return FAILURE;
    }

    if (bumount() == FAILURE) {
        fprintf(stderr, "Error while unmounting the virtual device\n");
        return FAILURE;
    }

    return SUCCESS;
}
