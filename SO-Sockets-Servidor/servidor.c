#include "servidor.h"

/*
============================================================
                      VARIABLES GLOBALES
============================================================
*/
int clientes[MAX_CLIENTES];              // Sockets de clientes conectados
pthread_t hilos[MAX_CLIENTES];           // Identificadores de hilos para cada cliente
int num_clientes = 0;                    // Contador global de clientes conectados
GestorClientes *gestor;                  // Estructura central de gestión de clientes

// Condición para esperar lugar disponible
pthread_cond_t espacio_disponible = PTHREAD_COND_INITIALIZER;

/*
============================================================
                      FUNCIONES PRINCIPALES
============================================================
*/

/**
 * Envía un mensaje de alerta a todos los clientes conectados.
 * @param gestor Puntero al gestor de clientes.
 * @param mensaje Mensaje a enviar (alerta de temperatura o humedad).
 */
void enviar_alerta_todos(GestorClientes *gestor, const char *mensaje)
{
    pthread_mutex_lock(&gestor->mutex); // Acceso exclusivo al arreglo de clientes

    for(int i = 0; i < gestor->cantidad_clientes; i++)
    {
        // Enviar mensaje a cada cliente
        send(gestor->clientes[i], mensaje, strlen(mensaje), 0);
    }

    pthread_mutex_unlock(&gestor->mutex);
}

/**
 * Función que atiende a un cliente individual en un hilo.
 *
 * Recibe mensajes, interpreta valores de temperatura o humedad,
 * responde "OK" si está en rango, o envía alertas si están fuera.
 *
 * @param arg Estructura con cliente + gestor.
 */
void *atender_cliente(void *arg)
{
    // Desempaquetar argumentos (cliente + gestor)
    struct { ClienteInfo *cliente; GestorClientes *gestor; } *paquete = arg;

    ClienteInfo *cliente = paquete->cliente;
    GestorClientes *gestor = paquete->gestor;
    free(paquete);  // Liberar memoria de estructura temporal

    char buffer[BUFFER_SIZE];
    int bytes_recibidos;

    // Bucle de recepción de mensajes
    while((bytes_recibidos = recv(cliente->socket, buffer, BUFFER_SIZE - 1, 0)) > 0)
    {
        buffer[bytes_recibidos] = '\0';              // Asegurar terminador de cadena
        printf("Dato recibido: %s", buffer);          // Mostrar mensaje recibido
        fflush(stdout);

        int valor;

        // Procesar mensaje de temperatura
        if(sscanf(buffer, "TEMPERATURA %d", &valor) == 1)
        {
            if(valor < 0 || valor > 40)
            {
                char alerta[BUFFER_SIZE];
                snprintf(alerta, sizeof(alerta),
                         "ALERTA: temperatura fuera de rango (%d C)\n", valor);
                enviar_alerta_todos(gestor, alerta); // Broadcast de alerta
            }
            else
            {
                send(cliente->socket, "OK\n", 3, 0);  // Confirmación simple
            }
        }
        // Procesar mensaje de humedad
        else if(sscanf(buffer, "HUMEDAD %d", &valor) == 1)
        {
            if(valor < 10 || valor > 90)
            {
                char alerta[BUFFER_SIZE];
                snprintf(alerta, sizeof(alerta),
                         "ALERTA: humedad fuera de rango (%d%%)\n", valor);
                enviar_alerta_todos(gestor, alerta);
            }
            else
            {
                send(cliente->socket, "OK\n", 3, 0);
            }
        }
    }

    // Cliente se desconectó
    close(cliente->socket);  // Cerrar su socket

    pthread_mutex_lock(&gestor->mutex);

    // Eliminar al cliente del arreglo
    for(int i = 0; i < gestor->cantidad_clientes; i++)
    {
        if(gestor->clientes[i] == cliente->socket)
        {
            // Reemplazar con el último cliente
            gestor->clientes[i] = gestor->clientes[gestor->cantidad_clientes - 1];
            gestor->cantidad_clientes--;

            pthread_cond_signal(&espacio_disponible);  // Avisar que hay espacio

            if(gestor->cantidad_clientes == 0)
            {
                printf("No hay más clientes conectados. Cerrando servidor...\n");
                cerrar_servidor(); // Limpieza completa
                exit(0);
            }
            break;
        }
    }

    pthread_mutex_unlock(&gestor->mutex);
    free(cliente);
    return NULL;
}

