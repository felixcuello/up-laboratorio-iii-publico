/*
 * compartido.h
 *
 * Header compartido entre el sensor (productor) y el monitor (consumidor).
 * Define las constantes, el nombre de los recursos IPC, y las estructuras
 * de datos que viven en la memoria compartida.
 *
 * IMPORTANTE: ambos procesos deben incluir este mismo archivo para
 * garantizar que el layout de la memoria compartida sea identico.
 */

#ifndef COMPARTIDO_H
#define COMPARTIDO_H

#include <time.h>

/*
 * Nombres de los recursos IPC.
 * Todos deben empezar con '/' segun POSIX.
 */
#define SHM_NOMBRE          "/sensor_iot_buffer"
#define SEM_VACIOS_NOMBRE   "/sensor_iot_vacios"
#define SEM_LLENOS_NOMBRE   "/sensor_iot_llenos"
#define SEM_MUTEX_NOMBRE    "/sensor_iot_mutex"

/*
 * Capacidad del buffer circular.
 * Define cuantas mediciones caben antes de que el productor se bloquee.
 */
#define BUFFER_CAPACIDAD 10

/*
 * Estructura que representa una medicion del sensor.
 */
struct Medicion {
    time_t timestamp;       /* Momento en que se tomo la medicion */
    double temperatura;     /* Valor de la temperatura en grados Celsius */
    int sensor_id;          /* Identificador del sensor (por si hay varios) */
};

/*
 * Estructura que representa el buffer compartido completo.
 * Esta estructura es lo que vive en la memoria compartida.
 */
struct BufferCompartido {
    struct Medicion mediciones[BUFFER_CAPACIDAD];
    int cabeza;             /* Indice donde el productor escribe */
    int cola;               /* Indice donde el consumidor lee */
};

#endif /* COMPARTIDO_H */
