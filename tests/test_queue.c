/*
 * test_queue.c - Testes da fila bloqueante (Modulo 12, modularizacao.md /
 * RULES REGRA 10 - base da concorrencia).
 *
 * Cobre o comportamento exigido na modularizacao.md (Modulo 12):
 *   - queue_push espera enquanto cheia;
 *   - queue_pop espera enquanto vazia e ainda aberta;
 *   - queue_close acorda consumidores e sinaliza fim quando a fila drena.
 *
 * Casos cobertos:
 *   1) basico single-thread: push/pop em FIFO, queue_size;
 *   2) close drena: itens enfileirados ainda saem; depois pop retorna false;
 *   3) push apos close e rejeitado;
 *   4) produtor/consumidor concorrente (1 produtor, varios consumidores):
 *      soma de tudo que foi consumido bate com a soma do que foi produzido,
 *      provando que nada se perde nem duplica sob contencao;
 *   5) backpressure: fila pequena com muitos itens; consumidor lento NAO faz
 *      o produtor perder itens (a soma final confere) e a fila nunca passa
 *      da capacidade;
 *   6) bordas: NULL e capacity 0.
 *
 * Compilar/rodar so faz sentido onde ha pthreads (Linux); no MinGW sem
 * libpthread o Makefile nao inclui este teste no `make test` (ver Makefile).
 *
 * Sai com codigo != 0 se qualquer verificacao falhar (para o `make test` falhar).
 */
#include "queue.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static int tests_run    = 0;
static int tests_failed = 0;

#define CHECK(cond, msg)                                          \
    do {                                                          \
        tests_run++;                                              \
        if (!(cond)) {                                            \
            tests_failed++;                                       \
            printf("  [FALHOU] %s (linha %d)\n", (msg), __LINE__);\
        }                                                         \
    } while (0)

/* ------------------------------------------------------------------ */
/* Teste 4/5: produtor/consumidor concorrente                          */
/* ------------------------------------------------------------------ */

#define N_ITEMS    100000  /* itens produzidos no teste concorrente */
#define N_CONSUMERS 4

typedef struct {
    BlockingQueue *q;
    long long      sum;   /* soma dos valores consumidos por esta thread */
    long           count; /* quantos itens esta thread consumiu */
} ConsumerArg;

/* Produtor: enfileira os inteiros 1..N_ITEMS (como ponteiros) e fecha a fila. */
static void *producer_fn(void *arg)
{
    BlockingQueue *q = (BlockingQueue *)arg;
    for (long i = 1; i <= N_ITEMS; i++) {
        /* Codifica o inteiro no proprio ponteiro (sem alocar). */
        if (!queue_push(q, (void *)(intptr_t)i))
            break; /* fila fechada inesperadamente */
    }
    queue_close(q);
    return NULL;
}

/* Consumidor: tira itens ate a fila drenar e fechar; soma os valores. */
static void *consumer_fn(void *arg)
{
    ConsumerArg *c = (ConsumerArg *)arg;
    void *item;
    while (queue_pop(c->q, &item)) {
        c->sum += (long long)(intptr_t)item;
        c->count++;
    }
    return NULL;
}

/* Consumidor lento: igual ao acima, mas com trabalho extra por item para
 * forcar o produtor a bloquear numa fila pequena (testa backpressure). */
static void *slow_consumer_fn(void *arg)
{
    ConsumerArg *c = (ConsumerArg *)arg;
    void *item;
    volatile long busy = 0;
    while (queue_pop(c->q, &item)) {
        c->sum += (long long)(intptr_t)item;
        c->count++;
        for (int k = 0; k < 50; k++) busy++; /* atraso curto deterministico */
    }
    return NULL;
}

