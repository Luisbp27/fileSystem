#include "simulacion.h"

int acabados = 0;

void reaper() {
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        acabados++;
    #if DEBUG12
        fprintf(stderr, "[simulación.c → Acabado proceso con PID %d, total acabados: %d]\n", ended, acabados);
    #endif
    }
}

int main(int argc, char **argv) {
    signal(SIGCHLD, reaper);

    // Check syntax
    if (argc < 2) {
        fprintf(stderr, "Command syntax should be: ./simulacion <disco>");
        return FAILURE;
    }

    // Mount virtual device
    if (bmount(argv[1]) == FAILURE) {
        return FAILURE;
    }

    // Simulation directory
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    char path[80];
    char tmp[100];
    strcpy(path, "/simul_");
    sprintf(tmp, "%d%02d%02d%02d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
    strcat(path, tmp);
    strcat(path, "/");

    if (mi_creat(path, 6) == FAILURE) {
        return FAILURE;
    }

    pid_t pid;
    printf("*** SIMULACIÓN DE %d PROCESOS REALIZANDO CADA UNO %d ESCRITURAS ***\n", NUMPROCESOS, NUMESCRITURAS);
    for (int proceso = 1; proceso <= NUMPROCESOS; proceso++) {
        pid = fork();
        // If it's the child
        if (pid == 0) {
            if (bmount(argv[1]) == FAILURE) {
                return FAILURE;
            }

            // Creating the directory
            char name_dir[200];
            sprintf(name_dir, "%sproceso_%d/", path, getpid());
            if (mi_creat(name_dir, 6) == FAILURE) {
                bumount();
                exit(0);
            }

            // Creating the file
            char file[100];
            strcpy(file, name_dir);
            strcat(file, "prueba.dat");
            if (mi_creat(file, 6) == FAILURE) {
                bumount();
                exit(0);
            }
            // fprintf(stderr, "Fichero del proceso %i creado\n", proceso);

            // Initialize random seed
            srand(time(NULL) + getpid());

            for (int nescritura = 1; nescritura <= NUMESCRITURAS; nescritura++) {
                struct REGISTRO registro;

                gettimeofday(&registro.fecha, NULL);
                registro.pid = getpid();
                registro.nEscritura = nescritura;
                registro.nRegistro = rand() % REGMAX;

                // Writing the file
                //fprintf(stderr, "Proceso: %d | Reg: %d\n", registro.pid, registro.nRegistro);
                if (mi_write(file, &registro, registro.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO)) == FAILURE) {
                    bumount();
                    exit(0);
                }

                #if DEBUG12
                    fprintf(stderr, "[simulación.c → Escritura %i en %s]\n", nescritura, file);
                #endif

                // Sleeping in microseconds
                usleep(50000);

                // Check if the process has to end
                if (nescritura == NUMESCRITURAS) {
                    fprintf(stderr,"Proceso %d: Completadas %d escrituras en %s\n", proceso, NUMESCRITURAS, path);
                }
            }

            bumount();
            exit(0);
        }

        // Sleeping in microseconds
        usleep(150000);
    }

    // Waiting the childs
    while (acabados < NUMPROCESOS) {
        pause();
    }
    // fprintf(stderr, "Total de procesos terminados: %d\n", acabados);

    if (bumount() == FAILURE) {
        exit(0);
    }

    return SUCCESS;
}

