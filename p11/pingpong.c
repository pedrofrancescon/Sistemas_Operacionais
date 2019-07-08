#include "pingpong.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "queue.h" //biblioteca de filas

#define FCFS //descomentar para executar sem prioridade: pingpong-contab


task_t *tarefa_atual; //variavel global da tarefa corrente
task_t *fila_tarefas; //fila de fila_tarefas
task_t *fila_tarefas_suspensas; //fila de tarefas suspenas (ainda ñ usado aqui)
task_t *fila_tarefas_adormecidas;
int id_tarefa; //incrementa id para proxima tarefa
unsigned int program_clock; // contador de tempo transcorrido (em milisegundos)
task_t tarefa_principal; // tarefa main, não pode ser ponteiro(?)
task_t dispatcher; //tarefa despachante

struct sigaction acao;
struct itimerval tempo;

unsigned int systime ()
{
    return program_clock;
}
void acorda_tarefas(){
  //percorre fila de fila_tarefas_adormecidas
  task_t *tarefa = fila_tarefas_adormecidas;
  //Salva tamanho da fila
  short tam = queue_size((queue_t*)fila_tarefas_adormecidas);
  short i = 0;
  //percorre fila inteira
  while (i < tam){
    //se já tiver dado o tempo da soneca
    if (tarefa->sleep <= systime()){
      //remove da fila de adormecidas e coloca na fila de prontas
      queue_remove((queue_t **) &fila_tarefas_adormecidas, (queue_t *) tarefa);
      queue_append((queue_t **) &fila_tarefas, (queue_t*) tarefa);
    }
    //'anda' na fila
    tarefa = tarefa->next;
    i++;
  }
}
task_t* scheduler(){
    #ifdef FCFS
        return fila_tarefas;
    #endif
    #ifndef FCFS
        task_t *prioritario = fila_tarefas; //primeiro elemento é o mais prioritario
        task_t *candidato_prioritario = fila_tarefas->next; //segundo elemento é candidato

        //encontra a tarefa mais prioritaria
        while (candidato_prioritario != fila_tarefas){ //enquanto não percorreu toda fila
            //compara prioridades dinamicas
            if (candidato_prioritario->prioridade_dinamica <= prioritario->prioridade_dinamica)
                prioritario = candidato_prioritario; //caso seja 'maior', candidato vira o prioritario
            candidato_prioritario->prioridade_dinamica -= 1; //envelhecimento da tarefa
            candidato_prioritario = candidato_prioritario->next; //'anda' na fila
        }
        filaprogram_clock_tarefas->prioridade_dinamica -= 1; //envelhecimento do primeiro elemento
        prioritario->prioridade_dinamica = prioritario->prioridade_estatica; //retorno da prioridade original
        return prioritario;
    #endif
}
void dispatcher_body (){
    task_t *prox; //próxima tarefa, escolhida pelo despachante
    //Percorre fila de prontas e adormecidas
    while (fila_tarefas > 0 || fila_tarefas_adormecidas > 0){
        acorda_tarefas();
        prox = scheduler();
        if (prox){
            task_switch(prox);
        }
    }
    task_exit(0); //devolve processador para Main
}
void tique(){
  program_clock += 1;
  tarefa_atual->process_time_count += 1;
  if (task_id() != 1){ //tarefa de usuário ou main
    tarefa_atual->quantum -= 1;

    if (tarefa_atual->quantum == 0)
    task_yield();
  }
}
void pingpong_init ()
{
    setvbuf (stdout, 0, _IONBF, 0); // Para desativar o buffer do printf.
    id_tarefa = 1; //atribuido a novas tarefas id com valores maior que zero
    tarefa_principal.id = 0;
    tarefa_atual=&tarefa_principal; //primeira tarefa deve ser a principal
    program_clock = 0;
    tarefa_atual->quantum = 20;
    tarefa_atual->init_exec_time = systime();
    tarefa_atual->process_time_count = 0;
    tarefa_atual->activations = 0;


    //task_create(&tarefa_principal, (void *) principal, (void*) 1); //cria tarefa main

    //Temporizador
    acao.sa_handler = tique; //função que será chamada a cada disparo
    sigemptyset (&acao.sa_mask);
    acao.sa_flags = 0 ;
    if (sigaction (SIGALRM, &acao, 0) < 0)
    {
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }
        tempo.it_value.tv_usec = 100;	// primeiro disparo, em micro-segundos
        tempo.it_interval.tv_usec = 1000;	// disparos subsequentes, em micro-segundos
    if (setitimer (ITIMER_REAL, &tempo, 0) < 0)
    {
        perror ("Erro em setitimer: ") ;
        exit (1) ;
    }
    task_create(&dispatcher, (void *)dispatcher_body, NULL); //cria tarefa dispatcher
    //Fim da Inicialização do Temporizador

}

