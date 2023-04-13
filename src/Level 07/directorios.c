#include "directorios.h"

/**
 * This method extracts the path of a file or directory from a given path.
 * 
 * @param camino 
 * @param inicial 
 * @param final 
 * 
 * @return 0 if the path is correct, -1 otherwise
*/
int extraer_camino(const char *camino, char *inicial, char *final) {

}

/**
 * This method searches for an entry in a given path.
 * 
 * @param camino_parcial
 * @param p_inodo_dir
 * @param p_inodo
 * @param p_entrada
 * @param reservar
 * @param permisos
 * 
 * @return 0 if the entry is found, -1 otherwise
*/
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {

}

/**
 * This method shows an error message.
 * 
 * @param error
*/
void mostrar_error_entrada(int error) {
    switch (error) {
        case -2: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
        case -3: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
        case -4: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
        case -5: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
        case -6: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
        case -7: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
        case -8: fprintf(stderr, "Error: No es un directorio.\n"); break;
    }
}