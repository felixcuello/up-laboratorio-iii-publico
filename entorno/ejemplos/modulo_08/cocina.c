/*
 * Ejemplo del restaurante con threads: varios mozos atienden pedidos
 * de una lista compartida, protegida por un mutex.
 *
 * Uso: ./cocina [num_mozos] [num_pedidos]
 *   num_mozos:   cantidad de mozos/threads (por defecto 3)
 *   num_pedidos: cantidad de pedidos a generar (por defecto 15)
 *
 * Compilacion:
 *   gcc -std=c89 -D_XOPEN_SOURCE=700 -pedantic -Wall -Werror cocina.c -o cocina -lpthread
 *
 * Este ejemplo demuestra:
 * - Creacion de multiples threads con pthread_create
 * - Paso de datos a threads mediante estructuras
 * - Proteccion de datos compartidos con mutex
 * - Espera de threads con pthread_join
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include "restaurante.h"

/* Tiempo de atencion de un pedido (en microsegundos) */
#define TIEMPO_MIN_US  100000   /* 100 ms minimo */
#define TIEMPO_VAR_US  200000   /* hasta 200 ms adicionales */

/* Valores por defecto */
#define DEFAULT_MOZOS   3
#define DEFAULT_PEDIDOS 15

/*
 * Busca el proximo pedido pendiente y lo marca como EN_PROCESO.
 * Devuelve el indice del pedido, o -1 si no hay mas pendientes.
 *
 * IMPORTANTE: esta funcion asume que el mutex YA esta tomado.
 * El caller es responsable de hacer lock/unlock.
 */
int buscar_pedido_pendiente(struct Restaurante *rest, int mozo_id)
{
    int i;

    for (i = 0; i < rest->total_pedidos; i++) {
        if (rest->pedidos[i].estado == PEDIDO_PENDIENTE) {
            /* Marcar como en proceso y asignar al mozo */
            rest->pedidos[i].estado = PEDIDO_EN_PROCESO;
            rest->pedidos[i].atendido_por = mozo_id;
            return i;
        }
    }

    return -1;  /* No hay pedidos pendientes */
}

/*
 * Marca un pedido como completado.
 *
 * IMPORTANTE: esta funcion asume que el mutex YA esta tomado.
 */
void completar_pedido(struct Restaurante *rest, int indice)
{
    rest->pedidos[indice].estado = PEDIDO_COMPLETADO;
    rest->pedidos_completados++;
}

/*
 * Funcion que ejecuta cada thread mozo.
 *
 * Recibe un puntero a struct DatosMozo con su ID y el puntero
 * al restaurante. Cada mozo:
 * 1. Busca un pedido pendiente (con mutex)
 * 2. Lo atiende (simula trabajo con usleep)
 * 3. Lo marca como completado (con mutex)
 * 4. Repite hasta que no haya mas pedidos
 */
void *funcion_mozo(void *arg)
{
    struct DatosMozo *datos = (struct DatosMozo *)arg;
    struct Restaurante *rest = datos->restaurante;
    int pedido_idx;
    int tiempo_atencion;

    printf("[Mozo %d] Listo para trabajar\n", datos->id);

    while (1) {
        /* Buscar un pedido pendiente */
        pthread_mutex_lock(&rest->mutex);
        pedido_idx = buscar_pedido_pendiente(rest, datos->id);
        pthread_mutex_unlock(&rest->mutex);

        if (pedido_idx == -1) {
            /* No hay mas pedidos pendientes */
            break;
        }

        /* Atender el pedido */
        printf("[Mozo %d] Atendiendo pedido #%d: %s\n",
               datos->id,
               rest->pedidos[pedido_idx].numero,
               rest->pedidos[pedido_idx].descripcion);

        /* Simular tiempo de atencion variable */
        tiempo_atencion = TIEMPO_MIN_US + (rand() % TIEMPO_VAR_US);
        usleep(tiempo_atencion);

        /* Marcar como completado */
        pthread_mutex_lock(&rest->mutex);
        completar_pedido(rest, pedido_idx);
        pthread_mutex_unlock(&rest->mutex);

        printf("[Mozo %d] Completo pedido #%d\n",
               datos->id, rest->pedidos[pedido_idx].numero);

        datos->pedidos_atendidos++;
    }

    printf("[Mozo %d] Termino. Atendio %d pedidos.\n",
           datos->id, datos->pedidos_atendidos);

    return NULL;
}

