#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pingpong.h"
#include "queue.h"

queue_t *buffer;
task_t p1, p2, p3, c1, c2 ;
semaphore_t s_buffer , s_item , s_vaga ;

void produtor (void * arg) {
  while(1) {
    task_sleep (1) ;
    queue_t *item;
    item->value = rand() % 100;

    sem_down(&s_vaga);

    sem_down(&s_buffer);
    if (queue_size(buffer) < 5)
      queue_append(&buffer, item);
    sem_up(&s_buffer);

    sem_up(&s_item);

    printf("%s produziu %d\n", (char *) arg, item->value);
  }
}

void consumidor (void * arg) {
  while(1) {
    sem_down(&s_item);

    sem_down(&s_buffer);
    queue_t *item = buffer;
    queue_remove(&buffer,item);
    sem_up(&s_buffer);

    sem_up(&s_vaga);

    printf("%s consumiu %d\n", (char *) arg, item->value);

    task_sleep (1) ;
  }
}

int main (int argc, char *argv[])
{
   printf ("Main INICIO\n") ;

   pingpong_init() ;

   srand(time(NULL));

   sem_create (&s_buffer, 0) ;
   sem_create (&s_item, 0) ;
   sem_create (&s_vaga, 0) ;

   task_create (&p1, produtor, "p1") ;
   task_create (&p2, produtor, "p2") ;
   task_create (&p3, produtor, "p3") ;

   task_create (&c1, consumidor, "p1") ;
   task_create (&c2, consumidor, "p2") ;

   task_join (&p1) ;
   task_join (&p2) ;
   task_join (&p3) ;

   task_join (&c1) ;
   task_join (&c2) ;

   printf ("Main FIM\n") ;
   task_exit (0) ;

   exit (0) ;
}
