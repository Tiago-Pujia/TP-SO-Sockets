#include "cliente.h"
void pedir_ip();
void pedir_puerto();
char ip_servidor[INET_ADDRSTRLEN];
int puerto;
int main()
{
    pedir_ip();
    pedir_puerto();

    printf("Iniciando servidor en %s:%d...\n", ip_servidor, puerto);
    iniciar_cliente(ip_servidor, puerto);

    return 0;
}
// Función para pedir IP
void pedir_ip()
{
    while (1)
    {
        printf("Ingrese la IP: ");
        fgets(ip_servidor, sizeof(ip_servidor), stdin);
        ip_servidor[strcspn(ip_servidor, "\n")] = 0; // quita salto de línea

        struct sockaddr_in sa;
        int res = inet_pton(AF_INET, ip_servidor, &(sa.sin_addr));
        if (res == 1)
            break;
        else
            printf("IP inválida, intente de nuevo.\n");
    }
}

// Función para pedir puerto
void pedir_puerto()
{
    char buffer[10];
    int p = 0;
    while (1)
    {
        printf("Ingrese el puerto (1024-65535): ");
        fgets(buffer, sizeof(buffer), stdin);
        p = atoi(buffer);
        if (p >= 1024 && p <= 65535)
        {
            puerto = p;
            break;
        }
        else
            printf("Puerto inválido, intente de nuevo.\n");
    }
}
