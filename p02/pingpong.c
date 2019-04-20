#include "pingpong.h"

task_t *tarefa_atual; //variavel global da tarefa cadeado

void pingpong_init () 
{
	// Para desativar o buffer do printf.
	setvbuf (stdout, 0, _IONBF, 0);
}

// gerência de tarefas =========================================================

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create (task_t *task,			// descritor da nova tarefa
                 void (*start_func)(void *),	// funcao corpo da tarefa
                 void *arg) ;			// argumentos para a tarefa

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exitCode) ;

// alterna a execução para a tarefa indicada
int task_switch (task_t *task) 
{
    task_t *tarefa_anterior = tarefa_atual;
    tarefa_atual = task;
    swapcontext(tarefa_atual->task_context, task->task_context);
}


// retorna o identificador da tarefa corrente (main eh 0)
int task_id ()
{
    return tarefa_atual->id;
}