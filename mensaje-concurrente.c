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
#include <semaphore.h> 

#include <stdio.h>
/* shm_* stuff, and mmap() */
#include <sys/mman.h>
#include <sys/types.h>
/* exit() etc */
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
/* for random() stuff */
#include <stdlib.h>
#include <time.h>

#define SHMOBJ_PATH         "/the-fool-satn"
#define SEM_NAME            "semaforo-satn"

void *banner(void *param);
void *receiveMessage(void *param);
void *showMessagesCounter(void *param);

/* message structure for messages in the shared segment */
typedef struct {
    int amountReceived;
    int amountSent;
} msg_counter;
int sharedMemoryFd;
int shared_seg_size = (1 * sizeof(msg_counter));

int amountReceived;
int amountSent;

msg_counter *messageCounter;
sem_t *semaphore;

/* Programa principal */
int main(int argc, char *argv[]) {  

   if (argc<3)
   {   perror("uso: send-mq <nombre-cola-send> <nombre-cola-receive>\n"); exit(1); }

   if((semaphore=sem_open(SEM_NAME, O_CREAT, 0644, 1))==(sem_t *)-1)
   {   perror("No se puede crear el semáforo"); exit(1); }
    amountReceived = 0;
    amountSent = 0;

    sharedMemoryFd = shm_open(SHMOBJ_PATH, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG);
    if (sharedMemoryFd < 0) {
        perror("In shm_open()");
        exit(1);
    }
    ftruncate(sharedMemoryFd, shared_seg_size);

    messageCounter = (msg_counter *)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemoryFd, 0);
    if (messageCounter == NULL) {
        perror("In mmap()");
        exit(1);
    }
    //messageCounter->amountReceived = 0;
    //messageCounter->amountSent = 0;

    mqd_t queueSend;
    if ((queueSend = mq_open (argv[1],  O_RDWR )) == -1) 
    { perror("No se puede acceder a la cola de mensajes"); exit(1); }

    //Prepara pantalla
    initscr(); // Inicializar Ncurses

    clear(); // Limpiar la pantalla
    refresh();

    //Crea una ventana donde ingresar el texto
    WINDOW *windowSendMessage;
    windowSendMessage=newwin(12,80,12,0);
    box(windowSendMessage,0,0);

    mvwprintw(windowSendMessage, 1, 2, "Ingrese texto:"); 
    wrefresh(windowSendMessage);

    pthread_t t_receiveMessage;
    pthread_create(&t_receiveMessage,NULL,(void *)receiveMessage,(void *)argv[2]); //dispara receiveMessage
    pthread_t t_showMessagesCounter;
    pthread_create(&t_showMessagesCounter,NULL,(void *)showMessagesCounter,NULL); //dispara receiveMessage

    char text[200]="";
    //lee texto en la ventana, hasta el FIN
    while (strcmp(text,"FIN")!=0) {
        wgetstr(windowSendMessage, (char*)text);
        if (mq_send(queueSend, text, strlen (text),0) == -1) {
            perror("Error al enviar el mensaje");
            exit(1); 
        }
        sem_wait(semaphore);
        messageCounter->amountSent++;
        sem_post(semaphore);
    }

    pthread_cancel(t_receiveMessage);
    pthread_cancel(t_showMessagesCounter);

    werase(windowSendMessage);
    endwin(); // Finalizar Ncurses y termina


    sem_close(semaphore); 
    sem_unlink(SEM_NAME);

    if (shm_unlink(SHMOBJ_PATH) != 0) {
        perror("In shm_unlink()");
        exit(1);
    }

    
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
        sem_wait(semaphore);
        messageCounter->amountReceived++;
        sem_post(semaphore);
        row++;
        if (row >= 11) {
            row = 1;
            werase(windowReceiveMessage);
            box(windowReceiveMessage, 0, 0);
        }
        
    }
}


void *showMessagesCounter(void *param) {  
    while (1){
      //sem_wait(semaphore);
      mvprintw(1, 50, "Mensajes recibidos: %d", messageCounter->amountReceived);
      mvprintw(2, 50, "Mensajes enviados: %d", messageCounter->amountSent);
      sleep(1); // espera un segundo
      refresh(); // refresca pantalla
      //sem_post(semaphore);
    }  
    pthread_exit(0);   
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