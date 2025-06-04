#include "cliente.h"
#define MAX_CLIENTES 3

/**
 * Hilo que escucha y muestra los mensajes recibidos desde el servidor.
 *
 * @param arg Puntero al socket conectado al servidor.
 */
void *escuchar_servidor(void *arg)
{
    int sockfd = *(int*)arg;                     // Obtener el descriptor de socket
    char buffer[BUFFER_SIZE];                    // Buffer para recibir mensajes

    while(1)
    {
        int bytes = recv(sockfd, buffer, BUFFER_SIZE - 1, 0); // Espera mensaje del servidor
        // Si se pierde la conexión o se cierra, salir
        if(bytes <= 0)
        {
            break;
        }

        buffer[bytes] = '\0';                    // Agregar terminador de cadena
        printf("Servidor: %s", buffer);          // Imprimir mensaje recibido
    }

    return NULL;
}

int seguir_enviando = 1;  // Variable global de control para el hilo emisor

/**
 * Hilo que genera y envía mensajes de forma periódica al servidor.
 *
 * @param arg Puntero al socket conectado al servidor.
 */
void* enviar_datos(void* arg)
{
    int sockfd = *(int*)arg;                     // Obtener el descriptor del socket
    char buffer[BUFFER_SIZE];                    // Buffer para construir mensajes
    srand(time(NULL));                           // Inicializar semilla aleatoria

    while(seguir_enviando)
    {
        int tipo = rand() % 2;                   // Seleccionar aleatoriamente 0 (TEMP) o 1 (HUM)
        int valor = tipo ? (rand() % 101)        // Si HUMEDAD: 0 a 100
                         : ((rand() % 61) - 10); // Si TEMPERATURA: -10 a 50

        if(tipo)
            snprintf(buffer, BUFFER_SIZE, "HUMEDAD %d\n", valor);     // Generar mensaje de humedad
        else
            snprintf(buffer, BUFFER_SIZE, "TEMPERATURA %d\n", valor); // Generar mensaje de temperatura

        send(sockfd, buffer, strlen(buffer), 0); // Enviar mensaje al servidor
        sleep(3);                                // Esperar 3 segundos antes del siguiente envío
    }

    return NULL;
}

/**
 * Función principal del cliente. Establece conexión, lanza hilos y espera "salir".
 */
void iniciar_cliente()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);        // Crear socket TCP
    struct sockaddr_in serv_addr;                        // Estructura para la dirección del servidor

    serv_addr.sin_family = AF_INET;                      // Familia IPv4
    serv_addr.sin_port = htons(PUERTO);                  // Puerto (convertido a orden de red)
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);// Convertir IP de texto a binario

    // Intentar conectarse al servidor
    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error al conectar"); // Mostrar error si falla
        exit(1);
    }

    // Aviso de conexión exitosa
    printf("[Cliente] Conectado al servidor. "
        "Si el servidor está lleno, espere a que se libere un lugar...\n");
    fflush(stdout);

    pthread_t escucha, emisor; // Identificadores de los hilos

    // Crear hilo para escuchar mensajes del servidor
    pthread_create(&escucha, NULL, escuchar_servidor, &sockfd);

    // Crear hilo para enviar datos periódicamente
    pthread_create(&emisor, NULL, enviar_datos, &sockfd);

    char buffer[BUFFER_SIZE]; // Buffer para entrada del usuario

    // Bucle para esperar que el usuario escriba "salir"
    while(1)
    {
        printf("Escriba 'salir' para terminar: \n");
        fgets(buffer, BUFFER_SIZE, stdin);               // Leer desde stdin

        if(strncmp(buffer, "salir", 5) == 0)             // Si escribe "salir"
        {
            seguir_enviando = 0;                         // Señal para detener el hilo emisor
            break;
        }
    }

    pthread_join(emisor, NULL);                          // Esperar que finalice el hilo emisor

    close(sockfd);                                       // Cerrar la conexión con el servidor
    printf("Cliente finalizado.\n");
}

/*void iniciar_cliente()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PUERTO);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error al conectar");
        exit(1);
    }

    pthread_t escucha;
    pthread_create(&escucha, NULL, escuchar_servidor, &sockfd);

    srand(time(NULL));
    char buffer[BUFFER_SIZE];

    while(1 && !strncmp(buffer, "salir", 5))
    {
        int tipo = rand() % 2; // humedad o temparatura
        int valor = tipo ? (rand() % 101) : ((rand() % 61) - 10); //si es humedad (0-100) si es temp (-10-50)

        if(tipo)
            snprintf(buffer, BUFFER_SIZE, "HUMEDAD %d\n", valor);
        else
            snprintf(buffer, BUFFER_SIZE, "TEMPERATURA %d\n", valor);

        send(sockfd, buffer, strlen(buffer), 0);

        // Chequear si el usuario quiere salir (no automático)
        printf("Escriba 'salir' para terminar: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        fflush(stdin);
        sleep(3);
        //if(strncmp(buffer, "salir", 5) == 0) break;
    }

    close(sockfd);
    printf("Cliente finalizado.\n");
}*/
