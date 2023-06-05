#include "verificacion.h"

void guardarRegistro(struct INFORMACION info, char *str_informe, int *offset) { //metodo para printear los registros
    /* hay que formatear el guardado en el archivo para que al leerlo salga asi:
    [Placeholders]

    PID: 16776
    Numero de escrituras: 50
    Primera Escritura   1       327582  Wed May 23 13:38:28 2018
    Ultima escritura    50      nEscritura Fecha
    Menor Posicion      22
    Mayor Posicion      46

    PID: 16776
    Numero de escrituras: 50
    Primera Escritura   1       327582  Wed May 23 13:38:28 2018
    Ultima escritura    50      nEscritura Fecha
    Menor Posicion      22
    Mayor Posicion      46
    */
    char primerafecha[24], ultimafecha[24], maxfecha[24], minfecha[24];
    struct tm *tp;
    tp = localtime(&info.PrimeraEscritura.fecha);
    strftime(primerafecha, sizeof(primerafecha), "%a %Y-%m-%d %H:%M:%S", tp);
    tp = localtime(&info.UltimaEscritura.fecha);
    strftime(ultimafecha, sizeof(ultimafecha), "%a %Y-%m-%d %H:%M:%S", tp);
    tp = localtime(&info.MenorPosicion.fecha);
    strftime(minfecha, sizeof(minfecha), "%a %Y-%m-%d %H:%M:%S", tp);
    tp = localtime(&info.MayorPosicion.fecha);
    strftime(maxfecha, sizeof(maxfecha), "%a %Y-%m-%d %H:%M:%S", tp);

    char outputBuffer[BLOCKSIZE];
    memset(outputBuffer, 0, BLOCKSIZE);
    sprintf(outputBuffer, "PID: %d\nNumero de escrituras:\t%d\nPrimera escritura: \t%d\t%d\t%s\nUltima escritura:\t%d\t%d\t%s\nMenor posición:\t\t%d\t%d\t%s\nMayor posición:\t\t%d\t%d\t%s\n\n",
            info.pid, info.nEscrituras, info.PrimeraEscritura.nEscritura,
            info.PrimeraEscritura.nRegistro, primerafecha, info.UltimaEscritura.nEscritura,
            info.UltimaEscritura.nRegistro, ultimafecha, info.MenorPosicion.nEscritura,
            info.MenorPosicion.nRegistro, minfecha, info.MayorPosicion.nEscritura,
            info.MayorPosicion.nRegistro, maxfecha);
    //fprintf(stderr, "%s\n", outputBuffer);
    //fprintf(stderr, "offset: %d", *offset);
    *offset += mi_write(str_informe, outputBuffer, *offset, sizeof(outputBuffer)); //el offset deberia ir incrementando a medida que se hacen escrituras en el archivo
}

