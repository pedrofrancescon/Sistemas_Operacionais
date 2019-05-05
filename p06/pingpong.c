#include "pingpong.h"

task_t *tarefa_atual; //variavel global da tarefa corrente
task_t *fila_tarefas; //fila de fila_tarefas
task_t *fila_tarefas_suspensas; //fila de tarefas suspenas (ainda ñ usado aqui)
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

task_t* scheduler(){
    //return fila_tarefas;
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
    fila_tarefas->prioridade_dinamica -= 1; //envelhecimento do primeiro elemento
    prioritario->prioridade_dinamica = prioritario->prioridade_estatica; //retorno da prioridade original
    return prioritario;
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
void tique(){
    program_clock += 1;
    tarefa_atual->process_time_count += 1;
    if (task_id() != 1){ //tarefa de usuário
        tarefa_atual->quantum -= 1;
        
        if (tarefa_atual->quantum == 0)
            task_yield();
    }
}
void pingpong_init ()
{
    setvbuf (stdout, 0, _IONBF, 0); // Para desativar o buffer do printf.
    id_tarefa = 1; //atribuido a novas tarefas id com valores maior que zero
    tarefa_principal.id=0; //tarefa main deve ter id zero
    tarefa_atual=&tarefa_principal; //primeira tarefa deve ser a principal
    program_clock = 0;

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
    
    //Fim da Inicialização do Temporizador
    _task_create(&dispatcher, (void *)dispatcher_body, NULL); //cria tarefa dispatcher
}

int _task_create (task_t *task, void (*start_func)(void *), void *arg){
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
    printf ("Task %d exit: execution time %4d ms, processor time %4d ms, %d activations",
            tarefa_atual->id,
            tarefa_atual->final_exec_time - tarefa_atual->init_exec_time,
            tarefa_atual->process_time_count,
            tarefa_atual->activations) ;
    if (tarefa_atual != &dispatcher){
        queue_remove((queue_t**) &fila_tarefas, (queue_t*) tarefa_atual);
        task_switch(&dispatcher);
        printf ("INICIO em %4d ms\n", systime()) ;
        
        return;
    }
    task_switch(&tarefa_principal); //devolve o processador para tarefa main
}

int task_switch (task_t *task) //ttroca o uso do processador entre tarefas
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
    if (tarefa_atual != &tarefa_principal){
        queue_remove((queue_t**) &fila_tarefas, (queue_t*) tarefa_atual);
        queue_append((queue_t**) &fila_tarefas, (queue_t*) tarefa_atual);
    }
    task_switch(&dispatcher);
}
void _task_resume (task_t *task){
    queue_remove((queue_t**) &fila_tarefas_suspensas, (queue_t*) task);
    queue_append((queue_t**) &fila_tarefas, (queue_t*) task);
    task->status = 'P'; //tarefa pronta
}
void _task_suspend (task_t *task, task_t **queue){
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
