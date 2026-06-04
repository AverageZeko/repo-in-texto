// mutex.c 
// Ejemplo básico de uso de semáforos
// 
// Simula una zona crítica protegida por el semáforo nombrado 
// pasado como primer parámetro externo
//
// El tránsito en la zona critica se simula con un sleep(s)
// La cantidad de segundos s es recibida como segundo parámetro externo
//
// Para verlo en funcionamiento ejecute este programa desde dos terminales
// en forma concurrente.
//
// Autor: CB - UNPSJB 
// 
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h> 
#include <pthread.h> 
#include <string.h>
#include <fcntl.h>

int main (int argc, char *argv[ ])
{
   sem_t *semaforo;
   int seg;

   // Controla parámetros de entrada
   if (argc<3)
   {   perror("uso: mutex <semaforo> <segs zona critica>\n"); exit(1); }
   seg = atoi(argv[2]);
  
   /* Crea el semáforo nombrado */
   if((semaforo=sem_open(argv[1], O_CREAT, 0644, 1))==(sem_t *)-1)
   {   perror("No se puede crear el semáforo"); exit(1); }
   
   printf("Espera por entrar a la zona crítica\n");
   sem_wait(semaforo);
   printf("ENTRA a la zona crítica (demora %d segundos)\n",seg);
     sleep(seg);
   printf("SALE de la zona crítica\n");
   sem_post(semaforo);
   printf("Cierra semaforos y termina\n");
   sem_close(semaforo); 
   exit(0);
}