int task_create (task_t *task, void (*start_func)(void *), void *arg){
    // Inicia contexto atual em "tarefa_atual"
    if(task==NULL) return -1; //retorna erro se a tarefa é invalida
    getcontext(&(task->task_context));
    task->id = id_tarefa; //define um id unico para a nova tarefa
    task->init_exec_time = systime();
    task->process_time_count = 0;
    task->activations = 0;
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
    tarefa_atual->final_exec_time = systime();
  /*  printf ("Task %d exit: execution time %4d ms, processor time %4d ms, %d activations\n",
            tarefa_atual->id,
            tarefa_atual->final_exec_time - tarefa_atual->init_exec_time,
            tarefa_atual->process_time_count,
            tarefa_atual->activations) ;*/
    if (tarefa_atual != &dispatcher){
        tarefa_atual->status = 'T'; //altera status para terminada
        tarefa_atual->exitCode = exitCode; //salva exit code
        queue_remove((queue_t**) &fila_tarefas, (queue_t*) tarefa_atual);

        while (queue_size((queue_t*)tarefa_atual->joined) > 0) {
          //preciso salvar a referencia pra não perder no remove
          task_t *aux = tarefa_atual->joined;
          queue_remove((queue_t**) &(tarefa_atual->joined), (queue_t*) aux);
          queue_append ((queue_t**) &fila_tarefas, (queue_t*) aux); //adiciona na fila
        }
        task_switch(&dispatcher);
        return;
    }
    task_switch(&tarefa_principal); //devolve o processador para tarefa main
}

int task_switch (task_t *task) //troca o uso do processador entre tarefas
{
    if(task==NULL) return -1; //retorna erro se a tarefa é invalida
    task_t *tarefa_anterior = tarefa_atual;
    tarefa_atual = task;
    tarefa_atual->quantum = 20;
    tarefa_atual->activations += 1;
    swapcontext(&(tarefa_anterior->task_context), &(tarefa_atual->task_context));
    return 0;
}

int task_id ()
{
    return tarefa_atual->id;
}
//retorna processador para despachante, a tarefa vai para o final da fila
void task_yield (){
    //if (tarefa_atual != &tarefa_principal){ //agora deixa a principal entrar na fila também
    queue_remove((queue_t**) &fila_tarefas, (queue_t*) tarefa_atual);
    queue_append((queue_t**) &fila_tarefas, (queue_t*) tarefa_atual);
    //}
    task_switch(&dispatcher);
}
void task_resume (task_t *task){
    queue_remove((queue_t**) &fila_tarefas_suspensas, (queue_t*) task);
    queue_append((queue_t**) &fila_tarefas, (queue_t*) task);
    task->status = 'P'; //tarefa pronta
}
void task_suspend (task_t *task, task_t **queue){
    if (task != NULL){
      //Remove da Fila de Prontas (obrigatório)
      queue_remove((queue_t **) &fila_tarefas, (queue_t *) task);
      //Adiciona na Fila de Suspensas (se for usar em outra fila, comentar)
      // queue_append((queue_t **) queue, (queue_t *) tarefa_atual);
      //Altera o Status
      task->status = 'S';
    }
    /*if (task != NULL){
        // adicionada tarefa atual na lista da tarefa ¨task¨
        queue_append((queue_t **) queue, (queue_t *) tarefa_atual);
        // tarefa atual é suspensa
        tarefa_atual->status = 'S';
        // volta p/ o dispatcher
        task_yield();
    }*/
    // else {
    //     task = tarefa_atual;
    //     queue_append((queue_t **) queue, (queue_t *) task);
    // }
    // task->status = 'S'; //tarefa suspensa
    // if (queue != NULL){
    //     queue_remove((queue_t **) fila_tarefas, (queue_t *) task);
    // }
}
void task_setprio (task_t *task, int prio){
    if (task == NULL)
        task = tarefa_atual;

    if (prio >= -20 && prio <= 20){
        task->prioridade_estatica = prio;
        task->prioridade_dinamica = prio;
    }
}
int task_getprio (task_t *task){
    if (task != NULL)
        return task->prioridade_estatica;
    return tarefa_atual->prioridade_estatica;
}