/*
 * Genera los pedidos iniciales del restaurante.
 */
void generar_pedidos(struct Restaurante *rest, int cantidad)
{
    int i;
    const char *platos[] = {
        "Pizza Margherita",
        "Hamburguesa clasica",
        "Ensalada Caesar",
        "Milanesa napolitana",
        "Pasta carbonara",
        "Empanadas x6",
        "Lomo completo",
        "Ravioles caseros",
        "Bife de chorizo",
        "Tarta de verduras"
    };
    int num_platos = 10;

    if (cantidad > MAX_PEDIDOS) {
        cantidad = MAX_PEDIDOS;
    }

    rest->total_pedidos = cantidad;
    rest->pedidos_completados = 0;

    for (i = 0; i < cantidad; i++) {
        rest->pedidos[i].numero = i + 1;
        strncpy(rest->pedidos[i].descripcion,
                platos[i % num_platos],
                MAX_DESCRIPCION - 1);
        rest->pedidos[i].descripcion[MAX_DESCRIPCION - 1] = '\0';
        rest->pedidos[i].estado = PEDIDO_PENDIENTE;
        rest->pedidos[i].atendido_por = -1;
    }
}

/*
 * Muestra el resumen final de los pedidos.
 */
void mostrar_resumen(struct Restaurante *rest, struct DatosMozo *mozos, int num_mozos)
{
    int i;

    printf("\n========================================\n");
    printf("           RESUMEN DEL SERVICIO\n");
    printf("========================================\n\n");

    printf("Pedidos atendidos por cada mozo:\n");
    for (i = 0; i < num_mozos; i++) {
        printf("  Mozo %d: %d pedidos\n", mozos[i].id, mozos[i].pedidos_atendidos);
    }

    printf("\nDetalle de pedidos:\n");
    for (i = 0; i < rest->total_pedidos; i++) {
        printf("  Pedido #%2d: %-25s -> Mozo %d\n",
               rest->pedidos[i].numero,
               rest->pedidos[i].descripcion,
               rest->pedidos[i].atendido_por);
    }

    printf("\nTotal: %d pedidos completados\n", rest->pedidos_completados);
}

int main(int argc, char *argv[])
{
    struct Restaurante restaurante;
    struct DatosMozo datos_mozos[MAX_MOZOS];
    pthread_t hilos[MAX_MOZOS];
    int num_mozos;
    int num_pedidos;
    int i;
    int error;

    /* Parsear argumentos */
    num_mozos = (argc > 1) ? atoi(argv[1]) : DEFAULT_MOZOS;
    num_pedidos = (argc > 2) ? atoi(argv[2]) : DEFAULT_PEDIDOS;

    /* Validar rangos */
    if (num_mozos < 1 || num_mozos > MAX_MOZOS) {
        fprintf(stderr, "Cantidad de mozos debe estar entre 1 y %d\n", MAX_MOZOS);
        return EXIT_FAILURE;
    }
    if (num_pedidos < 1 || num_pedidos > MAX_PEDIDOS) {
        fprintf(stderr, "Cantidad de pedidos debe estar entre 1 y %d\n", MAX_PEDIDOS);
        return EXIT_FAILURE;
    }

    /* Inicializar */
    printf("========================================\n");
    printf("     RESTAURANTE CON %d MOZOS\n", num_mozos);
    printf("========================================\n\n");

    srand(time(NULL));
    pthread_mutex_init(&restaurante.mutex, NULL);
    generar_pedidos(&restaurante, num_pedidos);

    printf("Se generaron %d pedidos.\n\n", restaurante.total_pedidos);

    /* Crear los threads mozos */
    for (i = 0; i < num_mozos; i++) {
        datos_mozos[i].id = i + 1;
        datos_mozos[i].restaurante = &restaurante;
        datos_mozos[i].pedidos_atendidos = 0;

        error = pthread_create(&hilos[i], NULL, funcion_mozo, &datos_mozos[i]);
        if (error != 0) {
            fprintf(stderr, "Error creando thread para mozo %d\n", i + 1);
            return EXIT_FAILURE;
        }
    }

    /* Esperar a que todos los mozos terminen */
    for (i = 0; i < num_mozos; i++) {
        pthread_join(hilos[i], NULL);
    }

    /* Mostrar resumen */
    mostrar_resumen(&restaurante, datos_mozos, num_mozos);

    /* Limpiar */
    pthread_mutex_destroy(&restaurante.mutex);

    return EXIT_SUCCESS;
}
