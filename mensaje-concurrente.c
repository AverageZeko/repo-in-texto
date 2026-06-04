// Ejemplo de threads
// Dispara dos threads y queda leyendo texto del usuario
// El thread 1 mueve un banner en la esquina superior izquierda
// El thread 2 actualiza la hora en la esquina superior derecha
// El thread principal realiza lectura de teclado dentro de una ventana
//
// Autor: CB
#include <stdlib.h>
#include <stdio.h>  
#include <pthread.h>  
#include <ncurses.h>
#include <time.h>
#include <string.h>
#include <mqueue.h>

//Esta función mostrará el banner en un thread
void *banner(void *param);

void *receiveMessage(void *param);

/* Programa principal */
int main(int argc, char *argv[]) {  

   if (argc<3)
   {   perror("uso: send-mq <nombre-cola-send> <nombre-cola-receive>\n"); exit(1); }

    mqd_t queueSend;
    pthread_t t_receiveMessage;

    WINDOW *windowSendMessage;
    char text[200]="";

   if ((queueSend = mq_open (argv[1],  O_RDWR )) == -1) 
    { perror("No se puede acceder a la cola de mensajes"); exit(1); }

    //Prepara pantalla
    initscr(); // Inicializar Ncurses

    clear(); // Limpiar la pantalla
    refresh();

    //Crea una ventana donde ingresar el texto
    
    windowSendMessage=newwin(12,80,12,0);
    box(windowSendMessage,0,0);

    mvwprintw(windowSendMessage, 1, 2, "Ingrese texto:"); 
    wrefresh(windowSendMessage);

    pthread_create(&t_receiveMessage,NULL,(void *)receiveMessage,(void *)argv[2]); //dispara receiveMessage

    //lee texto en la ventana, hasta el FIN
    while (strcmp(text,"FIN")!=0) {
        wgetstr(windowSendMessage, (char*)text);
        if (mq_send(queueSend, text, strlen (text),0) == -1) {
            perror("Error al enviar el mensaje");
            exit(1); 
        }
    }

    pthread_cancel(t_receiveMessage);

    werase(windowSendMessage);
    endwin(); // Finalizar Ncurses y termina
    return 0;  
}

void *receiveMessage(void *param) {
    mqd_t queueReceive;
    const char *queueName = param;
    char buff[1024];

    WINDOW *windowReceiveMessage;
    windowReceiveMessage=newwin(12,80,0,0);
    box(windowReceiveMessage,0,0);
    wrefresh(windowReceiveMessage); 

    if ((queueReceive = mq_open (queueName,  O_RDWR)) == -1) {
        perror("No se puede acceder a la cola de mensajes");
        exit(1); 
    }


    int row = 1;
    while (1) {
        ssize_t n = mq_receive(queueReceive, buff, sizeof(buff) - 1, 0);
        if (n == -1) {
            perror("Error al recibir el mensaje");
            exit(1);
        }
        buff[n] = '\0';

        mvwprintw(windowReceiveMessage, row, 2, "%s", buff);
        wrefresh(windowReceiveMessage);

        row++;
        if (row >= 11) {
            row = 1;
            werase(windowReceiveMessage);
            box(windowReceiveMessage, 0, 0);
        }
        
    }
}




// Función que muestra el banner en la esquina superior izquierda
void *banner(void *param) {  
int i = 0;
char cartel[] = "Sistemas Operativos ";
char s1[100];

    // cada un segundo actualiza el banner
    while (1) {
        strncpy(s1, &cartel[i], sizeof(cartel)); //desplaza texto 1 caracter
        mvaddstr(0, 0,s1 ); //muestra el banner
        refresh(); // refresca pantalla      
        i=(i+1)%(sizeof(cartel)-1); //incrementa posición
        usleep(500000); // espera medio segundo
    }  
    pthread_exit(0);   
}