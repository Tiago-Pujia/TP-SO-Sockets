#ifndef CLIENTE_H_INCLUDED
#define CLIENTE_H_INCLUDED

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


void iniciar_cliente();
void *escuchar_servidor(void *arg);
void* enviar_datos(void* arg);
#endif // CLIENTE_H_INCLUDED
