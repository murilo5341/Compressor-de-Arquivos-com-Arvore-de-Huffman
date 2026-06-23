/*
 * test_huffman_codes.c - Testes da tabela de codigos de Huffman (Modulo 4).
 *
 * Cobre os casos exigidos pela modularizacao.md (Modulo 4) e pela RULES REGRA 10:
 *   1) bytes MAIS frequentes recebem codigos MAIS curtos;
 *   2) propriedade de PREFIXO: nenhum codigo e prefixo de outro;
 *   3) folha unica (um unico byte distinto) -> codigo "0" (bits=0, length=1);
 *   4) cada codigo, seguido bit a bit na arvore, leva de volta ao seu simbolo
 *      (a tabela e coerente com a arvore que a gerou);
 *   5) bordas: root NULL / table NULL -> false; simbolos ausentes -> length 0;
 *   6) arquivo com os 256 valores de byte -> todos recebem codigo.
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
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

/*
 * Verdadeiro se o codigo 'a' (de comprimento la) for PREFIXO do codigo 'b'
 * (de comprimento lb), com la <= lb. Compara os 'la' bits mais significativos
 * de 'b' (os primeiros bits do caminho) com 'a'.
 */
static bool eh_prefixo(uint64_t a, uint8_t la, uint64_t b, uint8_t lb)
{
    if (la == 0 || la > lb)
        return false;
    uint64_t topo_de_b = b >> (lb - la); /* primeiros 'la' bits de 'b' */
    return topo_de_b == a;
}

/*
 * Segue os 'length' bits do codigo descendo a arvore (0 = esquerda, 1 = direita,
 * MSB-first) e devolve o simbolo da folha alcancada. Retorna -1 se o caminho
 * sair da arvore ou nao terminar exatamente numa folha.
 */
static int decodificar(const HuffNode *raiz, uint64_t bits, uint8_t length)
{
    const HuffNode *no = raiz;

    /* Folha unica: codigo de 1 bit que nao "anda" na arvore. */
    if (no != NULL && no->is_leaf)
        return (length == 1) ? no->symbol : -1;

    for (int i = length - 1; i >= 0; i--) {
        if (no == NULL || no->is_leaf)
            return -1;
        uint64_t bit = (bits >> i) & 1u;
        no = bit ? no->right : no->left;
    }
    if (no == NULL || !no->is_leaf)
        return -1;
    return no->symbol;
}

/* ------------------------------------------------------------------ */
/* Testes                                                             */
/* ------------------------------------------------------------------ */

/*
 * Teste 1: distribuicao conhecida "aaabbc" (a=3, b=2, c=1).
 * Arvore: c+b -> 3, depois a+ (c+b) -> 6. Logo 'a' tem codigo de 1 bit e
 * 'b','c' de 2 bits. O byte mais frequente recebe o codigo mais curto.
 */
static void test_frequente_mais_curto(void)
{
    printf("Teste 1: byte mais frequente recebe codigo mais curto\n");

    const char *texto = "aaabbc";
    uint64_t freq[256];
    huffman_count_frequencies(texto, 6, freq);

    HuffNode *raiz = huffman_build_tree(freq);
    CHECK(raiz != NULL, "arvore nao deve ser NULL");

    HuffCode tab[256];
    CHECK(huffman_build_codes(raiz, tab), "build_codes deve ter sucesso");

    CHECK(tab['a'].length == 1, "'a' (mais frequente) deve ter codigo de 1 bit");
    CHECK(tab['b'].length == 2, "'b' deve ter codigo de 2 bits");
    CHECK(tab['c'].length == 2, "'c' deve ter codigo de 2 bits");

    /* freq(a) > freq(b) => length(a) <= length(b) (propriedade do codigo otimo). */
    CHECK(tab['a'].length <= tab['b'].length, "byte mais frequente nao pode ter codigo mais longo");
    CHECK(tab['a'].length <= tab['c'].length, "byte mais frequente nao pode ter codigo mais longo");

    /* Simbolos ausentes ficam com length 0. */
    CHECK(tab['z'].length == 0, "byte ausente deve ter length 0");

    huffman_free_tree(raiz);
}

/*
 * Teste 2: propriedade de prefixo - nenhum codigo e prefixo de outro.
 * Verifica todos os pares de simbolos presentes.
 */
static void test_propriedade_prefixo(void)
{
    printf("Teste 2: propriedade de prefixo\n");

    /* Distribuicao variada para gerar comprimentos diferentes. */
    unsigned char buf[] = {
        'a','a','a','a','a','a','a','a',
        'b','b','b','b',
        'c','c',
        'd','e'
    };
    uint64_t freq[256];
    huffman_count_frequencies(buf, sizeof(buf), freq);

    HuffNode *raiz = huffman_build_tree(freq);
    CHECK(raiz != NULL, "arvore nao deve ser NULL");

    HuffCode tab[256];
    CHECK(huffman_build_codes(raiz, tab), "build_codes deve ter sucesso");

    bool algum_prefixo = false;
    for (int i = 0; i < 256; i++) {
        if (tab[i].length == 0)
            continue;
        for (int j = 0; j < 256; j++) {
            if (i == j || tab[j].length == 0)
                continue;
            if (eh_prefixo(tab[i].bits, tab[i].length, tab[j].bits, tab[j].length)) {
                algum_prefixo = true;
            }
        }
    }
    CHECK(!algum_prefixo, "nenhum codigo pode ser prefixo de outro");

    huffman_free_tree(raiz);
}

