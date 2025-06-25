#include "cliente.h"
#define MAX_CLIENTES 3
void *escuchar_servidor(void *arg)
{
    int sockfd = *(int*)arg;
    char buffer[BUFFER_SIZE];

    while(1)
    {
        int bytes = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if(bytes <= 0) break;
        buffer[bytes] = '\0';
        printf("Servidor: %s", buffer);
    }

    return NULL;
}

int seguir_enviando = 1;

void* enviar_datos(void* arg)
{
    int sockfd = *(int*)arg;
    char buffer[BUFFER_SIZE];
    srand(time(NULL));

    while(seguir_enviando)
    {
        int tipo = rand() % 2;
        int valor = tipo ? (rand() % 101) : ((rand() % 61) - 10);

        if(tipo)
            snprintf(buffer, BUFFER_SIZE, "HUMEDAD %d\n", valor);
        else
            snprintf(buffer, BUFFER_SIZE, "TEMPERATURA %d\n", valor);

        send(sockfd, buffer, strlen(buffer), 0);
        sleep(3);
    }

    return NULL;
}

void iniciar_cliente(const char* ip, int puerto)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(puerto);
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
    {
        perror("IP inválida");
        exit(1);
    }

    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error al conectar");
        exit(1);
    }

    pthread_t escucha, emisor;
    pthread_create(&escucha, NULL, escuchar_servidor, &sockfd);
    pthread_create(&emisor, NULL, enviar_datos, &sockfd);

    char buffer[BUFFER_SIZE];
    while(1)
    {
        printf("Escriba 'salir' para terminar: \n");
        fgets(buffer, BUFFER_SIZE, stdin);
        if(strncmp(buffer, "salir", 5) == 0)
        {
            seguir_enviando = 0;
            break;
        }
    }

    pthread_join(emisor, NULL);
    close(sockfd);
    printf("Cliente finalizado.\n");
}

