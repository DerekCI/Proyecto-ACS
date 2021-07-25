// Importa librerías necesarias
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>


// Constante con el número de puerto por el cuál va escuchar peticiones el servidor
#define MYPORT 3490


// Tamaño de la cola para conexiones pendientes
#define BACKLOG 10

// Constante con el número máximo de bytes que podemos envíar
#define MAXDATASIZE 300

// Función de captura de señal
void sigchld_handler(int s){
    while(wait(NULL) > 0);
}


int main(int argc, char *argv[ ]){


    // Declaración de variables y estructuras necesarias
    int sockfd, new_fd, numbytes;
    char buf[MAXDATASIZE];


    // Información de direcciones del servidor
    struct sockaddr_in my_addr;


    // Información de direcciones de conectores (puerto)
    struct sockaddr_in their_addr;
    int sin_size;
    struct sigaction sa;
    int yes = 1;


    // Crea un socket 
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {

        // Si no se puede crear, muestra el error y termina el programa
        perror("Server-socket() error lol!");
        exit(1);
    }else {

        // Si se crea, muestra un mensaje de éxito
        printf("Server-socket() sockfd is OK...\n");
    }


    /* 
        setsocketopt() - Establece las opciones asociadas al socket sockfd:

        Establece la opción SO_REUSEADDR en el nivel de protocolo SOL_SOCKET, al valor 
        apuntado por 'yes' para el socket asociado con el descriptor de archivo 'sockfd'.

        - SO_REUSEADDR: Especifica que las reglas utilizadas en la validación de direcciones
        proporcionadas a bind() deben permitir la reutilización de direcciones locales, si el 
        protocolo lo admite. Esta opción toma un valor int. Esta es una opción booleana. 
        
        SO_REUSEADDR se configura comúnmente en programas de servidor de red, ya que un 
        patrón de uso común es realizar un cambio de configuración, luego se requiere 
        reiniciar ese programa para que el cambio surta efecto. Sin SO_REUSEADDR, la llamada 
        bind() en la nueva instancia del programa reiniciado fallará si había conexiones 
        abiertas a la instancia anterior cuando la eliminó. Esas conexiones mantendrán el puerto 
        TCP en el estado TIME_WAIT durante 30-120 segundos.
        
        - SOL_SOCKET es la propia capa de conexión. Se utiliza para opciones que son independientes
        del protocolo.
    */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        
        // Si ocurre un error, lo comunica
        perror("Server-setsockopt() error lol!");
        exit(1);
    }else {
        
        // Si no ocurre un error, emite un mensaje de éxito
        printf("Server-setsockopt is OK...\n");
    }


    // Orden de bytes del host (4 bytes para direcciones IPv4 -> AF_INET)
    my_addr.sin_family = AF_INET;


    /* Almacena en la estructura de tipo sockaddr_in llamada my_addr, el puerto de comunicación 
    y la dirección IP en el campo s_addr de la estructura interna sin_addr */

    /*
        INADDR_ANY: en realidad es la IP especial 0.0.0.0 que representa 
        "cualesquiera que sean las IPs de todas las interfaces de red de este ordenador".

        Al usar INADDR_ANY, por tanto, se consiguen dos cosas:

        1. Admitir datos que provengan de cualquiera de las interfaces de red que el servidor 
        tenga.
        2. Aunque el servidor cambie su IP, el programa seguirá funcionando sin cambios.
    */
    my_addr.sin_port = htons(MYPORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    printf("Server-Using %s and port %d...\n", inet_ntoa(my_addr.sin_addr), MYPORT);


    // Establece en 0 el campo sin_zero de la estructura sockaddr_in
    memset(&(my_addr.sin_zero), '\0', 8);


    // Intenta enlazar el servidor al socket 'sockfd' para escuchar peticiones por ahí
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {

        // Si el enlace falla, muestra el error y termina el programa 
        perror("Server-bind() error");
        exit(1);
    } else {

        // Si el enlace no falla, muestra un mensaje de éxito
        printf("Server-bind() is OK...\n");
    }
        

    // Escucha las conexiones de socket y limita la cola de conexiones entrantes
    if (listen(sockfd, BACKLOG) == -1) {
        perror("Server-listen() error");
        exit(1);
    }


    printf("Server-listen() is OK...Listening...\n");


    // Limpia todos los procesos muertos
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;


    /* 
        sigaction - Examina y cambia una acción de señal

        int sigaction(int sig, const struct sigaction *restrict act, struct sigaction *restrict oact);

        La función sigaction() permite que el proceso de llamada examine y/o especifique la 
        acción que se asociará con una señal específica. El argumento sig especifica la señal; 
        los valores aceptables se definen en <signal.h>.

        La estructura sigaction, se utiliza para describir una acción a realizar.

        - SIGCHLD es la señal enviada a un proceso cuando uno de sus procesos hijos termina.
        - &sa es la estructura de tipo sigaction que especifica la acción a realizar.
    */
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {

        // Si falla muestra el error y termina el programa
        perror("Server-sigaction() error");
        exit(1);
    } else {

        // Si no falla muestra un mensaje de éxito
        printf("Server-sigaction() is OK...\n");
    }


    // Ciclo infinito para aceptar peticiones
    while(1) {


        // Almacena el tamaño de una estructura tipo sockaddr_in
        sin_size = sizeof(struct sockaddr_in);

        
        // Acepta una nueva conexión en un socket
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
            
            //Si ocurre un error, muestra un mensaje y el programa continua
            perror("Server-accept() error");
            continue;
        }else {

            // Si no ocurre error, se muestran un par de mensajes de éxito
            printf("Server-accept() is OK...\n");
            printf("Server-new socket, new_fd is OK...\n");
            printf("Server: Got connection from %s\n", inet_ntoa(their_addr.sin_addr));
        }


        // Se crea un proceso hijo para que atienda al cliente
        if(!fork()){
            // Aquí entra el proceso hijo únicamente

            // El proceso hijo cierra el socket 'sockfd' porque no lo necesita
            close(sockfd);

            // Recibe un mensaje del cliente
            if((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {

                // Si ocurre un error, lo reporta y el programa continua
                perror("Server-recv() error lol!");
            } else {

                // Si se recibe el mensaje sin problema, muestra el mensaje del cliente
                printf("Server-recv() is OK...\n");
                buf[numbytes] = '\0';
                printf("Server-Received: %s", buf);  

                int i=0;
                
                char *tokens = strtok(buf, " ");
                char *palabras[2];
                while(tokens != NULL ){
                    palabras[i++] = tokens;
                    tokens = strtok(NULL, " ");
                }
                strcat(palabras[1],".txt");
                printf("%s", palabras[1]);
                //trata de abrir el archivo con el numero de cuenta
                FILE *archivo;
                archivo = fopen(palabras[1], "r");
                if (archivo == NULL) printf("Error al abrir el fichero");



                

                /*palabras = strcat(palabras,".txt");
                printf("%s", palabras);
                FILE *fp;
                fp = fopen( palabras, "r");
                if (fp==NULL){fputs("File error", stderr); exit(1);}
                fclose(fp);
                */
            }


            // Envía un mensaje en un socket
            if(send(new_fd, "This is a test string from server!\n", 37, 0) == -1) {

                // Si ocurre un error local, lo reporta y el programa continua
                perror("Server-send() error lol!");
            }

            // El proceso hijo ierra el socket de comunicación con el cliente y termina su ejecución
            close(new_fd);
            exit(0);
        }else {

            // El proceso padre muestra un mensaje de éxito
            printf("Server-send is OK...!\n");
        }

        // El proceso padre cierra el socket de comunicación con el cliente, no lo necesita
        close(new_fd);
        printf("Server-new socket, new_fd closed successfully...\n");
    }

    // Termina el servidor
    return 0;
}