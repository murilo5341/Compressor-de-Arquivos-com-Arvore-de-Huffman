/*
 * test_huffman_tree.c - Testes da construcao da arvore de Huffman (Modulo 3).
 *
 * Cobre os casos exigidos pela modularizacao.md (Modulo 3) e pela RULES REGRA 10:
 *   1) contagem de frequencias correta a partir de um vetor de bytes;
 *   2) construcao de uma arvore VALIDA (folhas = simbolos distintos, frequencia
 *      de cada no interno = soma dos filhos, total preservado);
 *   3) bloco com um UNICO byte distinto -> arvore e uma folha unica;
 *   4) bloco vazio -> arvore NULL;
 *   5) arquivo com os 256 valores possiveis de byte -> arvore valida;
 *   6) simbolos mais frequentes ficam mais perto da raiz (codigos mais curtos).
 *
 * Sai com codigo != 0 se qualquer verificacao falhar (para o `make test` falhar).
 */
#include "huffman.h"

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
/* Helpers de inspecao da arvore (usados so nos testes)                */
/* ------------------------------------------------------------------ */

/* Conta quantas folhas a arvore possui. */
static size_t contar_folhas(const HuffNode *no)
{
    if (no == NULL)
        return 0;
    if (no->is_leaf)
        return 1;
    return contar_folhas(no->left) + contar_folhas(no->right);
}

/* Soma a frequencia de todas as folhas (deve bater com o total de bytes). */
static uint64_t somar_folhas(const HuffNode *no)
{
    if (no == NULL)
        return 0;
    if (no->is_leaf)
        return no->frequency;
    return somar_folhas(no->left) + somar_folhas(no->right);
}

/*
 * Verifica a estrutura: todo no interno tem dois filhos e frequencia igual a
 * soma dos filhos; toda folha nao tem filhos. Retorna true se tudo coerente.
 */
static bool estrutura_valida(const HuffNode *no)
{
    if (no == NULL)
        return false;

    if (no->is_leaf)
        return no->left == NULL && no->right == NULL;

    if (no->left == NULL || no->right == NULL)
        return false;
    if (no->frequency != no->left->frequency + no->right->frequency)
        return false;
    return estrutura_valida(no->left) && estrutura_valida(no->right);
}

/*
 * Profundidade da folha do byte 'symbol' (numero de arestas da raiz ate ela).
 * Retorna -1 se o simbolo nao estiver na arvore. Numa folha unica, retorna 0.
 */
static int profundidade_simbolo(const HuffNode *no, uint8_t symbol, int prof)
{
    if (no == NULL)
        return -1;
    if (no->is_leaf)
        return (no->symbol == symbol) ? prof : -1;

    int esq = profundidade_simbolo(no->left, symbol, prof + 1);
    if (esq >= 0)
        return esq;
    return profundidade_simbolo(no->right, symbol, prof + 1);
}

/* ------------------------------------------------------------------ */
/* Testes                                                              */
/* ------------------------------------------------------------------ */

/* Teste 1: contagem de frequencias de um buffer conhecido. */
static void test_contagem_frequencias(void)
{
    printf("Teste 1: contagem de frequencias\n");

    const char *texto = "aaabbc"; /* a=3, b=2, c=1 */
    uint64_t freq[256];
    huffman_count_frequencies(texto, 6, freq);

    CHECK(freq['a'] == 3, "frequencia de 'a' deve ser 3");
    CHECK(freq['b'] == 2, "frequencia de 'b' deve ser 2");
    CHECK(freq['c'] == 1, "frequencia de 'c' deve ser 1");
    CHECK(freq['z'] == 0, "byte ausente deve ter frequencia 0");

    /* O vetor deve ser reiniciado a cada chamada (nao acumula). */
    huffman_count_frequencies("xx", 2, freq);
    CHECK(freq['x'] == 2, "apos nova contagem, 'x' deve ser 2");
    CHECK(freq['a'] == 0, "apos nova contagem, 'a' deve voltar a 0");

    /* Buffer vazio / NULL: tudo zero, sem crash. */
    huffman_count_frequencies(NULL, 0, freq);
    uint64_t total = 0;
    for (int i = 0; i < 256; i++)
        total += freq[i];
    CHECK(total == 0, "buffer vazio gera frequencias todas zero");
}

/* Teste 2: arvore valida a partir de varios simbolos. */
static void test_arvore_valida(void)
{
    printf("Teste 2: construcao de arvore valida\n");

    const char *texto = "aaabbc"; /* a=3, b=2, c=1, total=6, 3 distintos */
    uint64_t freq[256];
    huffman_count_frequencies(texto, 6, freq);

    HuffNode *raiz = huffman_build_tree(freq);
    CHECK(raiz != NULL, "arvore nao deve ser NULL");
    CHECK(!raiz->is_leaf, "raiz com 3 simbolos deve ser no interno");
    CHECK(contar_folhas(raiz) == 3, "deve haver 3 folhas (3 simbolos distintos)");
    CHECK(raiz->frequency == 6, "frequencia da raiz deve ser o total (6)");
    CHECK(somar_folhas(raiz) == 6, "soma das folhas deve ser o total (6)");
    CHECK(estrutura_valida(raiz), "todo no interno = soma dos filhos");

    huffman_free_tree(raiz);
}

