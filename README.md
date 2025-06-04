# Area del Cliente

## 📋 Descripción general

Este programa actúa como cliente TCP/IP, conectándose a un servidor en localhost:8080. Una vez conectado:

-   Lanza 2 hilos
    -   Uno que escucha mensajes del servidor
    -   Otro que envía datos aleatorios de temperatura o humedad cada 3 segundos
-   El usuario puede finalizar el cliente escribiendo "salir" por consola

El diseño se basa en la comunicación concurrente con sockets y threads, cumpliendo con los requisitos de la consigna para sistemas operativos.

## 🗂️ Archivos del Proyecto

```
cliente/
├── cliente.c       # Implementación del cliente
├── cliente.h       # Prototipos y constantes
├── main.c          # Punto de entrada principal
```

## 🧰 Compilación

```
gcc -o cliente main.c cliente.c -pthread
```

## 🔄 Flujo de Vida del Cliente

### 🟢 1. Inicio del programa

```C
int main() {
    iniciar_cliente();
    return 0;
}
```

### 🔌 2. Establecimiento de conexión

Dentro de `iniciar_cliente()`:

-   Se crea un socket TCP con socket(AF_INET, SOCK_STREAM, 0).
-   Se configura la dirección del servidor (127.0.0.1:8080).
-   Se realiza la conexión con connect(...).

> 📌 Si la conexión falla, el programa finaliza con un mensaje de error.

### 🧵 3. Creación de hilos

Una vez conectado:

-   Se crean 2 hilos con pthread_create:
    -   escuchar_servidor: espera mensajes del servidor.
    -   enviar_datos: envía datos al servidor cada 3 segundos.

Esto permite comunicación bidireccional en paralelo.

### 📤 3.1. Hilo 1: Envío de datos (enviar_datos)

-   En un bucle, el hilo:
    -   Genera al azar una medición:
        -   `"TEMPERATURA X"` → X entre -10 y 50
        -   `"HUMEDAD Y"` → Y entre 0 y 100
    -   La envía al servidor usando send(...).
    -   Espera 3 segundos ( sleep(3) ).
-   Se repite hasta que la variable global seguir_enviando se vuelva 0.

### 📥 3.2. Hilo 2: Recepción de datos (escuchar_servidor)

-   Escucha constantemente mensajes del servidor con recv(...).
-   Cada mensaje recibido se imprime por consola como:

```
Servidor: <mensaje>
```

> Si el servidor cierra la conexión o hay un error → el hilo termina.

### 🧑‍💻 6. Interacción del usuario

Mientras ambos hilos están activos:
- El proceso principal espera input del usuario con fgets().
- Si el usuario escribe "salir", se interrumpe el envío de datos:
  - Se cambia la variable global seguir_enviando = 0.
  - El hilo de envío finaliza en su siguiente iteración.

### 🛑 7. Finalización ordenada

Una vez que el usuario escribió "salir":
- Se llama a pthread_join(emisor, NULL) para esperar que el hilo emisor termine.
- Se cierra el socket con close(sockfd).
- Se imprime "Cliente finalizado.".

📝 El hilo receptor termina por sí solo cuando el servidor cierra o corta la conexión.

### Diagrama

~~~
               +------------------------+
               |      main()           |
               +------------------------+
                         |
                         v
               +------------------------+
               |  iniciar_cliente()     |
               | - conectar al servidor |
               | - crear socket         |
               +------------------------+
                         |
           +-------------+-------------+
           |                           |
           v                           v
+-------------------+      +-------------------------+
| escuchar_servidor |      |     enviar_datos        |
| (hilo receptor)    |      | (hilo emisor)           |
+-------------------+      +-------------------------+
| espera mensajes   |      | genera mensaje aleatorio|
| del servidor      |      | lo envía cada 3 segundos|
+-------------------+      +-------------------------+
           ^                           |
           |                           v
         (servidor envía)     (usuario escribe "salir")
                         +------------------+
                         | cerrar socket    |
                         | terminar cliente |
                         +------------------+