/*
 * Teste 3: folha unica (um unico byte distinto) -> codigo "0", 1 bit.
 */
static void test_folha_unica(void)
{
    printf("Teste 3: folha unica -> codigo de 1 bit (0)\n");

    const char *texto = "zzzzzz";
    uint64_t freq[256];
    huffman_count_frequencies(texto, 6, freq);

    HuffNode *raiz = huffman_build_tree(freq);
    CHECK(raiz != NULL, "arvore de um simbolo nao deve ser NULL");

    HuffCode tab[256];
    CHECK(huffman_build_codes(raiz, tab), "build_codes deve ter sucesso");

    CHECK(tab['z'].length == 1, "folha unica deve ter codigo de 1 bit");
    CHECK(tab['z'].bits == 0, "o codigo da folha unica deve ser 0");

    /* Todos os outros bytes continuam ausentes. */
    int presentes = 0;
    for (int i = 0; i < 256; i++)
        if (tab[i].length != 0)
            presentes++;
    CHECK(presentes == 1, "apenas um simbolo deve ter codigo");

    huffman_free_tree(raiz);
}

/*
 * Teste 4: a tabela e coerente com a arvore - cada codigo decodifica de volta
 * para o seu proprio simbolo descendo a arvore bit a bit.
 */
static void test_decodificacao_coerente(void)
{
    printf("Teste 4: cada codigo decodifica de volta ao seu simbolo\n");

    const char *texto = "mississippi river";
    uint64_t freq[256];
    huffman_count_frequencies(texto, 17, freq);

    HuffNode *raiz = huffman_build_tree(freq);
    CHECK(raiz != NULL, "arvore nao deve ser NULL");

    HuffCode tab[256];
    CHECK(huffman_build_codes(raiz, tab), "build_codes deve ter sucesso");

    bool todos_ok = true;
    for (int s = 0; s < 256; s++) {
        if (tab[s].length == 0)
            continue;
        int dec = decodificar(raiz, tab[s].bits, tab[s].length);
        if (dec != s)
            todos_ok = false;
    }
    CHECK(todos_ok, "todo codigo deve levar de volta ao seu simbolo na arvore");

    huffman_free_tree(raiz);
}

/*
 * Teste 5: bordas - root NULL e table NULL retornam false.
 */
static void test_bordas(void)
{
    printf("Teste 5: bordas (root/table NULL)\n");

    HuffCode tab[256];
    CHECK(!huffman_build_codes(NULL, tab), "root NULL deve retornar false");

    const char *texto = "abc";
    uint64_t freq[256];
    huffman_count_frequencies(texto, 3, freq);
    HuffNode *raiz = huffman_build_tree(freq);
    CHECK(raiz != NULL, "arvore nao deve ser NULL");
    CHECK(!huffman_build_codes(raiz, NULL), "table NULL deve retornar false");

    huffman_free_tree(raiz);
}

/*
 * Teste 6: arquivo com os 256 valores de byte -> todos recebem um codigo e a
 * propriedade de prefixo continua valida.
 */
static void test_todos_256(void)
{
    printf("Teste 6: todos os 256 valores de byte recebem codigo\n");

    uint64_t freq[256];
    for (int i = 0; i < 256; i++)
        freq[i] = 1;

    HuffNode *raiz = huffman_build_tree(freq);
    CHECK(raiz != NULL, "arvore de 256 simbolos nao deve ser NULL");

    HuffCode tab[256];
    CHECK(huffman_build_codes(raiz, tab), "build_codes deve ter sucesso");

    int presentes = 0;
    for (int i = 0; i < 256; i++)
        if (tab[i].length != 0)
            presentes++;
    CHECK(presentes == 256, "todos os 256 simbolos devem ter codigo");

    /* Com 256 simbolos equiprovaveis a arvore e balanceada: 8 bits por codigo. */
    bool todos_8 = true;
    for (int i = 0; i < 256; i++)
        if (tab[i].length != 8)
            todos_8 = false;
    CHECK(todos_8, "256 simbolos equiprovaveis devem gerar codigos de 8 bits");

    /* Cada codigo deve decodificar de volta ao seu simbolo. */
    bool todos_ok = true;
    for (int s = 0; s < 256; s++) {
        int dec = decodificar(raiz, tab[s].bits, tab[s].length);
        if (dec != s)
            todos_ok = false;
    }
    CHECK(todos_ok, "todo codigo deve decodificar de volta ao seu simbolo");

    huffman_free_tree(raiz);
}

int main(void)
{
    printf("=== Testes da tabela de codigos de Huffman (Modulo 4) ===\n");
    test_frequente_mais_curto();
    test_propriedade_prefixo();
    test_folha_unica();
    test_decodificacao_coerente();
    test_bordas();
    test_todos_256();

    printf("\n%d verificacoes, %d falha(s).\n", tests_run, tests_failed);
    if (tests_failed == 0) {
        printf("TODOS OS TESTES PASSARAM.\n");
        return EXIT_SUCCESS;
    }
    printf("HOUVE FALHAS.\n");
    return EXIT_FAILURE;
}
