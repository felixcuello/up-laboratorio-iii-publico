/*
 * Monitor IoT simulado. Recibe mediciones de temperatura desde una
 * cola de mensajes POSIX y las muestra en pantalla.
 *
 * Uso: ./consumidor [cantidad]
 *   cantidad: numero de mediciones a recibir (por defecto 20)
 *
 * Compilacion:
 *   gcc -std=c89 -pedantic -Wall -Werror consumidor.c -o consumidor -lrt
 *
 * Comparacion con el modulo 06 (memoria compartida + semaforos):
 * - NO hay semaforos manuales (sem_wait, sem_post)
 * - NO hay buffer circular ni aritmetica modular
 * - NO hay calculo de indices
 * - El bloqueo cuando la cola esta vacia es AUTOMATICO en mq_receive
 *
 * IMPORTANTE: el consumidor es responsable de hacer mq_unlink al terminar,
 * eliminando la cola del sistema.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <mqueue.h>

#include "mensajes.h"

/* Cantidad por defecto de mediciones a recibir */
#define CANTIDAD_DEFAULT 20

/* Intentos maximos para conectarse a la cola */
#define MAX_INTENTOS 10

/* Variable global para el descriptor de la cola */
static mqd_t cola = (mqd_t)-1;

/*
 * Inicializa la cola de mensajes.
 * El consumidor NO crea la cola (sin O_CREAT), solo la abre.
 * Si la cola no existe todavia, espera y reintenta.
 *
 * Comparacion con modulo 06:
 * - Alla habia que hacer shm_open + mmap + 3x sem_open
 * - Aca es solo mq_open
 */
int inicializar_cola(void)
{
    int intentos;

    for (intentos = 0; intentos < MAX_INTENTOS; intentos++) {
        /* Abrir la cola existente para lectura (O_RDONLY) */
        cola = mq_open(COLA_NOMBRE, O_RDONLY);
        if (cola != (mqd_t)-1) {
            return 0;  /* Exito */
        }

        /* Si el error es que no existe, esperar y reintentar */
        if (errno == ENOENT) {
            printf("[Monitor] Cola no existe, esperando al sensor... (%d/%d)\n",
                   intentos + 1, MAX_INTENTOS);
            sleep(1);
        } else {
            /* Otro error: fallar */
            perror("mq_open");
            return -1;
        }
    }

    fprintf(stderr, "[Monitor] Timeout esperando la cola\n");
    return -1;
}

/*
 * Libera los recursos.
 * El consumidor SI hace mq_unlink: es el "ultimo en irse".
 */
void liberar_cola(void)
{
    if (cola != (mqd_t)-1) {
        mq_close(cola);
    }
    /* Eliminar la cola del sistema */
    mq_unlink(COLA_NOMBRE);
}

/*
 * Formatea un timestamp para mostrar.
 */
void formatear_tiempo(time_t t, char *buffer, size_t len)
{
    struct tm *tm_info;
    tm_info = localtime(&t);
    strftime(buffer, len, "%H:%M:%S", tm_info);
}

int main(int argc, char *argv[])
{
    struct Medicion m;
    char tiempo_str[16];
    int cantidad;
    int i;
    ssize_t bytes;
    double suma;
    double promedio;

    /* Parsear argumento */
    if (argc > 1) {
        cantidad = atoi(argv[1]);
        if (cantidad <= 0) {
            cantidad = CANTIDAD_DEFAULT;
        }
    } else {
        cantidad = CANTIDAD_DEFAULT;
    }

    printf("[Monitor] Iniciando...\n");
    printf("[Monitor] Esperando %d mediciones.\n\n", cantidad);

    /* Inicializar la cola */
    if (inicializar_cola() == -1) {
        fprintf(stderr, "[Monitor] Error inicializando cola\n");
        return EXIT_FAILURE;
    }

    printf("[Monitor] Conectado a la cola.\n\n");

    /* Loop principal: recibir 'cantidad' mediciones */
    suma = 0.0;
    for (i = 0; i < cantidad; i++) {
        /*
         * Recibir un mensaje de la cola.
         *
         * Si la cola esta vacia (no hay mensajes),
         * mq_receive se BLOQUEA automaticamente hasta que llegue uno.
         *
         * Comparacion con modulo 06:
         * - Alla haciamos: sem_wait(llenos) + sem_wait(mutex) + leer + sem_post(mutex) + sem_post(vacios)
         * - Aca es solo: mq_receive (la sincronizacion viene incluida)
         *
         * Nota: el tamaño del buffer debe ser >= mq_msgsize de la cola.
         */
        bytes = mq_receive(cola, (char *)&m, sizeof(m), NULL);
        if (bytes == -1) {
            perror("mq_receive");
            liberar_cola();
            return EXIT_FAILURE;
        }

        /* Formatear el timestamp */
        formatear_tiempo(m.timestamp, tiempo_str, sizeof(tiempo_str));

        /* Mostrar la medicion */
        printf("[Monitor] [%s] Sensor %d: %.2f C\n",
               tiempo_str, m.sensor_id, m.temperatura);

        suma += m.temperatura;
    }

    /* Calcular y mostrar el promedio */
    promedio = suma / cantidad;
    printf("\n[Monitor] Terminado. Se recibieron %d mediciones.\n", cantidad);
    printf("[Monitor] Temperatura promedio: %.2f C\n", promedio);

    /* Limpiar (incluyendo mq_unlink) */
    liberar_cola();

    return EXIT_SUCCESS;
}