int main(int argc,char **argv){

    if(argv[1] == NULL || argv[2] == NULL || argv[2][strlen(argv[2])-1] != '/'){ // Checkear syntax y aseguramos que sea un directorio
        fprintf(stderr,"Command syntax should be: ./verificacion <disco> <directorio_simulacion>\n");
        return -1;
    }

    if(bmount(argv[1]) == -1){
        fprintf(stderr, "Error while mounting\n");
    }

    struct STAT stat;
    struct entrada buff_entrada [NUMPROCESOS];
    memset(buff_entrada, 0, sizeof(buff_entrada)); //NUMPROCESOS * sizeof(struct entrada)); // Init a 0's


    mi_stat(argv[2],&stat);
    //calculamos número de entradas del inodo
    int num_entradas = stat.tamEnBytesLog / sizeof(struct entrada);

    if(num_entradas != NUMPROCESOS){
        fprintf(stderr, "Error: num_entradas != NUMPROCESOS\n");
        return -1;
    }
    //reservamos memoria para copiar la dirección de simulación y un nombre.txt
    char str_informe[128];
    memset(str_informe, 0, sizeof(str_informe));

    //char *str_informe = malloc(strlen(argv[2]) + 20); // El char ocupa 1 byte, da igual multiplicar

    strcpy(str_informe, argv[2]);
    strcat(str_informe, "informe.txt");

    //creamos archivo
    if(mi_creat(str_informe, 6) < 0){
        fprintf(stderr, "Error while creating");
        return -1;
    }

    //mejora en las entradas usando mi_read para reducir la cantidad de lecturas en el disco
    //leemos los directorios correspondientes a los procesos
    if(mi_read(argv[2], &buff_entrada, 0, sizeof(buff_entrada)) < 0) { //argv[2] may be &argv[2] ; NUMPROCESOS * sizeof(struct entrada) -> sizeof(buff_entrada)
        return -1;
    }


    //int cant_registros_buffer_escrituras = 256;
    //struct REGISTRO buffer_escrituras[cant_registros_buffer_escrituras];

    struct entrada entrada_aux;

    int offset = 0;

    for(int i = 0 ; i < NUMPROCESOS ; i++){

        struct INFORMACION info;

        memset(&entrada_aux, 0, sizeof(struct entrada));
        // Creemos que son equivalentes | "leemos" la i-esima entrada del directorio
        // memcpy(&entrada_aux, i * sizeof(struct entrada) + buff_entrada, sizeof(struct entrada));
        memcpy(&entrada_aux, &buff_entrada[i], sizeof(struct entrada));

        info.pid = atoi(strchr(entrada_aux.nombre, '_') + 1);
        char dir_prueba[64];

        // Crear la direccion de prueba.dat
        sprintf(dir_prueba, "%s%s", argv[2], entrada_aux.nombre);
        strcat(dir_prueba, "/prueba.dat");


        //Se podria hacer con buscar entrada, a partir del inodo que se encuentra en entrada_aux.ninodo
        //de esa manera, funcionará aunque nos cambien el nombre "prueba.dat". Si lo de arriba funciona, es mas eficiente.
        //buscar_entrada(entrada_aux.ninodo) le hace falta mas cosicas pero ahora no las tenemos

        int cant_registros_buffer_escrituras = 256;
        struct REGISTRO buffer_escrituras [cant_registros_buffer_escrituras];
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));

        int nReg = 0; // contador de registros
        int contadorEscriturasValidadas = 0;

        while(mi_read(dir_prueba, buffer_escrituras, nReg * sizeof(struct REGISTRO), sizeof(buffer_escrituras)) > 0 ){
            //fprintf(stderr, "iteracion nueva del while\n");
            for(int j = 0; j < cant_registros_buffer_escrituras; j++){

                if(info.pid == buffer_escrituras[j].pid){
                    if(!contadorEscriturasValidadas){
                        info.PrimeraEscritura = buffer_escrituras[j];
                        info.UltimaEscritura = buffer_escrituras[j];
                        info.MayorPosicion = buffer_escrituras[j];
                        info.MenorPosicion = buffer_escrituras[j];
                    }else{
                        if(difftime(info.PrimeraEscritura.fecha, buffer_escrituras[j].fecha) > 0){
                            info.PrimeraEscritura = buffer_escrituras[j];
                        }else if((difftime(info.PrimeraEscritura.fecha, buffer_escrituras[j].fecha)) == 0){
                            if(info.PrimeraEscritura.nEscritura >= buffer_escrituras[j].nEscritura){ // La primera escritura sera aquella que tenga menos nescrituras
                                info.PrimeraEscritura = buffer_escrituras[j];
                            }
                        }

                        if(difftime(info.UltimaEscritura.fecha, buffer_escrituras[j].fecha) < 0){
                            info.UltimaEscritura = buffer_escrituras[j];
                        }else if((difftime(info.UltimaEscritura.fecha, buffer_escrituras[j].fecha)) == 0){
                            if(info.UltimaEscritura.nEscritura <= buffer_escrituras[j].nEscritura){ // La primera escritura sera aquella que tenga menos nescrituras
                                info.UltimaEscritura = buffer_escrituras[j];
                            }
                        }

                        if(buffer_escrituras[j].nRegistro < info.MenorPosicion.nRegistro){
                            info.MenorPosicion = buffer_escrituras[j];
                        } else if(buffer_escrituras[j].nRegistro > info.MayorPosicion.nRegistro){
                            info.MayorPosicion = buffer_escrituras[j];
                        }

                    }

                    contadorEscriturasValidadas++;
                    info.nEscrituras = contadorEscriturasValidadas;


                }
            }

            nReg += cant_registros_buffer_escrituras; //Hemos leido cant_registros_buffer_escrituras
            memset(buffer_escrituras, 0, sizeof(buffer_escrituras));

        }


        //Añadir la información del struct info al fichero informe.txt por el final
        guardarRegistro(info, str_informe, &offset);
        //printRegistro(info);
        printf("%d) %d escrituras validadas en %s\n", i, contadorEscriturasValidadas, dir_prueba);
    }

    bumount();
}