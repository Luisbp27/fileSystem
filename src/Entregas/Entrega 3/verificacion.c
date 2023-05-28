#include "verificacion.h"

int main(int argc, char **argv) {

    // Check syntax
    if (argc < 2 ) {
        fprintf(stderr, "Command syntax should be: ./verificacion <disco> <directorio_simulacion>\n");
        return FAILURE;
    }

    // Mount virtual device
    if (bmount(argv[1]) == FAILURE) {
        return FAILURE;
    }

    struct STAT stat;
    struct entrada buff_entr [NUMPROCESOS];
    memset(buff_entr, 0, sizeof(buff_entr));

    // Check if the directory exists
    if (mi_stat(argv[2], &stat) == FAILURE) {
        return FAILURE;
    }

    // Calculate the number of entries of the inode
    int num_entr = stat.tamEnBytesLog / sizeof(struct entrada);
    if (num_entr != NUMPROCESOS) {
        fprintf(stderr, "The number of entries in the directory is not correct\n");
        bumount();

        return FAILURE;
    }

    // Create a report 
    char report[100];
    memset(report, 0, sizeof(report));
    sprintf(report, "%s%s", argv[2], "informe.txt");
    if (mi_creat(report, 7) == FAILURE) {
        bumount();
        exit(0);
    }

    // Upload the entries
    struct entrada entrada[num_entr];
    int error = mi_read(argv[2], entrada, 0, sizeof(entrada));
    if (error  == FAILURE) {
        mostrar_error_buscar_entrada(error);
        return FAILURE;
    }    

    for (int num_entrada = 0; num_entrada < num_entr; num_entrada++) {
        pid_t pid = atoi(strchr(entrada[num_entrada].nombre, '_') + 1);
        struct INFORMACION info;
        info.pid = pid;
        info.nEscrituras = 0;

        char prueba[128];
        sprintf(prueba, "%s%s/%s", argv[2], entrada[num_entrada].nombre, "prueba.dat");

        int w_buffer_registers = 256;
        struct REGISTRO w_buffer[w_buffer_registers];
        memset(w_buffer, 0, sizeof(w_buffer));

        fprintf(stderr, "hola0\n");

        int offset = 0;
        // While there are registers to read on prueba.dat
        while (mi_read(prueba, w_buffer, offset, sizeof(w_buffer)) > 0) {
            int num_reg = 0;
            // While there are registers in the buffer
            while (num_reg < w_buffer_registers) {
                // If it is a valid register
                if (w_buffer[num_reg].pid == info.pid) {
                    // If it is the first register
                    if (!info.nEscrituras) {
                        info.MenorPosicion = w_buffer[num_reg];
                        info.MayorPosicion = w_buffer[num_reg];
                        info.PrimeraEscritura = w_buffer[num_reg];
                        info.UltimaEscritura = w_buffer[num_reg];
                        info.nEscrituras++;
                    } else {
                        // Update the information of the first and last write
                        if ((difftime(w_buffer[num_reg].fecha, info.PrimeraEscritura.fecha)) <= 0 && w_buffer[num_reg].nEscritura < info.PrimeraEscritura.nEscritura) {
                            info.PrimeraEscritura = w_buffer[num_reg];
                        } else if ((difftime(w_buffer[num_reg].fecha, info.UltimaEscritura.fecha)) >= 0 && w_buffer[num_reg].nEscritura > info.UltimaEscritura.nEscritura) {
                            info.UltimaEscritura = w_buffer[num_reg];
                        } else if (w_buffer[num_reg].nRegistro < info.MenorPosicion.nRegistro) {
                            info.MenorPosicion = w_buffer[num_reg];
                        } else if (w_buffer[num_reg].nRegistro > info.MayorPosicion.nRegistro) {
                            info.MayorPosicion = w_buffer[num_reg];
                        }
                        info.nEscrituras++;
                    }
                }
                num_reg++;
            }
            memset(&w_buffer, 0, sizeof(w_buffer));
            offset += sizeof(w_buffer);
        }

        char first_time[128];
        char last_time[128];
        char minor_time[128];
        char major_time[128];
        struct tm *tm_info;

        tm_info = localtime(&info.PrimeraEscritura.fecha);
        strftime(first_time, sizeof(first_time), "%a %Y-%m-%d %H:%M:%S", tm_info);
        tm_info = localtime(&info.UltimaEscritura.fecha);
        strftime(last_time, sizeof(last_time), "%a %Y-%m-%d %H:%M:%S", tm_info);
        tm_info = localtime(&info.MenorPosicion.fecha);
        strftime(minor_time, sizeof(minor_time), "%a %Y-%m-%d %H:%M:%S", tm_info);
        tm_info = localtime(&info.MayorPosicion.fecha);
        strftime(major_time, sizeof(major_time), "%a %Y-%m-%d %H:%M:%S", tm_info);

        char buffer[BLOCKSIZE];
        memset(buffer, 0, sizeof(buffer));

        sprintf(buffer, "PID: %i\nNumero de escrituras: %i\n", pid, info.nEscrituras);
        sprintf(buffer + strlen(buffer), "%s %i %i %s",
                "Primera escritura",
                info.PrimeraEscritura.nEscritura,
                info.PrimeraEscritura.nRegistro,
                asctime(localtime(&info.PrimeraEscritura.fecha)));

        sprintf(buffer + strlen(buffer), "%s %i %i %s",
                "Ultima escritura",
                info.UltimaEscritura.nEscritura,
                info.UltimaEscritura.nRegistro,
                asctime(localtime(&info.UltimaEscritura.fecha)));

        sprintf(buffer + strlen(buffer), "%s %i %i %s",
                "Menor posicion",
                info.MenorPosicion.nEscritura,
                info.MenorPosicion.nRegistro,
                asctime(localtime(&info.MenorPosicion.fecha)));

        sprintf(buffer + strlen(buffer), "%s %i %i %s",
                "Mayor posicion",
                info.MayorPosicion.nEscritura,
                info.MayorPosicion.nRegistro,
                asctime(localtime(&info.MayorPosicion.fecha)));

        sprintf(buffer,
                "PID: %d\nNumero de escrituras:\t%d\nPrimera escritura:"
                "\t%d\t%d\t%s\nUltima escritura:\t%d\t%d\t%s\nMayor po"
                "sición:\t\t%d\t%d\t%s\nMenor posición:\t\t%d\t%d\t%s\n\n",
                info.pid, info.nEscrituras,
                info.PrimeraEscritura.nEscritura,
                info.PrimeraEscritura.nRegistro,
                first_time,
                info.UltimaEscritura.nEscritura,
                info.UltimaEscritura.nRegistro,
                last_time,
                info.MenorPosicion.nEscritura,
                info.MenorPosicion.nRegistro,
                minor_time,
                info.MayorPosicion.nEscritura,
                info.MayorPosicion.nRegistro,
                major_time);
            

        if ((offset += mi_write(report, &buffer, offset, strlen(buffer))) < 0) {
            fprintf(stderr, "Error writing to %s\n", report);
            bumount();

            return FAILURE;
        }
    }

    if (bumount() == FAILURE) {
        return FAILURE;
    }
}