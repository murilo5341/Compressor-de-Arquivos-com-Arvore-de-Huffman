/*
 * queue.h - Fila bloqueante (limitada) com mutex e condvars (Modulo 12).
 *
 * Esta e a BASE DA CONCORRENCIA do projeto (RULES REGRA 5 / E3). O pipeline de
 * compressao (Modulos 13/14) liga seus estagios por filas deste tipo:
 *   thread leitora --> [fila de blocos crus] --> N codificadoras
 *   N codificadoras --> [fila de blocos comprimidos] --> thread escritora
 *
 * A fila e LIMITADA (capacidade fixa) para dar contrapressao (backpressure):
 * quando um estagio e mais rapido que o outro, o produtor bloqueia ao encher a
 * fila em vez de consumir memoria sem limite (relevante no teste de fogo de 1 GB).
 *
 * Implementada com um buffer circular protegido por um pthread_mutex_t e duas
 * condition variables:
 *   - not_empty: consumidores esperam aqui enquanto a fila esta vazia;
 *   - not_full : produtores esperam aqui enquanto a fila esta cheia.
 *
 * A fila guarda ponteiros opacos (void*). Quem usa decide o que o ponteiro
 * significa (no Modulo 13, um bloco cru ou um bloco comprimido). A fila NAO e
 * dona dos itens: nao libera o que foi enfileirado.
 *
 * CONTRATO DE FECHAMENTO (close): quando nao havera mais producao, o produtor
 * chama queue_close(). Isso acorda todos os consumidores parados em queue_pop;
 * os itens ja enfileirados ainda saem normalmente (a fila "drena") e, depois de
 * vazia, queue_pop passa a retornar false — o sinal de fim para o consumidor
 * encerrar seu laco sem precisar de um item sentinela.
 *
 * PORTABILIDADE: usa pthreads. O MinGW (GCC 6.3.0, modelo win32) do ambiente de
 * desenvolvimento NAO possui libpthread, entao a concorrencia (Modulos 12-14) e
 * compilada/validada em Linux (-pthread + make tsan/asan). Por isso o teste
 * test_queue so entra no `make test` em Linux (ver Makefile).
 */
#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stddef.h>

/* Tipo opaco: o usuario manipula a fila apenas pelos ponteiros abaixo. */
typedef struct BlockingQueue BlockingQueue;

/*
 * Cria uma fila com capacidade 'capacity' (numero maximo de itens simultaneos).
 * capacity deve ser > 0. Retorna NULL se capacity == 0 ou se faltar memoria /
 * falhar a inicializacao do mutex/condvars.
 */
BlockingQueue *queue_create(size_t capacity);

/*
 * Insere 'item' na fila. BLOQUEIA enquanto a fila estiver cheia, ate haver
 * espaco (ou ate a fila ser fechada). Retorna:
 *   - true  : item enfileirado;
 *   - false : fila NULL, ou fila fechada (queue_close ja foi chamada) -> o item
 *             NAO e enfileirado. Apos o close, nenhum push e aceito.
 */
bool queue_push(BlockingQueue *q, void *item);

/*
 * Remove o item mais antigo da fila (FIFO) e o devolve em *out. BLOQUEIA
 * enquanto a fila estiver vazia E ainda aberta. Retorna:
 *   - true  : item retirado e escrito em *out;
 *   - false : fila NULL, ou fila VAZIA E FECHADA (drenada) -> fim da producao,
 *             o consumidor deve encerrar. *out fica inalterado nesse caso.
 * 'out' pode ser NULL (o item retirado e descartado), embora o uso normal
 * passe um ponteiro valido.
 */
bool queue_pop(BlockingQueue *q, void **out);

/*
 * Marca a fila como FECHADA e acorda todos os produtores e consumidores
 * bloqueados. Itens ja enfileirados continuam saindo por queue_pop; quando a
 * fila esvaziar, queue_pop retorna false. Idempotente e segura com NULL.
 */
void queue_close(BlockingQueue *q);

/* Quantidade atual de itens na fila (0 se q for NULL). */
size_t queue_size(BlockingQueue *q);

/*
 * Libera a fila e os recursos do mutex/condvars. NAO libera os itens que ainda
 * estiverem enfileirados (a fila nao e dona deles). Segura chamar com NULL.
 */
void queue_destroy(BlockingQueue *q);

#endif /* QUEUE_H */
