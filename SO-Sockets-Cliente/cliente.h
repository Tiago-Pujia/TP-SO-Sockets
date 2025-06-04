#ifndef CLIENTE_H_INCLUDED
#define CLIENTE_H_INCLUDED

/*
============================================================
             BIBLIOTECAS
============================================================
*/

#include <stdio.h>        // Funciones de entrada/salida est�ndar: printf, perror, fgets, etc.
#include <stdlib.h>       // Funciones de utilidad general: exit, malloc, free, rand, srand, etc.
#include <string.h>       // Manipulaci�n de cadenas: strlen, strcmp, strncmp, strcpy, snprintf, etc.
#include <unistd.h>       // Funciones POSIX: close, sleep, read, write, etc.
#include <pthread.h>      // Manejo de hilos POSIX: pthread_create, pthread_join, etc.
#include <arpa/inet.h>    // Funciones para manejo de direcciones IP: inet_pton, htons, etc.
#include <signal.h>       // Gesti�n de se�ales del sistema: signal, SIGINT, SIGTERM, etc.
#include <netinet/in.h>   // Definiciones de estructuras para sockets: sockaddr_in, INADDR_ANY, etc.
#include <time.h>         // Funciones de tiempo y generaci�n de n�meros aleatorios: time, time_t, etc.

/*
============================================================
                      CONSTANTES
============================================================
*/

#define PUERTO 8080      // Puerto al que se conecta el cliente
#define BUFFER_SIZE 128  // Tama�o m�ximo del buffer para enviar/recibir datos

/*
============================================================
                PROTOTIPOS DE FUNCIONES
============================================================
*/

/**
 * Funci�n principal que inicializa y ejecuta la l�gica del cliente.
 *
 * Se conecta al servidor, lanza hilos para enviar y recibir datos, y espera
 * que el usuario escriba "salir" para finalizar.
 */
void iniciar_cliente();

/**
 * Hilo encargado de recibir mensajes desde el servidor y mostrarlos por consola.
 *
 * @param arg Puntero al descriptor del socket conectado al servidor.
 */
void *escuchar_servidor(void *arg);

/**
 * Hilo encargado de generar y enviar datos peri�dicos (temperatura o humedad).
 *
 * @param arg Puntero al descriptor del socket conectado al servidor.
 */
void* enviar_datos(void* arg);

#endif // CLIENTE_H_INCLUDED