int task_join (task_t *task) {

  /*
  Não pode estar em duas filas por causa desse trecho
  do if no queue.c
  (elem->next == NULL) && (elem->prev == NULL)
  */
  if (task != NULL && task->status != 'T'){
    //ñ precisa adicionar, ela já foi criada (task_create), portanto...
    //queue_append((queue_t**) &fila_tarefas, (queue_t*) task);
    task_suspend (tarefa_atual, (task_t**) &fila_tarefas_suspensas);
    queue_append((queue_t**) &(task->joined), (queue_t*) tarefa_atual);
    task_switch(&dispatcher);
    return task->exitCode;
  }
  return -1;
}

void task_sleep (int t) {
  //Retira tarefa da fila de prontas
  queue_remove((queue_t **) &fila_tarefas, (queue_t *) tarefa_atual);
  //Coloca a tarefa na fila de adormecidas
  queue_append((queue_t **) &fila_tarefas_adormecidas, (queue_t*) tarefa_atual);
  //Calcula o instante em que a tarefa será acordada
  tarefa_atual->sleep = systime() + t*1000;
  //Retorna controle para o dispatcher
  task_switch(&dispatcher);
}

int sem_create (semaphore_t *s, int value) {
  if(s==NULL) return -1; //retorna erro se o semáforo é invalido

  s->counter = value;
  return 0;
}

int sem_down (semaphore_t *s) {
  if(s==NULL) return -1; //retorna erro se o semáforo é invalido

  s->counter--;

  if (s->counter < 0) {
    task_suspend (tarefa_atual, (task_t**) &fila_tarefas_suspensas);
    queue_append((queue_t**) &(s->queue), (queue_t*) tarefa_atual);
    task_switch(&dispatcher);
  }
  return 0;
}

// libera o semáforo
int sem_up (semaphore_t *s) {
  if(s==NULL) return -1; //retorna erro se o semáforo é invalido

  s->counter++;

  if (s->counter <= 0) {
    task_t *tarefa = s->queue;
    queue_remove((queue_t **) &s->queue, (queue_t *) tarefa);
    queue_append((queue_t **) &fila_tarefas, (queue_t*) tarefa);
    task_switch(&dispatcher);
  }
  return 0;
}

// destroi o semáforo, liberando as tarefas bloqueadas
int sem_destroy (semaphore_t *s) {
  if(s==NULL) return -1; //retorna erro se o semáforo é invalido

  task_t *tarefa = s->queue;
  //Salva tamanho da fila
  short tam = queue_size((queue_t*)s->queue);
  short i = 0;
  //percorre fila inteira
  while (i < tam){
    //remove da fila do semáforo e coloca na fila de prontas
    queue_remove((queue_t **) &s->queue, (queue_t *) tarefa);
    queue_append((queue_t **) &fila_tarefas, (queue_t*) tarefa);
    //'anda' na fila
    tarefa = tarefa->next;
    i++;
  }

  return 0;
}

// Inicializa uma barreira
int barrier_create (barrier_t *b, int N) {
  if(b==NULL) return -1; //retorna erro se a barreira é invalida
  if (N > 0)
    b->count = N;
  else
    return -1;
  return 0;
}

// Chega a uma barreira
int barrier_join (barrier_t *b) {
  if(b==NULL) return -1; //retorna erro se a barreira é invalida

  task_suspend (tarefa_atual, (task_t**) &fila_tarefas_suspensas);
  queue_append((queue_t**) &(b->queue), (queue_t*) tarefa_atual);
  b->count--;

  if (b->count <= 0) {
    barrier_destroy(b);
  }

  task_switch(&dispatcher);
  return 0;
}

// Destrói uma barreira
int barrier_destroy (barrier_t *b) {
  if(b==NULL) return -1; //retorna erro se o semáforo é invalido

  task_t *tarefa = b->queue;
  //Salva tamanho da fila
  short tam = queue_size((queue_t*)b->queue);
  short i = 0;
  //percorre fila inteira

  while (i < tam) {
    task_t *prox_tarefa = tarefa->next;

    //remove da fila do semáforo e coloca na fila de prontas
    queue_remove((queue_t **) &(b->queue), (queue_t *) tarefa);
    queue_append((queue_t **) &fila_tarefas, (queue_t*) tarefa);

    //'anda' na fila
    tarefa = prox_tarefa;
    i++;
  }

  return 0;
}
