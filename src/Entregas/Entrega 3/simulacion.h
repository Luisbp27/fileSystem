#include "directorios.h"
#include <sys/wait.h>
#include <signal.h>

#define NUMPROCESOS 100
#define NUMESCRITURAS 50
#define REGMAX 500000

struct REGISTRO { // sizeof(struct REGISTRO): 24 bytes
   time_t fecha; // Seconds
   pid_t pid;
   int nEscritura; // From 1 to 50 (ordered by time)
   int nRegistro;
};