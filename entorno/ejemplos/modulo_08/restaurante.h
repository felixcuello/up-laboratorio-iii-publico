/*
 * Header compartido para el ejemplo del restaurante con threads.
 * Define las estructuras y constantes que usan tanto el main
 * como la funcion de los threads mozos.
 */

#ifndef RESTAURANTE_H
#define RESTAURANTE_H

#include <pthread.h>

/*
 * Configuracion del restaurante.
 */
#define MAX_PEDIDOS     50      /* Capacidad maxima de pedidos */
#define MAX_MOZOS       8       /* Cantidad maxima de mozos */
#define MAX_DESCRIPCION 64      /* Longitud maxima de descripcion */

/*
 * Estados de un pedido.
 */
#define PEDIDO_PENDIENTE  0
#define PEDIDO_EN_PROCESO 1
#define PEDIDO_COMPLETADO 2

/*
 * Estructura que representa un pedido individual.
 */
struct Pedido {
    int numero;                         /* Numero de pedido (1, 2, 3, ...) */
    char descripcion[MAX_DESCRIPCION];  /* Descripcion del pedido */
    int estado;                         /* PENDIENTE, EN_PROCESO, COMPLETADO */
    int atendido_por;                   /* ID del mozo que lo atendio (-1 si nadie) */
};

/*
 * Estructura que representa el estado compartido del restaurante.
 * Esta estructura es accedida por todos los threads, por lo que
 * el acceso debe estar protegido por el mutex.
 */
struct Restaurante {
    struct Pedido pedidos[MAX_PEDIDOS]; /* Lista de pedidos */
    int total_pedidos;                  /* Cuantos pedidos hay en la lista */
    int pedidos_completados;            /* Cuantos ya se completaron */
    pthread_mutex_t mutex;              /* Mutex para proteger el acceso */
};

/*
 * Estructura con los datos que recibe cada thread mozo.
 * Cada mozo tiene su propia copia de esta estructura.
 */
struct DatosMozo {
    int id;                             /* Identificador del mozo (1, 2, 3, ...) */
    struct Restaurante *restaurante;    /* Puntero al estado compartido */
    int pedidos_atendidos;              /* Contador local: cuantos atendio este mozo */
};

#endif /* RESTAURANTE_H */
