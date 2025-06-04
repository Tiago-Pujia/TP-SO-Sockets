#ifndef SERVIDOR_H_INCLUDED
#define SERVIDOR_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>

#define PUERTO 8080
#define BUFFER_SIZE 128
#include <netinet/in.h>
#include <time.h>
#define MAX_CLIENTES 3

typedef struct
{
    int socket;
    struct sockaddr_in direccion;
}ClienteInfo;

typedef struct
{
    int clientes[MAX_CLIENTES];
    int cantidad_clientes;
    pthread_mutex_t mutex;
    int servidor_socket;
}GestorClientes;

void iniciar_servidor();
void *atender_cliente(void *arg);
void enviar_alerta_a_todos(const char *mensaje);
void cerrar_servidor();
void manejar_senial(int senial);

#endif // SERVIDOR_H_INCLUDED
