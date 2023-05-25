#include "simulacion.h"

struct INFORMACION {
  int pid;
  unsigned int nEscrituras; // validated 
  struct REGISTRO PrimeraEscritura;
  struct REGISTRO UltimaEscritura;
  struct REGISTRO MenorPosicion;
  struct REGISTRO MayorPosicion;
};
