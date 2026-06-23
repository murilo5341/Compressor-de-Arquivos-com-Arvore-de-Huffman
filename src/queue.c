/*
 * queue.c - Implementacao da fila bloqueante limitada (Modulo 12).
 *
 * Buffer circular de 'capacity' posicoes protegido por um unico mutex. Dois
 * indices controlam o anel:
 *   - head: posicao do proximo item a SAIR (pop);
 *   - count: quantidade de itens validos a partir de head.
 * A posicao de ENTRADA (push) e (head + count) % capacity. Guardar head+count
 * (em vez de head+tail) evita a ambiguidade classica "fila cheia x fila vazia"
 * que ocorre quando head == tail.
 *
 * SINCRONIZACAO (padrao monitor): toda manipulacao do anel acontece com o mutex
 * travado. As esperas usam pthread_cond_wait em LACO (while), nao em if, porque:
 *   - cond_wait pode acordar espuriamente;
 *   - varios threads podem ser acordados e competir pelo mutex; ao readquirir o
 *     mutex a condicao precisa ser reavaliada.
 * Por isso cada espera reavalia (cheia / vazia) antes de prosseguir.
 */
#include "queue.h"

#include <pthread.h>
#include <stdlib.h>

struct BlockingQueue {
    void          **items;     /* buffer circular de ponteiros opacos */
    size_t          capacity;  /* tamanho do buffer (numero maximo de itens) */
    size_t          head;      /* indice do proximo item a sair */
    size_t          count;     /* itens atualmente na fila */
    bool            closed;    /* true apos queue_close: nao aceita mais push */
    pthread_mutex_t mutex;
    pthread_cond_t  not_empty; /* sinalizada quando entra item (acorda pop) */
    pthread_cond_t  not_full;  /* sinalizada quando sai item (acorda push) */
};

BlockingQueue *queue_create(size_t capacity)
{
    if (capacity == 0)
        return NULL;

    BlockingQueue *q = malloc(sizeof(*q));
    if (q == NULL)
        return NULL;

    q->items = malloc(capacity * sizeof(*q->items));
    if (q->items == NULL) {
        free(q);
        return NULL;
    }

    q->capacity = capacity;
    q->head     = 0;
    q->count    = 0;
    q->closed   = false;

    /* Inicializa mutex e condvars; em falha, desfaz o que ja deu certo. */
    if (pthread_mutex_init(&q->mutex, NULL) != 0) {
        free(q->items);
        free(q);
        return NULL;
    }
    if (pthread_cond_init(&q->not_empty, NULL) != 0) {
        pthread_mutex_destroy(&q->mutex);
        free(q->items);
        free(q);
        return NULL;
    }
    if (pthread_cond_init(&q->not_full, NULL) != 0) {
        pthread_cond_destroy(&q->not_empty);
        pthread_mutex_destroy(&q->mutex);
        free(q->items);
        free(q);
        return NULL;
    }

    return q;
}

bool queue_push(BlockingQueue *q, void *item)
{
    if (q == NULL)
        return false;

    pthread_mutex_lock(&q->mutex);

    /* Espera enquanto cheia E aberta. Se fechar durante a espera, sai do laco. */
    while (q->count == q->capacity && !q->closed)
        pthread_cond_wait(&q->not_full, &q->mutex);

    /* Apos o fechamento nao se aceita mais producao. */
    if (q->closed) {
        pthread_mutex_unlock(&q->mutex);
        return false;
    }

    size_t tail = (q->head + q->count) % q->capacity;
    q->items[tail] = item;
    q->count++;

    /* Ha pelo menos um item: acorda um consumidor. */
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
    return true;
}

bool queue_pop(BlockingQueue *q, void **out)
{
    if (q == NULL)
        return false;

    pthread_mutex_lock(&q->mutex);

    /* Espera enquanto vazia E aberta. Fechada + vazia => fim da producao. */
    while (q->count == 0 && !q->closed)
        pthread_cond_wait(&q->not_empty, &q->mutex);

    if (q->count == 0) {
        /* So chega aqui se vazia e fechada: drenada, sinaliza fim. */
        pthread_mutex_unlock(&q->mutex);
        return false;
    }

    void *item = q->items[q->head];
    q->head    = (q->head + 1) % q->capacity;
    q->count--;
    if (out != NULL)
        *out = item;

    /* Abriu uma vaga: acorda um produtor. */
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);
    return true;
}

void queue_close(BlockingQueue *q)
{
    if (q == NULL)
        return;

    pthread_mutex_lock(&q->mutex);
    q->closed = true;
    /* Acorda TODOS: consumidores parados (para verem o fim) e produtores
     * parados (para desistirem do push). broadcast, nao signal. */
    pthread_cond_broadcast(&q->not_empty);
    pthread_cond_broadcast(&q->not_full);
    pthread_mutex_unlock(&q->mutex);
}

size_t queue_size(BlockingQueue *q)
{
    if (q == NULL)
        return 0;

    pthread_mutex_lock(&q->mutex);
    size_t n = q->count;
    pthread_mutex_unlock(&q->mutex);
    return n;
}

void queue_destroy(BlockingQueue *q)
{
    if (q == NULL)
        return;

    pthread_cond_destroy(&q->not_full);
    pthread_cond_destroy(&q->not_empty);
    pthread_mutex_destroy(&q->mutex);
    free(q->items);
    free(q);
}
