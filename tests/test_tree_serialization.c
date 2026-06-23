/*
 * test_tree_serialization.c - Testes da serializacao da arvore (Modulo 7,
 * modularizacao.md / RULES REGRA 10).
 *
 * O criterio do modulo: serializar uma arvore, desserializar e conferir que a
 * estrutura reconstruida DECODIFICA IGUAL. Aqui isso e provado de duas formas:
 *   - igualdade ESTRUTURAL recursiva (mesmos nos internos, mesmas folhas/simbolos);
 *   - igualdade da TABELA DE CODIGOS (Modulo 4) gerada a partir das duas arvores
 *     -> se as tabelas batem, a (de)codificacao e identica.
 *
 * Casos cobertos:
 *   1) roundtrip de arvore comum (estrutura + tabela de codigos identicas);
 *   2) folha unica (um unico byte distinto): "1 + 8 bits" -> 2 bytes;
 *   3) todos os 256 valores de byte;
 *   4) tamanho serializado coerente (tree_size em bytes > 0);
 *   5) robustez: bytes truncados/insuficientes -> NULL sem crash;
 *   6) bordas: NULL.
 *
 * Sai com codigo != 0 se qualquer verificacao falhar (para o `make test` falhar).
 */
#include "tree_serialization.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "huffman.h"

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

/*
 * Igualdade estrutural recursiva: mesma forma, mesmas folhas e mesmos simbolos.
 * O campo 'frequency' nao e comparado (a desserializacao nao o restaura).
 */
static int arvores_iguais(const HuffNode *a, const HuffNode *b)
{
    if (a == NULL || b == NULL)
        return a == b;
    if (a->is_leaf != b->is_leaf)
        return 0;
    if (a->is_leaf)
        return a->symbol == b->symbol;
    return arvores_iguais(a->left, b->left) && arvores_iguais(a->right, b->right);
}

/*
 * Igualdade de duas tabelas de codigos campo a campo (bits + length). Nao se usa
 * memcmp porque HuffCode tem bytes de padding entre 'length' e o fim da struct,
 * cujo conteudo e indefinido e quebraria a comparacao mesmo com valores iguais.
 */
static int tabelas_iguais(const HuffCode a[256], const HuffCode b[256])
{
    for (int i = 0; i < 256; i++)
        if (a[i].bits != b[i].bits || a[i].length != b[i].length)
            return 0;
    return 1;
}

/* Constroi uma arvore a partir de um bloco de bytes (atalho dos Modulos 3). */
static HuffNode *arvore_do_bloco(const void *buf, size_t len)
{
    uint64_t freq[256];
    huffman_count_frequencies(buf, len, freq);
    return huffman_build_tree(freq);
}

/* Teste 1: roundtrip de uma arvore comum. */
static void test_roundtrip_comum(void)
{
    printf("Teste 1: roundtrip de arvore comum (estrutura + tabela de codigos)\n");

    const char *texto = "ABRACADABRA"; /* distribuicao desigual -> arvore rica */
    HuffNode *orig = arvore_do_bloco(texto, strlen(texto));
    CHECK(orig != NULL, "arvore original construida");

    uint8_t *bytes = NULL;
    size_t   size  = 0;
    CHECK(tree_serialize(orig, &bytes, &size), "tree_serialize deve ter sucesso");
    CHECK(bytes != NULL && size > 0, "buffer serializado nao vazio");

    HuffNode *recon = tree_deserialize(bytes, size);
    CHECK(recon != NULL, "tree_deserialize deve ter sucesso");
    CHECK(arvores_iguais(orig, recon), "estrutura reconstruida deve ser identica");

    /* Tabelas de codigos identicas => (de)codificacao identica. */
    HuffCode ta[256], tb[256];
    CHECK(huffman_build_codes(orig, ta), "codigos da arvore original");
    CHECK(huffman_build_codes(recon, tb), "codigos da arvore reconstruida");
    CHECK(tabelas_iguais(ta, tb), "tabelas de codigos devem ser iguais");

    free(bytes);
    huffman_free_tree(orig);
    huffman_free_tree(recon);
}

