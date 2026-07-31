/*
 * Monitor IoT. Lee mediciones de temperatura de un buffer circular
 * en memoria compartida y las muestra por pantalla.
 *
 * Uso: ./consumidor [cantidad]
 *   cantidad: numero de mediciones a leer (por defecto 20)
 *
 * Compilacion:
 *   gcc -std=c89 -pedantic -Wall -Werror consumidor.c -o consumidor -lpthread -lrt
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
#include <errno.h>

#include "compartido.h"

/* Cantidad por defecto de mediciones a leer */
#define CANTIDAD_DEFAULT 20

/* Variables globales */
static struct BufferCompartido *buffer = NULL;
static sem_t *sem_vacios = NULL;
static sem_t *sem_llenos = NULL;
static sem_t *sem_mutex = NULL;

/*
 * Formatea un timestamp en una cadena legible.
 */
void formatear_tiempo(time_t t, char *buf, size_t size)
{
    struct tm *tm_info;

    tm_info = localtime(&t);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

/*
 * Inicializa los recursos IPC.
 * El consumidor NO crea: abre lo que el productor ya creo.
 */
int inicializar_recursos(void)
{
    int shm_fd;
    int intentos;

    /* Abrir la memoria compartida (sin O_CREAT: debe existir) */
    intentos = 0;
    shm_fd = -1;
    while (intentos < 10) {
        shm_fd = shm_open(SHM_NOMBRE, O_RDWR, 0);
        if (shm_fd != -1) {
            break;
        }
        if (errno == ENOENT) {
            /* El productor todavia no arranco, esperamos un poco */
            printf("[Monitor] Esperando al sensor...\n");
            sleep(1);
            intentos++;
        } else {
            perror("shm_open");
            return -1;
        }
    }

    if (shm_fd == -1) {
        fprintf(stderr, "[Monitor] El sensor no esta corriendo.\n");
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

    /* Abrir los semaforos (sin O_CREAT) */
    sem_vacios = sem_open(SEM_VACIOS_NOMBRE, 0);
    if (sem_vacios == SEM_FAILED) {
        perror("sem_open vacios");
        return -1;
    }

    sem_llenos = sem_open(SEM_LLENOS_NOMBRE, 0);
    if (sem_llenos == SEM_FAILED) {
        perror("sem_open llenos");
        return -1;
    }

    sem_mutex = sem_open(SEM_MUTEX_NOMBRE, 0);
    if (sem_mutex == SEM_FAILED) {
        perror("sem_open mutex");
        return -1;
    }

    return 0;
}

/*
 * Libera los recursos IPC.
 * El consumidor ES el que hace unlink: es el "ultimo en irse".
 */
void liberar_recursos(void)
{
    if (buffer != NULL && buffer != MAP_FAILED) {
        munmap(buffer, sizeof(struct BufferCompartido));
    }
    if (sem_vacios != NULL && sem_vacios != SEM_FAILED) {
        sem_close(sem_vacios);
        sem_unlink(SEM_VACIOS_NOMBRE);
    }
    if (sem_llenos != NULL && sem_llenos != SEM_FAILED) {
        sem_close(sem_llenos);
        sem_unlink(SEM_LLENOS_NOMBRE);
    }
    if (sem_mutex != NULL && sem_mutex != SEM_FAILED) {
        sem_close(sem_mutex);
        sem_unlink(SEM_MUTEX_NOMBRE);
    }

    /* Eliminar la memoria compartida */
    shm_unlink(SHM_NOMBRE);
}

int main(int argc, char *argv[])
{
    struct Medicion m;
    int cantidad;
    int i;
    char tiempo_str[32];

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

    /* Inicializar recursos IPC */
    if (inicializar_recursos() == -1) {
        fprintf(stderr, "[Monitor] Error inicializando recursos\n");
        return EXIT_FAILURE;
    }

    printf("[Monitor] Conectado al sensor. Recibiendo mediciones...\n\n");

    /* Loop principal: leer 'cantidad' mediciones */
    for (i = 0; i < cantidad; i++) {
        /* Esperar a que haya un elemento en el buffer */
        sem_wait(sem_llenos);

        /* Tomar el mutex para acceder al buffer */
        sem_wait(sem_mutex);

        /* Leer la medicion de la posicion cola */
        m = buffer->mediciones[buffer->cola];

        /* Avanzar cola con aritmetica modular */
        buffer->cola = (buffer->cola + 1) % BUFFER_CAPACIDAD;

        /* Liberar el mutex */
        sem_post(sem_mutex);

        /* Avisar que hay un espacio libre */
        sem_post(sem_vacios);

        /* Procesar la medicion (mostrarla) */
        formatear_tiempo(m.timestamp, tiempo_str, sizeof(tiempo_str));
        printf("[Monitor] #%d | Sensor %d | %s | %.2f C\n",
               i + 1, m.sensor_id, tiempo_str, m.temperatura);
    }

    printf("\n[Monitor] Terminado. Se recibieron %d mediciones.\n", cantidad);

    /* Limpiar (incluyendo unlink) */
    liberar_recursos();

    printf("[Monitor] Recursos eliminados del sistema.\n");

    return EXIT_SUCCESS;
}