/**
 * Inicializa y ejecuta el servidor TCP.
 * Acepta múltiples clientes (hasta MAX_CLIENTES) y los gestiona con hilos.
 */
void iniciar_servidor()
{
    int opt = 1;
    signal(SIGINT, manejar_senial);   // Captura Ctrl+C
    gestor = malloc(sizeof(GestorClientes));
    gestor->cantidad_clientes = 0;
    pthread_mutex_init(&gestor->mutex, NULL);  // Inicializar mutex

    // Crear socket TCP
    gestor->servidor_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servidor_addr;

    servidor_addr.sin_family = AF_INET;
    servidor_addr.sin_addr.s_addr = INADDR_ANY;
    servidor_addr.sin_port = htons(PUERTO);

    setsockopt(gestor->servidor_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bind(gestor->servidor_socket, (struct sockaddr *)&servidor_addr, sizeof(servidor_addr));
    listen(gestor->servidor_socket, MAX_CLIENTES);

    printf("Servidor iniciado en puerto %d...\n", PUERTO);

    // Aceptar conexiones continuamente
    while(1)
    {
        ClienteInfo *cliente = malloc(sizeof(ClienteInfo));
        socklen_t cliente_len = sizeof(cliente->direccion);
        cliente->socket = accept(gestor->servidor_socket,
                                 (struct sockaddr *)&cliente->direccion,
                                 &cliente_len);

        pthread_mutex_lock(&gestor->mutex);

        // Esperar si está lleno
        while(gestor->cantidad_clientes >= MAX_CLIENTES)
        {
            printf("[Servidor] Cola llena (%d clientes). Esperando que alguien se desconecte...\n", MAX_CLIENTES);
            fflush(stdout);
            pthread_cond_wait(&espacio_disponible, &gestor->mutex);
        }

        // Agregar nuevo cliente
        gestor->clientes[gestor->cantidad_clientes++] = cliente->socket;
        pthread_mutex_unlock(&gestor->mutex);

        // Crear estructura de argumentos dinámica
        pthread_t hilo;
        void *args = malloc(sizeof(struct { ClienteInfo *cliente; GestorClientes *gestor; }));
        memcpy(args, &(struct { ClienteInfo *cliente; GestorClientes *gestor; }){ cliente, gestor },
               sizeof(struct { ClienteInfo *cliente; GestorClientes *gestor; }));

        // Lanzar hilo para este cliente
        pthread_create(&hilo, NULL, atender_cliente, args);
        pthread_detach(hilo);  // No necesitamos esperar que termine
    }

    close(gestor->servidor_socket);
}

/**
 * Cierra todos los sockets, hilos y recursos del servidor.
 */
void cerrar_servidor()
{
    for(int i = 0; i < num_clientes; i++)
    {
        pthread_cancel(hilos[i]);       // Cancelar hilo (si existe)
        close(clientes[i]);             // Cerrar socket
    }

    close(gestor->servidor_socket);     // Cerrar socket del servidor
    printf("Servidor finalizado.\n");

    pthread_mutex_destroy(&gestor->mutex);
    pthread_cond_destroy(&espacio_disponible);
    free(gestor);
}

/**
 * Maneja señales del sistema.
 * @param senial Señal recibida (ej. SIGINT, SIGPIPE)
 */
void manejar_senial(int senial)
{
    if(senial == SIGINT)
    {
        printf("\n[Servidor] Finalizando por Ctrl+C...\n");
        cerrar_servidor();
        exit(0);
    }
    else if(senial == SIGPIPE)
    {
        printf("[Servidor] Cliente desconectado inesperadamente.\n");
    }
}
