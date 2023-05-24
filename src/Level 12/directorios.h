#include "ficheros.h"

// Directory/file name size, ext2 = 255
#define TAMNOMBRE 60

// Error codes
#define ERROR_CAMINO_INCORRECTO -2
#define ERROR_PERMISO_LECTURA -3
#define ERROR_NO_EXISTE_ENTRADA_CONSULTA -4
#define ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO -5
#define ERROR_PERMISO_ESCRITURA -6
#define ERROR_ENTRADA_YA_EXISTENTE -7
#define ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO -8

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define RESET "\x1b[0m"

#define TAMFILA 100
#define TAMBUFFER (TAMFILA * 1000) // suponemos un máx de 1000 entradas, aunque debería ser SB.totInodos

#define LBLUE "\x1b[94m"
#define LRED "\x1b[91m"

#define TAMNOMBRE 60
#define PROFUNDIDAD 32

#define MAX_CACHE 10

struct entrada {
    char nombre[TAMNOMBRE];
    unsigned int ninodo;
};

struct UltimaEntrada {
    char camino[TAMNOMBRE * PROFUNDIDAD];
    int p_inodo;
};

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo);
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos);
void mostrar_error_buscar_entrada(int error);

int mi_creat(const char *camino, unsigned char permisos);
int mi_chmod(const char *camino, unsigned char permisos);
int mi_stat(const char *camino, struct STAT *p_stat);
int mi_dir(const char *camino, char *buffer, char *tipo, int extended);

int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes);
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes);

int mi_link(const char *camino1, const char *camino2);
int mi_unlink(const char *camino);