/* Teste 2: folha unica -> "1 + 8 bits" = 9 bits = 2 bytes. */
static void test_folha_unica(void)
{
    printf("Teste 2: folha unica (um unico byte distinto)\n");

    uint8_t entrada[20];
    memset(entrada, 'Z', sizeof(entrada));
    HuffNode *orig = arvore_do_bloco(entrada, sizeof(entrada));
    CHECK(orig != NULL && orig->is_leaf, "raiz deve ser folha unica");

    uint8_t *bytes = NULL;
    size_t   size  = 0;
    CHECK(tree_serialize(orig, &bytes, &size), "serialize da folha unica");
    CHECK(size == 2, "9 bits (1 + 8) devem ocupar 2 bytes apos o flush");

    HuffNode *recon = tree_deserialize(bytes, size);
    CHECK(recon != NULL && recon->is_leaf, "reconstruida deve ser folha");
    CHECK(recon->symbol == 'Z', "simbolo da folha preservado");
    CHECK(arvores_iguais(orig, recon), "folha unica identica");

    free(bytes);
    huffman_free_tree(orig);
    huffman_free_tree(recon);
}

/* Teste 3: todos os 256 valores de byte. */
static void test_todos_os_bytes(void)
{
    printf("Teste 3: todos os 256 valores de byte\n");

    uint8_t entrada[256];
    for (int i = 0; i < 256; i++)
        entrada[i] = (uint8_t)i;

    HuffNode *orig = arvore_do_bloco(entrada, sizeof(entrada));
    CHECK(orig != NULL, "arvore dos 256 bytes construida");

    uint8_t *bytes = NULL;
    size_t   size  = 0;
    CHECK(tree_serialize(orig, &bytes, &size), "serialize dos 256 bytes");

    /* 256 folhas + 255 nos internos = 511 nos.
     * folhas: 1 + 8 = 9 bits cada; internos: 1 bit cada.
     * total = 256*9 + 255 = 2559 bits -> ceil(2559/8) = 320 bytes. */
    CHECK(size == 320, "tamanho serializado esperado para 256 folhas");

    HuffNode *recon = tree_deserialize(bytes, size);
    CHECK(recon != NULL, "deserialize dos 256 bytes");
    CHECK(arvores_iguais(orig, recon), "arvore dos 256 bytes identica");

    HuffCode ta[256], tb[256];
    CHECK(huffman_build_codes(orig, ta), "codigos originais (256)");
    CHECK(huffman_build_codes(recon, tb), "codigos reconstruidos (256)");
    CHECK(tabelas_iguais(ta, tb), "tabelas de codigos iguais (256)");

    free(bytes);
    huffman_free_tree(orig);
    huffman_free_tree(recon);
}

/* Teste 4: robustez - serializacao truncada deve falhar sem crash. */
static void test_truncada(void)
{
    printf("Teste 4: serializacao truncada/corrompida -> NULL sem crash\n");

    const char *texto = "MISSISSIPPI";
    HuffNode *orig = arvore_do_bloco(texto, strlen(texto));

    uint8_t *bytes = NULL;
    size_t   size  = 0;
    CHECK(tree_serialize(orig, &bytes, &size), "serialize ok");
    CHECK(size > 1, "precisa de mais de 1 byte para truncar");

    /* Passa apenas o primeiro byte: faltam bits no meio da arvore. */
    HuffNode *recon = tree_deserialize(bytes, 1);
    CHECK(recon == NULL, "deserialize truncado deve retornar NULL");

    free(bytes);
    huffman_free_tree(orig);
}

/* Teste 5: bordas (NULL / tamanho 0). */
static void test_bordas(void)
{
    printf("Teste 5: bordas (NULL / size 0)\n");

    uint8_t *bytes = (uint8_t *)0x1; /* sentinela: deve virar NULL */
    size_t   size  = 999;
    CHECK(!tree_serialize(NULL, &bytes, &size), "root NULL deve falhar");
    CHECK(bytes == NULL && size == 0, "saidas zeradas no erro");

    CHECK(!tree_serialize((const HuffNode *)0x1, NULL, &size), "out_bytes NULL falha");

    CHECK(tree_deserialize(NULL, 10) == NULL, "bytes NULL -> NULL");

    uint8_t b = 0;
    CHECK(tree_deserialize(&b, 0) == NULL, "size 0 -> NULL");
}

int main(void)
{
    printf("=== Testes da serializacao da arvore (Modulo 7) ===\n");
    test_roundtrip_comum();
    test_folha_unica();
    test_todos_os_bytes();
    test_truncada();
    test_bordas();

    printf("\n%d verificacoes, %d falha(s).\n", tests_run, tests_failed);
    if (tests_failed == 0) {
        printf("TODOS OS TESTES PASSARAM.\n");
        return EXIT_SUCCESS;
    }
    printf("HOUVE FALHAS.\n");
    return EXIT_FAILURE;
}
