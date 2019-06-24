#include "queue.h"
#include <stdio.h>
#ifndef NULL
#define NULL ((void *) 0)
#endif


//------------------------------------------------------------------------------
// Insere um elemento no final da fila.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - o elemento deve existir
// - o elemento nao deve estar em outra fila

void queue_append (queue_t **queue, queue_t *elem) {
    if ((queue != NULL) //&& (*queue != NULL)
        && (elem != NULL)
        && (elem->next == NULL) && (elem->prev == NULL)) {
        
        if (*queue == NULL) {
            // primeira insercao
            *queue = elem;
            elem->next = elem;
            elem->prev = elem;
        }
        else {
            // demais insercoes = insere no fim
            queue_t *aux = (*queue)->prev; // ultimo elemento
            aux->next = elem; // insere elemento no final
            elem->next = *queue; // proximo do ultimo elmento eh o primeiro
            elem->prev = aux; // anterior do elemento inserido eh o "antigo" ultimo elemento
            (*queue)->prev = elem; // anterior do primeiro elemento eh o ultimo
        }
    }
}

//------------------------------------------------------------------------------
// Remove o elemento indicado da fila, sem o destruir.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - a fila nao deve estar vazia
// - o elemento deve existir
// - o elemento deve pertencer a fila indicada
// Retorno: apontador para o elemento removido, ou NULL se erro

queue_t *queue_remove (queue_t **queue, queue_t *elem) {
    if ((queue != NULL) && (*queue != NULL) // fila existe?
        && ((*queue)->next != NULL) // esta vazia?
        && (elem != NULL) // elemento existe?
        ) {
        queue_t *aux = *queue;
        // busca pelo elemento na lista
        while ((aux->next != *queue) && (aux != elem)) {
            aux = aux->next;
        }
        if (aux == elem) {
            // encontrou, remove da lista
            // (apenas acerta os apontamentos
            elem->next->prev = elem->prev;
            elem->prev->next = elem->next;
            
            if (*queue == elem) {
                // o elemento removido eh o primeiro
                // precisa acertar a cabeca da lista
                *queue = elem->next;
                if (*queue == elem)
                    // o elemento removido eh o unico elemento da lista
                    // a lista deve ficar vazia
                    *queue = NULL;
            }
            
            // desconecta o elemento da lista
            elem->prev = elem->next = NULL;
            return elem;
        }
    }
    return NULL;
}

//------------------------------------------------------------------------------
// Conta o numero de elementos na fila
// Retorno: numero de elementos na fila

int queue_size (queue_t *queue) {
    int size;
    queue_t *aux;
    if ((queue == NULL) || ((queue != NULL) && (queue->next == NULL)))
        return 0;
    // se tiver elementos inseritos, contar a quantidade
    aux = queue;
    size = 1;
    while (aux->next != queue) {
        size++;
        aux = aux->next;
    }
    return size;
}

//------------------------------------------------------------------------------
// Percorre a fila e imprime na tela seu conteúdo. A impressão de cada
// elemento é feita por uma função externa, definida pelo programa que
// usa a biblioteca. Essa função deve ter o seguinte protótipo:
//
// void print_elem (void *ptr) ; // ptr aponta para o elemento a imprimir

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {
    queue_t *aux = queue;
    printf("%s: [", name);
    if (aux != NULL) {
        while (aux->next != queue) {
            print_elem(aux);
            aux = aux->next;
        }
        print_elem(aux);
    }
    printf("]\n");
}
