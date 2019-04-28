#include "pingpong.h"

task_t *tarefa_atual; //variavel global da tarefa corrente

void pingpong_init () 
{
    // Para desativar o buffer do printf.
    setvbuf (stdout, 0, _IONBF, 0);

    // Inicia contexto atual em "tarefa_atual"
    
    ucontext_t *context = tarefa_atual->task_context;

    getcontext(&context);

    char *stack;
    stack = malloc (STACKSIZE);

    if (stack)
    {
        context->uc_stack.ss_sp = stack ;
        context->uc_stack.ss_size = STACKSIZE;
        context->uc_stack.ss_flags = 0;
        context->uc_link = 0;
    }
    else
    {
        perror ("Erro na criação da pilha: ");
        exit (1);
    }
    
    
}

// gerência de tarefas =========================================================

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create (task_t *task,                  // descritor da nova tarefa
                 void (*start_func)(void *),    // funcao corpo da tarefa
                 void *arg) ;                   // argumentos para a tarefa

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exitCode) ;

// alterna a execução para a tarefa indicada
int task_switch (task_t *task) 
{
    task_t *tarefa_anterior = tarefa_atual;
    tarefa_atual = task;
    tarefa_atual->prev = tarefa_anterior;
    swapcontext(&(tarefa_anterior->task_context), &(tarefa_atual->task_context));
}


// retorna o identificador da tarefa corrente (main eh 0)
int task_id ()
{
    return tarefa_atual->id;
}