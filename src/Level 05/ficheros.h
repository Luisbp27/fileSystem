#include "ficheros_basico.h"

typedef union {
    struct
    {
        unsigned char tipo;                          
        unsigned char permisos;                      
        
        time_t atime;                                       
        time_t mtime;                       
        time_t ctime;                       
            
        unsigned int tamEnBytesLog;      
        unsigned int numBloquesOcupados;
        unsigned int nlinks; 
    };
} stat_t;

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_stat_f(unsigned int ninodo, stat_t *p_stat);
int mi_chmod_f(unsigned int ninodo, unsigned char permisos);