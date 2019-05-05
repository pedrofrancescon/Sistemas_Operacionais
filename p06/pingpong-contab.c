// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DAINF UTFPR
// Versão 1.0 -- Março de 2015
//
// Teste da contabilização - tarefas de mesma prioridade

#include <stdio.h>
#include <stdlib.h>
#include "pingpong.h"

task_t Pang, Peng, Ping, Pong, Pung ;

void Body (void * arg)
{
   int i,j ;

   printf ("%s INICIO em %4d ms\n", (char *) arg, systime()) ;

   for (i=0; i<40000; i++)
      for (j=0; j<40000; j++) ;

   printf ("%s FIM    em %4d ms\n", (char *) arg, systime()) ;
   task_exit (0) ;
}

int main (int argc, char *argv[])
{
   printf ("Main INICIO\n");

   pingpong_init () ;

   _task_create (&Pang, Body, "    Pang") ;
   _task_create (&Peng, Body, "        Peng") ;
   _task_create (&Ping, Body, "            Ping") ;
   _task_create (&Pong, Body, "                Pong") ;
   _task_create (&Pung, Body, "                    Pung") ;

   task_yield () ;

   printf ("Main FIM\n");
   exit (0);
}
