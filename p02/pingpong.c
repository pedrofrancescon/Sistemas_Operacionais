#include "pingpong.h"

task_t *tarefa_atual; //variavel global da tarefa corrente
int id_tarefa; //incrementa id para proxima tarefa
task_t tarefa_principal; // tarefa main, não pode ser ponteiro(?)

void pingpong_init ()
{
    // Para desativar o buffer do printf.
    setvbuf (stdout, 0, _IONBF, 0);
    id_tarefa = 1; //atribuido a novas tarefas id com valores maior que zero
    tarefa_principal.id=0; //tarefa main deve ter id zero
    tarefa_atual=&tarefa_principal; //primeira tarefa deve ser a principal
}

int task_create (task_t *task, void (*start_func)(void *), void *arg){
    // Inicia contexto atual em "tarefa_atual"
    if(task==NULL) return -1; //retorna erro se a tarefa é invalida
    getcontext(&(task->task_context));
    task->id = id_tarefa; //define um id unico para a nova tarefa
    id_tarefa++; //incrementa proximo id

    //inicializa pilha do contexto
    char *stack;
    stack = malloc (STACKSIZE);

    if (stack) //verifica criação da pilha e define atributos
    {
        task->task_context.uc_stack.ss_sp = stack ;
        task->task_context.uc_stack.ss_size = STACKSIZE;
        task->task_context.uc_stack.ss_flags = 0;
        task->task_context.uc_link = 0;
    }
    else
    {
        perror ("Erro na criação da pilha: ");
        exit (1);
    }
    //cria um novo contexto a partir da tarefa recebida
    makecontext(&(task->task_context), (void *)start_func, 1, arg);
    return task->id;
}

void task_exit (int exitCode) {
    task_switch(&tarefa_principal); //devolve o processador para tarefa main
}

int task_switch (task_t *task) //ttroca o uso do processador entre tarefas
{
    if(task==NULL) return -1; //retorna erro se a tarefa é invalida
    task_t *tarefa_anterior = tarefa_atual;
    tarefa_atual = task;
    tarefa_atual->prev = tarefa_anterior;
    swapcontext(&(tarefa_anterior->task_context), &(tarefa_atual->task_context));
}

int task_id ()
{
    return tarefa_atual->id;
}
