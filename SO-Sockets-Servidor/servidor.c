#include "servidor.h"
int clientes[MAX_CLIENTES];
pthread_t hilos[MAX_CLIENTES];
int num_clientes = 0;
GestorClientes *gestor;

pthread_cond_t espacio_disponible = PTHREAD_COND_INITIALIZER;

void enviar_alerta_todos(GestorClientes *gestor, const char *mensaje)
{
    pthread_mutex_lock(&gestor->mutex);
    for(int i = 0; i < gestor->cantidad_clientes; i++)
    {
        send(gestor->clientes[i], mensaje, strlen(mensaje), 0);
    }
    pthread_mutex_unlock(&gestor->mutex);
}

void *atender_cliente(void *arg)
{
    struct
    {
        ClienteInfo *cliente;
        GestorClientes *gestor;
    } *paquete = arg;

    ClienteInfo *cliente = paquete->cliente;
    GestorClientes *gestor = paquete->gestor;
    free(paquete);

    char buffer[BUFFER_SIZE];
    int bytes_recibidos;

    while((bytes_recibidos = recv(cliente->socket, buffer, BUFFER_SIZE - 1, 0)) > 0)
    {
        buffer[bytes_recibidos] = '\0';
        printf("Dato recibido: %s", buffer);
        fflush(stdout);

        int valor;
        if(sscanf(buffer, "TEMPERATURA %d", &valor) == 1)
        {
            if(valor < 0 || valor > 40)
            {
                char alerta[BUFFER_SIZE];
                snprintf(alerta, sizeof(alerta), "ALERTA: temperatura fuera de rango (%d�C)\n", valor);
                enviar_alerta_todos(gestor, alerta);
            }
            else
            {
                send(cliente->socket, "OK\n", 3, 0);
            }
        }
        else if(sscanf(buffer, "HUMEDAD %d", &valor) == 1)
        {
            if(valor < 10 || valor > 90)
            {
                char alerta[BUFFER_SIZE];
                snprintf(alerta, sizeof(alerta), "ALERTA: humedad fuera de rango (%d%%)\n", valor);
                enviar_alerta_todos(gestor, alerta);
            }
            else
            {
                send(cliente->socket, "OK\n", 3, 0);
            }
        }
    }

    close(cliente->socket);

    pthread_mutex_lock(&gestor->mutex);
    for(int i = 0; i < gestor->cantidad_clientes; i++)
    {
        if(gestor->clientes[i] == cliente->socket)
        {
            gestor->clientes[i] = gestor->clientes[gestor->cantidad_clientes - 1];
            gestor->cantidad_clientes--;

            // avisar que hay lugar para otro cliente
            pthread_cond_signal(&espacio_disponible);

            if(gestor->cantidad_clientes == 0)
            {
                printf("No hay mas clientes conectados. Cerrando servidor...\n");
                cerrar_servidor();
                exit(0);
            }
            break;
        }
    }
    pthread_mutex_unlock(&gestor->mutex);

    free(cliente);
    return NULL;
}

void iniciar_servidor()
{
    int opt = 1;
    signal(SIGINT, manejar_senial);
    gestor = malloc(sizeof(GestorClientes));
    gestor->cantidad_clientes = 0;
    pthread_mutex_init(&gestor->mutex, NULL);

    gestor->servidor_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servidor_addr;

    servidor_addr.sin_family = AF_INET;
    servidor_addr.sin_addr.s_addr = INADDR_ANY;
    servidor_addr.sin_port = htons(PUERTO);

    setsockopt(gestor->servidor_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bind(gestor->servidor_socket, (struct sockaddr *)&servidor_addr, sizeof(servidor_addr));
    listen(gestor->servidor_socket, MAX_CLIENTES);
    printf("Servidor iniciado en puerto %d...\n", PUERTO);

    while(1)
    {
        ClienteInfo *cliente = malloc(sizeof(ClienteInfo));
        socklen_t cliente_len = sizeof(cliente->direccion);
        cliente->socket = accept(gestor->servidor_socket, (struct sockaddr *)&cliente->direccion, &cliente_len);

        pthread_mutex_lock(&gestor->mutex);

        // Esperar hasta que haya espacio
        while(gestor->cantidad_clientes >= MAX_CLIENTES)
            pthread_cond_wait(&espacio_disponible, &gestor->mutex);

        gestor->clientes[gestor->cantidad_clientes++] = cliente->socket;
        pthread_mutex_unlock(&gestor->mutex);

        pthread_t hilo;
        void *args = malloc(sizeof(struct { ClienteInfo *cliente; GestorClientes *gestor; }));
        memcpy(args, &(struct { ClienteInfo *cliente; GestorClientes *gestor; }){ cliente, gestor },
               sizeof(struct { ClienteInfo *cliente; GestorClientes *gestor; }));

        pthread_create(&hilo, NULL, atender_cliente, args);
        pthread_detach(hilo);
    }

    close(gestor->servidor_socket);
}

void cerrar_servidor()
{
    for(int i = 0; i < num_clientes; i++)
    {
        pthread_cancel(hilos[i]);
        close(clientes[i]);
    }
    close(gestor->servidor_socket);
    printf("Servidor finalizado.\n");
    pthread_mutex_destroy(&gestor->mutex);
    pthread_cond_destroy(&espacio_disponible);
    free(gestor);
}

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