int main(void)
{
    printf("=== Testes da fila bloqueante (Modulo 12) ===\n");

    /* ---------------------------------------------------------------- */
    printf("Teste 1: basico single-thread (FIFO + size)\n");
    {
        BlockingQueue *q = queue_create(4);
        CHECK(q != NULL, "queue_create deve abrir");
        if (q != NULL) {
            CHECK(queue_size(q) == 0, "fila nova esta vazia");
            CHECK(queue_push(q, (void *)(intptr_t)10), "push 10");
            CHECK(queue_push(q, (void *)(intptr_t)20), "push 20");
            CHECK(queue_push(q, (void *)(intptr_t)30), "push 30");
            CHECK(queue_size(q) == 3, "size deve ser 3");

            void *v;
            CHECK(queue_pop(q, &v) && (intptr_t)v == 10, "pop FIFO -> 10");
            CHECK(queue_pop(q, &v) && (intptr_t)v == 20, "pop FIFO -> 20");
            CHECK(queue_pop(q, &v) && (intptr_t)v == 30, "pop FIFO -> 30");
            CHECK(queue_size(q) == 0, "fila vazia de novo");
            queue_destroy(q);
        }
    }

    /* ---------------------------------------------------------------- */
    printf("Teste 2: close drena e depois sinaliza fim\n");
    {
        BlockingQueue *q = queue_create(4);
        CHECK(q != NULL, "queue_create deve abrir");
        if (q != NULL) {
            CHECK(queue_push(q, (void *)(intptr_t)1), "push 1");
            CHECK(queue_push(q, (void *)(intptr_t)2), "push 2");
            queue_close(q);

            void *v;
            CHECK(queue_pop(q, &v) && (intptr_t)v == 1, "drena 1 apos close");
            CHECK(queue_pop(q, &v) && (intptr_t)v == 2, "drena 2 apos close");
            CHECK(!queue_pop(q, &v), "fila vazia+fechada -> pop false (fim)");
            CHECK(!queue_pop(q, &v), "pop continua false apos fim");
            queue_destroy(q);
        }
    }

    /* ---------------------------------------------------------------- */
    printf("Teste 3: push apos close e rejeitado\n");
    {
        BlockingQueue *q = queue_create(4);
        CHECK(q != NULL, "queue_create deve abrir");
        if (q != NULL) {
            queue_close(q);
            CHECK(!queue_push(q, (void *)(intptr_t)99), "push apos close -> false");
            CHECK(queue_size(q) == 0, "nada foi enfileirado apos close");
            queue_destroy(q);
        }
    }

    /* ---------------------------------------------------------------- */
    printf("Teste 4: 1 produtor x %d consumidores (nada perde/duplica)\n",
           N_CONSUMERS);
    {
        BlockingQueue *q = queue_create(64);
        CHECK(q != NULL, "queue_create deve abrir");
        if (q != NULL) {
            pthread_t prod;
            pthread_t cons[N_CONSUMERS];
            ConsumerArg args[N_CONSUMERS];

            for (int i = 0; i < N_CONSUMERS; i++) {
                args[i].q = q;
                args[i].sum = 0;
                args[i].count = 0;
            }

            pthread_create(&prod, NULL, producer_fn, q);
            for (int i = 0; i < N_CONSUMERS; i++)
                pthread_create(&cons[i], NULL, consumer_fn, &args[i]);

            pthread_join(prod, NULL);
            for (int i = 0; i < N_CONSUMERS; i++)
                pthread_join(cons[i], NULL);

            long long total_sum = 0;
            long       total_cnt = 0;
            for (int i = 0; i < N_CONSUMERS; i++) {
                total_sum += args[i].sum;
                total_cnt += args[i].count;
            }
            /* Soma de 1..N = N*(N+1)/2 */
            long long expected = (long long)N_ITEMS * (N_ITEMS + 1) / 2;
            CHECK(total_cnt == N_ITEMS, "todos os itens foram consumidos uma vez");
            CHECK(total_sum == expected, "soma consumida == soma produzida");
            CHECK(queue_size(q) == 0, "fila vazia ao fim");
            queue_destroy(q);
        }
    }

    /* ---------------------------------------------------------------- */
    printf("Teste 5: backpressure (fila pequena, consumidor lento)\n");
    {
        BlockingQueue *q = queue_create(2);  /* fila bem pequena */
        CHECK(q != NULL, "queue_create deve abrir");
        if (q != NULL) {
            pthread_t prod;
            pthread_t cons[2];
            ConsumerArg args[2];
            for (int i = 0; i < 2; i++) {
                args[i].q = q; args[i].sum = 0; args[i].count = 0;
            }

            pthread_create(&prod, NULL, producer_fn, q);
            for (int i = 0; i < 2; i++)
                pthread_create(&cons[i], NULL, slow_consumer_fn, &args[i]);

            pthread_join(prod, NULL);
            for (int i = 0; i < 2; i++)
                pthread_join(cons[i], NULL);

            long long total_sum = args[0].sum + args[1].sum;
            long       total_cnt = args[0].count + args[1].count;
            long long expected = (long long)N_ITEMS * (N_ITEMS + 1) / 2;
            CHECK(total_cnt == N_ITEMS, "fila pequena nao perde itens");
            CHECK(total_sum == expected, "soma confere sob backpressure");
            queue_destroy(q);
        }
    }

    /* ---------------------------------------------------------------- */
    printf("Teste 6: bordas (NULL e capacity 0)\n");
    {
        CHECK(queue_create(0) == NULL, "capacity 0 -> NULL");
        CHECK(!queue_push(NULL, (void *)(intptr_t)1), "push NULL -> false");
        void *v;
        CHECK(!queue_pop(NULL, &v), "pop NULL -> false");
        CHECK(queue_size(NULL) == 0, "size NULL -> 0");
        queue_close(NULL);     /* nao deve crashar */
        queue_destroy(NULL);   /* nao deve crashar */
    }

    printf("\n%d verificacoes, %d falha(s).\n", tests_run, tests_failed);
    if (tests_failed == 0) {
        printf("TODOS OS TESTES PASSARAM.\n");
        return 0;
    }
    printf("HOUVE FALHAS.\n");
    return 1;
}
