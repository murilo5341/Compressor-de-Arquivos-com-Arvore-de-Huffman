/*
 * test_block_compress.c - Testes unitarios da compressao de bloco (Modulo 6,
 * modularizacao.md / RULES REGRA 10).
 *
 * Como o descompressor formal so chega no Modulo 8, este teste inclui um
 * DECODIFICADOR MANUAL minimo (decodifica_bloco) que desce a arvore bit a bit
 * ate reconstruir 'original_size' bytes. Isso prova que o payload comprimido e
 * VALIDO (decodifica de volta ao bloco original), que e o criterio do modulo.
 *
 * Casos cobertos:
 *   1) bloco conhecido pequeno: saida valida + roundtrip;
 *   2) distribuicao enviesada: a compressao realmente REDUZ o tamanho;
 *   3) folha unica (um unico byte distinto repetido): 1 bit por byte;
 *   4) bloco vazio: resultado valido e vazio (tree NULL);
 *   5) todos os 256 valores de byte: roundtrip;
 *   6) bordas: NULL.
 *
 * Sai com codigo != 0 se qualquer verificacao falhar (para o `make test` falhar).
 */
#include "block.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitio.h"
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
 * Decodificador manual: desce a arvore (esquerda = 0, direita = 1) consumindo
 * bits do payload ate reconstruir 'original_size' bytes em 'out'. Trata a folha
 * unica (arvore com um unico no): cada byte gasta 1 bit e emite o simbolo da
 * raiz. Retorna 1 em sucesso, 0 se faltarem bits ou a arvore for inconsistente.
 */
static int decodifica_bloco(const BlockCompressed *bc, uint8_t *out)
{
    if (bc->original_size == 0)
        return 1; /* nada a decodificar */
    if (bc->tree == NULL)
        return 0;

    BitReader r;
    bit_reader_init(&r, bc->data, bc->compressed_size);

    for (size_t i = 0; i < bc->original_size; i++) {
        const HuffNode *node = bc->tree;

        if (node->is_leaf) {
            /* Folha unica: o codigo "0" gasta 1 bit por byte. */
            if (bit_reader_read_bit(&r) < 0)
                return 0;
            out[i] = node->symbol;
            continue;
        }

        while (!node->is_leaf) {
            int bit = bit_reader_read_bit(&r);
            if (bit < 0)
                return 0;
            node = bit ? node->right : node->left;
            if (node == NULL)
                return 0;
        }
        out[i] = node->symbol;
    }
    return 1;
}

/* Teste 1: bloco pequeno conhecido, com saida valida e roundtrip exato. */
static void test_bloco_conhecido(void)
{
    printf("Teste 1: bloco conhecido pequeno (saida valida + roundtrip)\n");

    const char *texto = "ABRACADABRA"; /* 11 bytes, distribuicao desigual */
    size_t len = strlen(texto);

    BlockCompressed bc;
    CHECK(block_compress(texto, len, &bc), "block_compress deve ter sucesso");
    CHECK(bc.original_size == len, "original_size deve igualar o tamanho de entrada");
    CHECK(bc.tree != NULL, "a arvore do bloco nao deve ser NULL");
    CHECK(bc.compressed_size > 0, "deve haver payload comprimido");
    CHECK(bc.data != NULL, "data nao deve ser NULL");

    uint8_t restaurado[32];
    CHECK(decodifica_bloco(&bc, restaurado), "decodificacao manual deve ter sucesso");
    CHECK(memcmp(restaurado, texto, len) == 0, "decodificado deve igualar o original");

    block_compressed_free(&bc);
    CHECK(bc.tree == NULL && bc.data == NULL, "free deve zerar a struct");
}

/* Teste 2: distribuicao enviesada -> a compressao realmente reduz o tamanho. */
static void test_compressao_reduz(void)
{
    printf("Teste 2: distribuicao enviesada reduz o tamanho\n");

    /* 1000 bytes: 'A' dominante e poucos outros simbolos -> codigos curtos. */
    uint8_t entrada[1000];
    for (int i = 0; i < 1000; i++)
        entrada[i] = (i % 16 == 0) ? (uint8_t)('A' + (i % 4)) : (uint8_t)'A';

    BlockCompressed bc;
    CHECK(block_compress(entrada, sizeof(entrada), &bc), "compress deve ter sucesso");
    CHECK(bc.compressed_size < bc.original_size,
          "payload deve ser menor que o original para dados enviesados");

    uint8_t *restaurado = (uint8_t *)malloc(sizeof(entrada));
    CHECK(restaurado != NULL, "malloc do restaurado");
    if (restaurado != NULL) {
        CHECK(decodifica_bloco(&bc, restaurado), "decodificacao deve ter sucesso");
        CHECK(memcmp(restaurado, entrada, sizeof(entrada)) == 0,
              "roundtrip deve bater byte a byte");
        free(restaurado);
    }

    block_compressed_free(&bc);
}