/* Teste 3: um unico byte distinto -> folha unica (caso especial obrigatorio). */
static void test_folha_unica(void)
{
    printf("Teste 3: bloco com um unico byte distinto -> folha unica\n");

    const char *texto = "zzzzzz"; /* so 'z', 6 vezes */
    uint64_t freq[256];
    huffman_count_frequencies(texto, 6, freq);

    HuffNode *raiz = huffman_build_tree(freq);
    CHECK(raiz != NULL, "arvore de um simbolo nao deve ser NULL");
    CHECK(raiz->is_leaf, "raiz deve ser a propria folha");
    CHECK(raiz->left == NULL && raiz->right == NULL, "folha nao tem filhos");
    CHECK(raiz->symbol == 'z', "folha deve representar o byte 'z'");
    CHECK(raiz->frequency == 6, "frequencia da folha deve ser 6");
    CHECK(contar_folhas(raiz) == 1, "deve existir exatamente uma folha");
    CHECK(profundidade_simbolo(raiz, 'z', 0) == 0, "folha unica esta na raiz (prof 0)");

    huffman_free_tree(raiz);
}

/* Teste 4: bloco vazio -> arvore NULL. */
static void test_bloco_vazio(void)
{
    printf("Teste 4: bloco vazio -> arvore NULL\n");

    uint64_t freq[256];
    huffman_count_frequencies(NULL, 0, freq);

    HuffNode *raiz = huffman_build_tree(freq);
    CHECK(raiz == NULL, "sem simbolos, build_tree deve retornar NULL");
    huffman_free_tree(raiz); /* seguro com NULL */

    CHECK(huffman_build_tree(NULL) == NULL, "freq NULL deve retornar NULL");
}

/* Teste 5: arquivo com os 256 valores de byte -> arvore valida. */
static void test_todos_256(void)
{
    printf("Teste 5: todos os 256 valores de byte\n");

    uint64_t freq[256];
    for (int i = 0; i < 256; i++)
        freq[i] = 1; /* cada byte aparece uma vez */

    HuffNode *raiz = huffman_build_tree(freq);
    CHECK(raiz != NULL, "arvore de 256 simbolos nao deve ser NULL");
    CHECK(contar_folhas(raiz) == 256, "deve haver 256 folhas");
    CHECK(raiz->frequency == 256, "frequencia da raiz deve ser 256");
    CHECK(estrutura_valida(raiz), "estrutura deve ser coerente");

    huffman_free_tree(raiz);
}

/* Teste 6: simbolos mais frequentes ficam mais perto da raiz. */
static void test_frequente_mais_raso(void)
{
    printf("Teste 6: simbolo mais frequente tem codigo mais curto\n");

    /* a=3 (mais frequente), b=2, c=1. Huffman: c+b=3, depois a+3=6.
     * Logo 'a' fica a profundidade 1 e 'b','c' a profundidade 2. */
    const char *texto = "aaabbc";
    uint64_t freq[256];
    huffman_count_frequencies(texto, 6, freq);

    HuffNode *raiz = huffman_build_tree(freq);
    CHECK(raiz != NULL, "arvore nao deve ser NULL");

    int prof_a = profundidade_simbolo(raiz, 'a', 0);
    int prof_b = profundidade_simbolo(raiz, 'b', 0);
    int prof_c = profundidade_simbolo(raiz, 'c', 0);

    CHECK(prof_a >= 1, "'a' deve estar na arvore");
    CHECK(prof_b >= 1 && prof_c >= 1, "'b' e 'c' devem estar na arvore");
    CHECK(prof_a <= prof_b, "'a' (mais frequente) nao pode ser mais fundo que 'b'");
    CHECK(prof_a <= prof_c, "'a' (mais frequente) nao pode ser mais fundo que 'c'");

    huffman_free_tree(raiz);
}

int main(void)
{
    printf("=== Testes da arvore de Huffman (Modulo 3) ===\n");
    test_contagem_frequencias();
    test_arvore_valida();
    test_folha_unica();
    test_bloco_vazio();
    test_todos_256();
    test_frequente_mais_raso();

    printf("\n%d verificacoes, %d falha(s).\n", tests_run, tests_failed);
    if (tests_failed == 0) {
        printf("TODOS OS TESTES PASSARAM.\n");
        return EXIT_SUCCESS;
    }
    printf("HOUVE FALHAS.\n");
    return EXIT_FAILURE;
}
