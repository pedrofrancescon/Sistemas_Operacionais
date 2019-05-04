#include "pingpong.h"

task_t *tarefa_atual; //variavel global da tarefa corrente
task_t *fila_tarefas; //fila de fila_tarefas
task_t *fila_tarefas_suspensas; //fila de tarefas suspenas (ainda ñ usado aqui)
int id_tarefa; //incrementa id para proxima tarefa
task_t tarefa_principal; // tarefa main, não pode ser ponteiro(?)
task_t dispatcher; //tarefa despachante

task_t* scheduler(){
    return fila_tarefas; //FCFS, retorna primeira tarefa da fila
}
void dispatcher_body (){
    task_t *prox; //próxima tarefa, escolhida pelo despachante
    while (fila_tarefas > 0){
        prox = scheduler();
        if (prox){
            task_switch(prox);
        }
    }
    task_exit(0); //devolve processador para Main
}
void pingpong_init ()
{
    setvbuf (stdout, 0, _IONBF, 0); // Para desativar o buffer do printf.
    id_tarefa = 1; //atribuido a novas tarefas id com valores maior que zero
    tarefa_principal.id=0; //tarefa main deve ter id zero
    tarefa_atual=&tarefa_principal; //primeira tarefa deve ser a principal
    task_create(&dispatcher, (void *)dispatcher_body, NULL); //cria tarefa dispatcher
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
    if (task->id != 1) //dispatcher não pode entrar na fila de tarefas
        queue_append ((queue_t**) &fila_tarefas, (queue_t*) task); //adiciona na fila
    return task->id;
}

void task_exit (int exitCode) {
    if (tarefa_atual != &dispatcher){
        queue_remove((queue_t**) &fila_tarefas, (queue_t*) tarefa_atual);
        task_switch(&dispatcher);
        return;
    }
    task_switch(&tarefa_principal); //devolve o processador para tarefa main
}

int task_switch (task_t *task) //ttroca o uso do processador entre tarefas
{
    if(task==NULL) return -1; //retorna erro se a tarefa é invalida
    task_t *tarefa_anterior = tarefa_atual;
    tarefa_atual = task;
    swapcontext(&(tarefa_anterior->task_context), &(tarefa_atual->task_context));
    return 0;
}

int task_id ()
{
    return tarefa_atual->id;
}

void task_yield (){
    if (tarefa_atual != &tarefa_principal){
        queue_remove((queue_t**) &fila_tarefas, (queue_t*) tarefa_atual);
        queue_append((queue_t**) &fila_tarefas, (queue_t*) tarefa_atual);
    }
    task_switch(&dispatcher);
}
void task_resume (task_t *task){
    queue_remove((queue_t**) &fila_tarefas_suspensas, (queue_t*) task);
    queue_append((queue_t**) &fila_tarefas, (queue_t*) task);
    task->status = 'P'; //tarefa pronta
}
void task_suspend (task_t *task, task_t **queue){
    if (task != NULL){
        queue_append((queue_t **) queue, (queue_t *) task);
        //muda o status
    }
    else{
        task = tarefa_atual;
        queue_append((queue_t **) queue, (queue_t *) task);
    }
    task->status = 'S'; //tarefa suspensa
    if (queue != NULL){
        queue_remove((queue_t **) fila_tarefas, (queue_t *) task);
    }
}
