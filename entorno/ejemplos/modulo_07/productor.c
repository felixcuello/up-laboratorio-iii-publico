/*
 * Sensor IoT simulado. Genera mediciones de temperatura y las envia
 * a una cola de mensajes POSIX.
 *
 * Uso: ./productor [cantidad]
 *   cantidad: numero de mediciones a generar (por defecto 20)
 *
 * Compilacion:
 *   gcc -std=c89 -D_XOPEN_SOURCE=700 -pedantic -Wall -Werror productor.c -o productor -lrt
 *
 * Comparacion con el modulo 06 (memoria compartida + semaforos):
 * - NO hay semaforos manuales (sem_wait, sem_post)
 * - NO hay buffer circular ni aritmetica modular
 * - NO hay inicializacion de indices
 * - El bloqueo cuando la cola esta llena es AUTOMATICO en mq_send
 *
 * NOTA: Si el programa se interrumpe con Ctrl+C antes de terminar,
 *       la cola queda en /dev/mqueue/ y hay que borrarla manualmente:
 *       rm /dev/mqueue/sensor_iot_cola
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <mqueue.h>

#include "mensajes.h"

/* Intervalo entre mediciones en microsegundos (500 ms) */
#define INTERVALO_US 500000

/* Cantidad por defecto de mediciones a generar */
#define CANTIDAD_DEFAULT 20

/* Rango de temperaturas simuladas */
#define TEMP_MIN 15.0
#define TEMP_MAX 35.0

/* Variable global para el descriptor de la cola */
static mqd_t cola = (mqd_t)-1;

/*
 * Genera una medicion simulada con valores aleatorios.
 */
struct Medicion generar_medicion(int sensor_id)
{
    struct Medicion m;
    double rango;

    m.timestamp = time(NULL);
    m.sensor_id = sensor_id;

    /* Temperatura aleatoria entre TEMP_MIN y TEMP_MAX */
    rango = TEMP_MAX - TEMP_MIN;
    m.temperatura = TEMP_MIN + ((double)rand() / RAND_MAX) * rango;

    return m;
}

/*
 * Inicializa la cola de mensajes.
 * El productor es el "creador": hace O_CREAT y define los atributos.
 *
 * Comparacion con modulo 06:
 * - Alla habia que crear shm_open + ftruncate + mmap + 3x sem_open
 * - Aca es solo mq_open con atributos
 */
int inicializar_cola(void)
{
    struct mq_attr attr;

    /* Configurar los atributos de la cola */
    attr.mq_flags = 0;                      /* Bloqueante */
    attr.mq_maxmsg = COLA_MAX_MENSAJES;     /* Capacidad del buffer */
    attr.mq_msgsize = COLA_TAM_MENSAJE;     /* Tamaño de cada mensaje */
    attr.mq_curmsgs = 0;                    /* Ignorado en mq_open */

    /* Crear la cola (O_CREAT) para escritura (O_WRONLY) */
    cola = mq_open(COLA_NOMBRE, O_CREAT | O_WRONLY, 0600, &attr);
    if (cola == (mqd_t)-1) {
        perror("mq_open");
        return -1;
    }

    return 0;
}

/*
 * Libera los recursos.
 * El productor NO hace mq_unlink: deja eso al consumidor (el ultimo en irse).
 */
void liberar_cola(void)
{
    if (cola != (mqd_t)-1) {
        mq_close(cola);
    }
}

int main(int argc, char *argv[])
{
    struct Medicion m;
    int sensor_id;
    int cantidad;
    int i;

    /* Parsear argumento */
    if (argc > 1) {
        cantidad = atoi(argv[1]);
        if (cantidad <= 0) {
            cantidad = CANTIDAD_DEFAULT;
        }
    } else {
        cantidad = CANTIDAD_DEFAULT;
    }

    /* Semilla para numeros aleatorios */
    srand(time(NULL));
    sensor_id = 1;

    printf("[Sensor %d] Iniciando...\n", sensor_id);
    printf("[Sensor %d] Generando %d mediciones.\n\n", sensor_id, cantidad);

    /* Inicializar la cola */
    if (inicializar_cola() == -1) {
        fprintf(stderr, "[Sensor %d] Error inicializando cola\n", sensor_id);
        return EXIT_FAILURE;
    }

    /* Loop principal: generar 'cantidad' mediciones */
    for (i = 0; i < cantidad; i++) {
        /* Generar la medicion */
        m = generar_medicion(sensor_id);

        /*
         * Enviar el mensaje a la cola.
         *
         * Si la cola esta llena (tiene COLA_MAX_MENSAJES mensajes),
         * mq_send se BLOQUEA automaticamente hasta que haya espacio.
         *
         * Comparacion con modulo 06:
         * - Alla haciamos: sem_wait(vacios) + sem_wait(mutex) + escribir + sem_post(mutex) + sem_post(llenos)
         * - Aca es solo: mq_send (la sincronizacion viene incluida)
         */
        if (mq_send(cola, (char *)&m, sizeof(m), 0) == -1) {
            perror("mq_send");
            liberar_cola();
            return EXIT_FAILURE;
        }

        /* Mostrar lo que hicimos */
        printf("[Sensor %d] Medicion #%d: %.2f C\n",
               sensor_id, i + 1, m.temperatura);

        /* Esperar antes de la proxima medicion */
        usleep(INTERVALO_US);
    }

    printf("\n[Sensor %d] Terminado. Se enviaron %d mediciones.\n", sensor_id, cantidad);

    /* Limpiar */
    liberar_cola();

    return EXIT_SUCCESS;
}
