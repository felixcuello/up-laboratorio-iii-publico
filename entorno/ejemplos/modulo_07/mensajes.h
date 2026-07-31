/*
 * 07_mensajes.h
 *
 * Header compartido entre el sensor (productor) y el monitor (consumidor).
 * Define las constantes y la estructura del mensaje.
 *
 * IMPORTANTE: ambos procesos deben incluir este mismo archivo para
 * garantizar que el formato del mensaje sea identico.
 *
 * Comparacion con el modulo 06:
 * - Ya no hay indices de buffer (cabeza/cola): la cola los maneja.
 * - Ya no hay nombres de semaforos: la sincronizacion viene incluida.
 * - Solo definimos el nombre de la cola y la estructura del mensaje.
 */

#ifndef MENSAJES_H
#define MENSAJES_H

#include <time.h>

/*
 * Nombre de la cola de mensajes.
 * Debe empezar con '/' segun POSIX.
 */
#define COLA_NOMBRE "/sensor_iot_cola"

/*
 * Atributos de la cola.
 * - COLA_MAX_MENSAJES: cuantos mensajes caben antes de que el productor se bloquee.
 * - COLA_TAM_MENSAJE: tamaño maximo de cada mensaje en bytes.
 *
 * Nota: COLA_TAM_MENSAJE debe ser >= sizeof(struct Medicion).
 */
#define COLA_MAX_MENSAJES 10   /* ver: cat /proc/sys/fs/mqueue/msg_max */
#define COLA_TAM_MENSAJE  sizeof(struct Medicion)

/*
 * Estructura que representa una medicion del sensor.
 * Es el "mensaje" que viaja por la cola.
 *
 * Esta estructura es identica a la del modulo 06, lo que permite
 * comparar directamente ambas implementaciones.
 */
struct Medicion {
    time_t timestamp;       /* Momento en que se tomo la medicion */
    double temperatura;     /* Valor de la temperatura en grados Celsius */
    int sensor_id;          /* Identificador del sensor (por si hay varios) */
};

#endif /* MENSAJES_H */
