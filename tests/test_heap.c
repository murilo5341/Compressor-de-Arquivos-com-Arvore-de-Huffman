/*
 * test_heap.c - Testes unitarios do min-heap binario (Modulo 1, RULES REGRA 10).
 *
 * Cobre os casos obrigatorios:
 *   1) insercao fora de ordem -> extracao em ordem crescente de frequencia;
 *   2) extracao de heap vazio (e ponteiros NULL) sem travar;
 *   3) empates resolvidos de forma deterministica (ordem de insercao).
 *
 * Sai com codigo != 0 se qualquer verificacao falhar (para o `make test` falhar).
 */
#include "heap.h"

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

/* Teste 1: frequencias inseridas fora de ordem saem em ordem crescente. */
static void test_ordem_crescente(void)
{
    printf("Teste 1: extracao em ordem crescente de frequencia\n");
    MinHeap *h = heap_create(0);
    CHECK(h != NULL, "heap_create deve retornar heap valido");

    uint64_t entrada[] = {5, 1, 8, 3, 1, 9, 2, 7, 0, 4};
    size_t n = sizeof(entrada) / sizeof(entrada[0]);
    for (size_t i = 0; i < n; i++) {
        HeapItem it = { entrada[i], NULL };
        CHECK(heap_push(h, it), "heap_push deve ter sucesso");
    }
    CHECK(heap_size(h) == n, "tamanho deve refletir os pushes");

    HeapItem out;
    uint64_t anterior = 0;
    bool primeiro = true;
    size_t extraidos = 0;
    while (heap_pop_min(h, &out)) {
        if (!primeiro)
            CHECK(out.frequency >= anterior,
                  "frequencias devem sair nao-decrescentes");
        anterior  = out.frequency;
        primeiro  = false;
        extraidos++;
    }
    CHECK(extraidos == n, "deve extrair todos os elementos inseridos");
    CHECK(heap_is_empty(h), "heap deve ficar vazio ao final");

    heap_destroy(h);
}

/* Teste 2: pop em heap vazio / NULL nao deve quebrar. */
static void test_heap_vazio(void)
{
    printf("Teste 2: extracao de heap vazio nao deve quebrar\n");
    MinHeap *h = heap_create(4);
    CHECK(h != NULL, "heap_create deve retornar heap valido");

    HeapItem out;
    CHECK(!heap_pop_min(h, &out), "pop em heap vazio deve retornar false");
    CHECK(!heap_pop_min(h, NULL), "pop com out NULL em heap vazio deve retornar false");
    CHECK(heap_is_empty(h), "heap recem-criado esta vazio");

    heap_destroy(h);

    /* Robustez com NULL (a API e segura contra NULL). */
    heap_destroy(NULL);
    CHECK(!heap_pop_min(NULL, &out), "pop em heap NULL deve retornar false");
    CHECK(heap_size(NULL) == 0, "tamanho de heap NULL e 0");
    CHECK(heap_is_empty(NULL), "heap NULL e considerado vazio");
}

/* Teste 3: empates de frequencia saem na ordem de insercao (deterministico). */
static void test_empates(void)
{
    printf("Teste 3: empate de frequencia sai em ordem de insercao\n");
    MinHeap *h = heap_create(0);
    CHECK(h != NULL, "heap_create deve retornar heap valido");

    /* Cinco itens com a MESMA frequencia, rotulados 0..4 na ordem de insercao. */
    static int rotulos[5] = {0, 1, 2, 3, 4};
    for (size_t i = 0; i < 5; i++) {
        HeapItem it = { 42, &rotulos[i] };
        CHECK(heap_push(h, it), "heap_push deve ter sucesso");
    }
    /* Intercala uma frequencia menor e uma maior para forcar reordenacao. */
    HeapItem menor = { 10, NULL };
    HeapItem maior = { 99, NULL };
    heap_push(h, menor);
    heap_push(h, maior);

    HeapItem out;
    /* Primeiro sai a menor (10). */
    CHECK(heap_pop_min(h, &out) && out.frequency == 10,
          "menor frequencia sai primeiro");

    /* Depois os cinco empates (42) na ordem de insercao: 0,1,2,3,4. */
    for (int esperado = 0; esperado < 5; esperado++) {
        CHECK(heap_pop_min(h, &out), "deve extrair empate");
        CHECK(out.frequency == 42, "empates tem frequencia 42");
        CHECK(out.payload != NULL && *(int *)out.payload == esperado,
              "empates saem na ordem de insercao");
    }
    /* Por fim a maior (99). */
    CHECK(heap_pop_min(h, &out) && out.frequency == 99,
          "maior frequencia sai por ultimo");

    heap_destroy(h);
}

int main(void)
{
    printf("=== Testes do heap binario (Modulo 1) ===\n");
    test_ordem_crescente();
    test_heap_vazio();
    test_empates();

    printf("\n%d verificacoes, %d falha(s).\n", tests_run, tests_failed);
    if (tests_failed == 0) {
        printf("TODOS OS TESTES PASSARAM.\n");
        return EXIT_SUCCESS;
    }
    printf("HOUVE FALHAS.\n");
    return EXIT_FAILURE;
}
