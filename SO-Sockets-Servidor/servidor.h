#ifndef SERVIDOR_H_INCLUDED
#define SERVIDOR_H_INCLUDED

/*
============================================================
                    BIBLIOTECAS
============================================================
*/

#include <stdio.h>        // Entrada/salida est�ndar (printf, perror, etc.)
#include <stdlib.h>       // Manejo de memoria din�mica y finalizaci�n del programa
#include <string.h>       // Manipulaci�n de cadenas (strlen, snprintf, etc.)
#include <unistd.h>       // Funciones del sistema POSIX (close, sleep, etc.)
#include <pthread.h>      // Manejo de hilos y sincronizaci�n (mutex, cond)
#include <arpa/inet.h>    // Conversi�n y manejo de direcciones IP
#include <signal.h>       // Manejo de se�ales del sistema (SIGINT, SIGPIPE)
#include <netinet/in.h>   // Estructuras para sockets TCP (sockaddr_in)
#include <time.h>         // Tiempo y aleatoriedad (time, rand, srand)

/*
============================================================
                      CONSTANTES
============================================================
*/

#define PUERTO 8080      // Puerto TCP en el que escucha el servidor
#define BUFFER_SIZE 128  // Tama�o del buffer para mensajes de red
#define MAX_CLIENTES 3   // M�ximo de clientes concurrentes aceptados

/*
============================================================
                    ESTRUCTURAS DE DATOS
============================================================
*/

/**
 * Representa la informaci�n de un cliente conectado.
 * Contiene el descriptor del socket y la direcci�n del cliente.
 */
typedef struct
{
    int socket;                   // Descriptor del socket del cliente
    struct sockaddr_in direccion; // Direcci�n IP y puerto del cliente
} ClienteInfo;

/**
 * Estructura central que gestiona todos los clientes conectados.
 * Incluye un arreglo de sockets activos, un contador, un mutex para control
 * de concurrencia y el descriptor del socket del servidor.
 */
typedef struct
{
    int clientes[MAX_CLIENTES]; // Lista de sockets activos
    int cantidad_clientes;      // Cantidad actual de clientes conectados
    pthread_mutex_t mutex;      // Mutex para proteger el acceso concurrente
    int servidor_socket;        // Descriptor del socket principal del servidor
} GestorClientes;

/*
============================================================
                   PROTOTIPOS DE FUNCIONES
============================================================
*/

/**
 * Inicializa y ejecuta el servidor TCP.
 * Escucha conexiones entrantes, acepta clientes y crea hilos para atenderlos.
 */
void iniciar_servidor();

/**
 * Funci�n ejecutada por cada hilo que atiende a un cliente.
 * Recibe y procesa los mensajes, emite alertas si es necesario.
 * @param arg Paquete con ClienteInfo* y GestorClientes*
 */
void *atender_cliente(void *arg);

/**
 * Env�a un mensaje de alerta a todos los clientes conectados.
 */
void enviar_alerta_todos(GestorClientes *gestor, const char *mensaje)

/**
 * Cierra todos los sockets, recursos y finaliza el servidor limpiamente.
 */
void cerrar_servidor();

/**
 * Maneja se�ales del sistema o desconexiones abruptas.
 * @param senial N�mero de se�al recibida (ej: SIGINT).
 */
void manejar_senial(int senial);

#endif // SERVIDOR_H_INCLUDED
