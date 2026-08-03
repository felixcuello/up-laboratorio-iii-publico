/*
 * Sensor IoT simulado. Genera mediciones de temperatura y las deposita
 * en un buffer circular en memoria compartida.
 *
 * Uso: ./productor [cantidad]
 *   cantidad: numero de mediciones a generar (por defecto 20)
 *
 * Compilacion:
 *   gcc -std=c89 -D_XOPEN_SOURCE=700 -pedantic -Wall -Werror productor.c -o productor -lpthread -lrt
 *
 * NOTA: Si el programa se interrumpe con Ctrl+C antes de terminar,
 *       los recursos IPC quedan en /dev/shm/ y hay que borrarlos manualmente:
 *       rm /dev/shm/sensor_iot*
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "compartido.h"

/* Intervalo entre mediciones en microsegundos (500 ms) */
#define INTERVALO_US 500000

/* Cantidad por defecto de mediciones a generar */
#define CANTIDAD_DEFAULT 20

/* Rango de temperaturas simuladas */
#define TEMP_MIN 15.0
#define TEMP_MAX 35.0

/* Variables globales */
static struct BufferCompartido *buffer = NULL;
static sem_t *sem_vacios = NULL;
static sem_t *sem_llenos = NULL;
static sem_t *sem_mutex = NULL;

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
 * Inicializa los recursos IPC: memoria compartida y semaforos.
 * El productor es el "creador": hace O_CREAT y ftruncate.
 */
int inicializar_recursos(void)
{
    int shm_fd;

    /* Crear la memoria compartida */
    shm_fd = shm_open(SHM_NOMBRE, O_CREAT | O_RDWR, 0600);
    if (shm_fd == -1) {
        perror("shm_open");
        return -1;
    }

    /* Definir el tamano */
    if (ftruncate(shm_fd, sizeof(struct BufferCompartido)) == -1) {
        perror("ftruncate");
        close(shm_fd);
        return -1;
    }

    /* Mapear en el espacio de direcciones */
    buffer = mmap(NULL, sizeof(struct BufferCompartido),
                  PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (buffer == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        return -1;
    }

    /* Ya no necesitamos el fd */
    close(shm_fd);

    /* Inicializar los indices del buffer (el productor es el primero) */
    buffer->cabeza = 0;
    buffer->cola = 0;

    /* Crear los semaforos */
    sem_vacios = sem_open(SEM_VACIOS_NOMBRE, O_CREAT, 0600, BUFFER_CAPACIDAD);
    if (sem_vacios == SEM_FAILED) {
        perror("sem_open vacios");
        return -1;
    }

    sem_llenos = sem_open(SEM_LLENOS_NOMBRE, O_CREAT, 0600, 0);
    if (sem_llenos == SEM_FAILED) {
        perror("sem_open llenos");
        return -1;
    }

    sem_mutex = sem_open(SEM_MUTEX_NOMBRE, O_CREAT, 0600, 1);
    if (sem_mutex == SEM_FAILED) {
        perror("sem_open mutex");
        return -1;
    }

    return 0;
}

/*
 * Libera los recursos IPC.
 * El productor NO hace unlink: deja eso al consumidor (el ultimo en irse).
 */
void liberar_recursos(void)
{
    if (buffer != NULL && buffer != MAP_FAILED) {
        munmap(buffer, sizeof(struct BufferCompartido));
    }
    if (sem_vacios != NULL && sem_vacios != SEM_FAILED) {
        sem_close(sem_vacios);
    }
    if (sem_llenos != NULL && sem_llenos != SEM_FAILED) {
        sem_close(sem_llenos);
    }
    if (sem_mutex != NULL && sem_mutex != SEM_FAILED) {
        sem_close(sem_mutex);
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

    /* Inicializar recursos IPC */
    if (inicializar_recursos() == -1) {
        fprintf(stderr, "[Sensor %d] Error inicializando recursos\n", sensor_id);
        return EXIT_FAILURE;
    }

    /* Loop principal: generar 'cantidad' mediciones */
    for (i = 0; i < cantidad; i++) {
        /* Generar la medicion */
        m = generar_medicion(sensor_id);

        /* Esperar a que haya espacio en el buffer */
        sem_wait(sem_vacios);

        /* Tomar el mutex para acceder al buffer */
        sem_wait(sem_mutex);

        /* Escribir la medicion en la posicion cabeza */
        buffer->mediciones[buffer->cabeza] = m;

        /* Avanzar cabeza con aritmetica modular */
        buffer->cabeza = (buffer->cabeza + 1) % BUFFER_CAPACIDAD;

        /* Liberar el mutex */
        sem_post(sem_mutex);

        /* Avisar que hay un elemento nuevo */
        sem_post(sem_llenos);

        /* Mostrar lo que hicimos */
        printf("[Sensor %d] Medicion #%d: %.2f C\n",
               sensor_id, i + 1, m.temperatura);

        /* Esperar antes de la proxima medicion */
        usleep(INTERVALO_US);
    }

    printf("\n[Sensor %d] Terminado. Se generaron %d mediciones.\n", sensor_id, cantidad);

    /* Limpiar */
    liberar_recursos();

    return EXIT_SUCCESS;
}