~~~

# Area del Servidor

## 📋 Descripción general

Este servidor TCP permite gestionar hasta 3 clientes concurrentes que envían datos simulados de temperatura o humedad. Los mensajes se procesan en hilos independientes y, si se detecta un valor fuera de rango, se dispara una alerta a todos los clientes conectados.

La arquitectura emplea sockets TCP, hilos POSIX y mecanismos de sincronización (mutex, cond) para gestionar la concurrencia de forma segura.

## 🗂️ Archivos del Proyecto

~~~
servidor/
├── main.c           # Punto de entrada
├── servidor.c       # Lógica del servidor
├── servidor.h       # Prototipos, estructuras y constantes
~~~

## 🧰 Compilación

~~~
gcc -o servidor main.c servidor.c -pthread
~~~

## 🔄 Flujo de Vida del Cliente

### 🟢 1. Inicio del servidor

- El programa comienza en main() y llama a iniciar_servidor().
- Se configura el socket TCP en el puerto 8080.
- Se inicializa la estructura GestorClientes, incluyendo:
    - El mutex para sincronización.
    - El arreglo de sockets de clientes.
    - El descriptor del socket servidor.
- Se establece el handler de señal para SIGINT (Ctrl+C).

### 🛑 2. Espera de conexiones

- El servidor entra en un bucle infinito while(1) donde:
  - Espera nuevos clientes con accept().
  - Si el número de clientes ≥ MAX_CLIENTES:
    - El hilo principal espera con pthread_cond_wait hasta que se libere espacio.

### 👥 3. Aceptación de un nuevo cliente

- Al aceptarse un nuevo cliente:
  - Se guarda su socket en el arreglo del GestorClientes.
  - Se crea un nuevo hilo con pthread_create.
  - Ese hilo ejecuta atender_cliente() para manejar exclusivamente a ese cliente.

### 🧠 4. Atención por hilo

En la función `atender_cliente()`:

1. Se recibe un mensaje desde el cliente (ej. "TEMPERATURA 45").
2. Se analiza el valor recibido:
    - Si fuera de rango, se construye un mensaje de alerta y se envía a todos los clientes con `enviar_alerta_todos()`.
    - Si está en rango, se responde "OK" al cliente.
3. Este ciclo se repite hasta que el cliente se desconecta.

### ❌ 5. Desconexión de un cliente

Cuando un cliente se desconecta:

- Se elimina su socket del arreglo de clientes.
- Se decrementa el contador.
- Se señala con `pthread_cond_signal` que hay espacio disponible.
- Si no queda ningún cliente, se imprime un mensaje y se cierra el servidor automáticamente.

### 🔚 6. Finalización del servidor

El servidor puede finalizar de dos formas:
1. Automáticamente cuando no quedan clientes conectados.
2. Manual, si el usuario presiona Ctrl+C (capturado con SIGINT).

En ambos casos:
- Se cancelan los hilos restantes.
- Se cierran todos los sockets.
- Se destruyen el mutex y la condición.
- Se libera memoria.

### Diagrama
~~~
                   ┌──────────────┐
                   │  main()      │
                   └────┬─────────┘
                        │
                        ▼
             ┌────────────────────┐
             │ iniciar_servidor() │
             └────┬───────────────┘
                  │
       ┌──────────▼────────────┐
       │ aceptar cliente       │
       └────────┬──────────────┘
                ▼
       ¿hay lugar disponible?
           │         │
          Sí         No
           │          ▼
           ▼   esperar con cond_wait
  ┌──────────────────────────────┐
  │ agregar cliente y lanzar hilo│
  └────────────┬─────────────────┘
               ▼
        atender_cliente() en hilo
               │
               ▼
    recibe datos y evalúa alertas
               │
               ▼
    envía OK o ALERTA a todos
               │
               ▼
   cliente se desconecta → liberar
               │
               ▼
    ¿quedan clientes? → No → cerrar
~~~