// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DAINF UTFPR
// Versão 1.0 -- Março de 2015
//
// Estruturas de dados internas do sistema operacional

#define STACKSIZE 32768        /* tamanho de pilha das threads */
#define _XOPEN_SOURCE 600    // para evitar erros POSIX no MacOS X

#ifndef __DATATYPES__
#define __DATATYPES__

#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>

// Estrutura que define uma tarefa
typedef struct task_t
{
	struct task_t *prev, *next; // para usar com a biblioteca de filas (cast)
	int id; // ID da tarefa
	short prioridade_estatica;
	short prioridade_dinamica;
	short quantum;
	char status; //estado da tarefa
	char type;
    unsigned int init_exec_time;
    unsigned int final_exec_time;
    unsigned int process_time_count; //tempo que tarefa 'ficou' com processador
    int activations;
	ucontext_t task_context; //contexto da tarefa, não pode ser ponteiro (??)

} task_t;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t;

#endif