/* Teste 3: folha unica (um unico byte distinto repetido) -> 1 bit por byte. */
static void test_folha_unica(void)
{
    printf("Teste 3: folha unica (um unico simbolo repetido)\n");

    uint8_t entrada[50];
    memset(entrada, 'Z', sizeof(entrada));

    BlockCompressed bc;
    CHECK(block_compress(entrada, sizeof(entrada), &bc), "compress deve ter sucesso");
    CHECK(bc.tree != NULL && bc.tree->is_leaf, "a arvore deve ser uma folha unica");
    CHECK(bc.original_size == sizeof(entrada), "original_size correto");

    /* 50 bytes * 1 bit = 50 bits -> 7 bytes (56 bits, 6 de padding). */
    CHECK(bc.compressed_size == 7, "50 bits devem ocupar 7 bytes apos o flush");

    uint8_t restaurado[50];
    CHECK(decodifica_bloco(&bc, restaurado), "decodificacao deve ter sucesso");
    CHECK(memcmp(restaurado, entrada, sizeof(entrada)) == 0, "roundtrip da folha unica");

    block_compressed_free(&bc);
}

/* Teste 4: bloco vazio -> resultado valido e vazio. */
static void test_bloco_vazio(void)
{
    printf("Teste 4: bloco vazio\n");

    BlockCompressed bc;
    CHECK(block_compress(NULL, 0, &bc), "compress de bloco vazio deve ter sucesso");
    CHECK(bc.tree == NULL, "bloco vazio nao tem arvore");
    CHECK(bc.data == NULL, "bloco vazio nao tem payload");
    CHECK(bc.compressed_size == 0, "compressed_size deve ser 0");
    CHECK(bc.original_size == 0, "original_size deve ser 0");

    block_compressed_free(&bc); /* nao deve travar com struct vazia */
}

/* Teste 5: todos os 256 valores de byte presentes -> roundtrip. */
static void test_todos_os_bytes(void)
{
    printf("Teste 5: todos os 256 valores de byte\n");

    uint8_t entrada[256];
    for (int i = 0; i < 256; i++)
        entrada[i] = (uint8_t)i;

    BlockCompressed bc;
    CHECK(block_compress(entrada, sizeof(entrada), &bc), "compress deve ter sucesso");
    CHECK(bc.original_size == 256, "original_size deve ser 256");
    /* 256 simbolos equiprovaveis -> codigos de 8 bits -> ~256 bytes. */
    CHECK(bc.compressed_size > 0, "deve haver payload");

    uint8_t restaurado[256];
    CHECK(decodifica_bloco(&bc, restaurado), "decodificacao deve ter sucesso");
    CHECK(memcmp(restaurado, entrada, sizeof(entrada)) == 0,
          "roundtrip dos 256 bytes deve bater");

    block_compressed_free(&bc);
}

/* Teste 6: bordas (NULL). */
static void test_bordas(void)
{
    printf("Teste 6: bordas (NULL)\n");

    CHECK(!block_compress("X", 1, NULL), "out NULL deve falhar");
    block_compressed_free(NULL); /* nao deve travar */
}

int main(void)
{
    printf("=== Testes da compressao de bloco (Modulo 6) ===\n");
    test_bloco_conhecido();
    test_compressao_reduz();
    test_folha_unica();
    test_bloco_vazio();
    test_todos_os_bytes();
    test_bordas();

    printf("\n%d verificacoes, %d falha(s).\n", tests_run, tests_failed);
    if (tests_failed == 0) {
        printf("TODOS OS TESTES PASSARAM.\n");
        return EXIT_SUCCESS;
    }
    printf("HOUVE FALHAS.\n");
    return EXIT_FAILURE;
}
