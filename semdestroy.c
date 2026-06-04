// semdestroy.c 
// Elimina un semáforo POSIX nombrado
// 
// Recibe como parámetro el nombre del semáforo a destruir
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

   // Controla parámetros de entrada
   if (argc==1)
   {   perror("uso: semdestroy <semaforo_nombrado>\n"); exit(1); }
  
   /* Crea el semáforo nombrado */
   if((semaforo=sem_open(argv[1], 0, 0644, 1))==(sem_t *)-1)
   {   printf("No se encuentra el semáforo %s",argv[1]); exit(1); }
   
   sem_unlink(argv[1]);
   printf("Semaforo %s eliminado\n",argv[1]);
   if (sem_close(semaforo) < 0)
   {   printf("error al cerrar el semáforo %s",argv[1]); exit(1); }
         
   exit(0);
}

