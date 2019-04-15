#define _XOPEN_SOURCE 600  /* para compilar no MacOS */
#define STACKSIZE 32768    /* tamanho de pilha das threads */

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

// operating system check
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif

ucontext_t ContextPing, ContextPong, ContextMain;

/*****************************************************/

void BodyPing (void * arg)
{
   int i ;

   printf ("%s iniciada\n", (char *) arg) ;
   
   for (i=0; i<4; i++)
   {
      printf ("%s %d\n", (char *) arg, i) ;
      // Troca o contexto p/ ContextPong (que irá para função BodyPong), após executar o print dessa função.
      swapcontext (&ContextPing, &ContextPong);
   }	
   printf ("%s FIM\n", (char *) arg) ;

   // Quando acaba, volta p/ contexto principal do programa (ContextMain).
   swapcontext (&ContextPing, &ContextMain) ;
}

/*****************************************************/

void BodyPong (void * arg)
{
   int i ;

   printf ("%s iniciada\n", (char *) arg) ;

   for (i=0; i<4; i++)
   {
      printf ("%s %d\n", (char *) arg, i) ;
      // Troca o contexto p/ ContextPing (que irá para função BodyPing), após executar o print dessa função.
      swapcontext (&ContextPong, &ContextPing);
   }
   printf ("%s FIM\n", (char *) arg) ;

   // Quando acaba, volta p/ contexto principal do programa (ContextMain).
   swapcontext (&ContextPong, &ContextMain) ;
}

/*****************************************************/

int main (int argc, char *argv[])
{
   char *stack ;

   printf ("Main INICIO\n");

   // Inicializa ContextPing com o contexto atual.
   getcontext (&ContextPing);

   // Aloca pilha p/ salvar informações sobre o contexto de "Ping".
   stack = malloc (STACKSIZE) ;
   // Se a pilha existir...
   if (stack)
   {
      // Inicializa struct ucontext_t que salva o contexto, com a respectiva pilha de memória e flags.
      ContextPing.uc_stack.ss_sp = stack ;
      ContextPing.uc_stack.ss_size = STACKSIZE;
      ContextPing.uc_stack.ss_flags = 0;
      ContextPing.uc_link = 0;
   }
   else
   {
      perror ("Erro na criação da pilha: ");
      exit (1);
   }

   // Atribui a função BodyPing ao contexto ContextPing, dando os parâmetros com os quais a função deve ser chamada.
   makecontext (&ContextPing, (void*)(*BodyPing), 1, "    Ping");

   printf ("MAIN: depois de ping\n");

   // Inicializa ContextPong com o contexto atual.
   getcontext (&ContextPong);

   // Aloca pilha p/ salvar informações sobre o contexto de "Pong".
   stack = malloc (STACKSIZE) ;
   // Se a pilha existir...
   if (stack)
   {
      // Inicializa struct ucontext_t que salva o contexto, com a respectiva pilha de memória e flags.
      ContextPong.uc_stack.ss_sp = stack ;
      ContextPong.uc_stack.ss_size = STACKSIZE;
      ContextPong.uc_stack.ss_flags = 0;
      ContextPong.uc_link = 0;
   }
   else
   {
      perror ("Erro na criação da pilha: ");
      exit (1);
   }

   // Atribui a função BodyPong ao contexto ContextPong, dando os parâmetros com os quais a função deve ser chamada.
   makecontext (&ContextPong, (void*)(*BodyPong), 1, "        Pong");

   // Salva o contexto atual em ContextMain e chama o contexto ContextPing (que dispara a função BodyPing atribuida anteriormente na chamada de "makecontext").
   swapcontext (&ContextMain, &ContextPing);
   // Chama o contexto de ContextPong uma última vez para terminar a execução do programa.
   swapcontext (&ContextMain, &ContextPong);

   printf ("Main FIM\n");

   exit (0);
}
