/*
 * test_block_decompress.c - Testes da descompressao de bloco (Modulo 8,
 * modularizacao.md / RULES REGRA 10).
 *
 * Criterio do modulo: comprimir um bloco -> descomprimir -> comparar com o
 * original. Aqui o roundtrip e fechado de verdade: block_compress (Modulo 6)
 * gera tree + payload, e block_decompress (Modulo 8) reconstroi os bytes.
 *
 * Casos cobertos:
 *   1) bloco conhecido pequeno ("ABRACADABRA"): roundtrip exato;
 *   2) folha unica (um unico byte distinto repetido);
 *   3) bloco vazio: sucesso com *out NULL;
 *   4) todos os 256 valores de byte;
 *   5) bloco maior enviesado (1000 bytes);
 *   6) robustez: payload truncado -> false sem crash;
 *   7) bordas: out NULL, tree NULL com original_size > 0.
 *
 * Sai com codigo != 0 se qualquer verificacao falhar (para o `make test` falhar).
 */
#include "block.h"

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

/* Comprime 'buf' e descomprime de volta; verifica igualdade byte a byte. */
static void roundtrip(const void *buf, size_t len, const char *nome)
{
    BlockCompressed bc;
    CHECK(block_compress(buf, len, &bc), nome);

    uint8_t *saida = NULL;
    CHECK(block_decompress(bc.tree, bc.data, bc.compressed_size,
                           bc.original_size, &saida),
          "block_decompress deve ter sucesso");

    if (len == 0) {
        CHECK(saida == NULL, "bloco vazio -> saida NULL");
    } else {
        CHECK(saida != NULL, "saida nao deve ser NULL");
        if (saida != NULL)
            CHECK(memcmp(saida, buf, len) == 0, "roundtrip deve bater byte a byte");
    }

    free(saida);
    block_compressed_free(&bc);
}

/* Teste 1: bloco conhecido pequeno. */
static void test_bloco_conhecido(void)
{
    printf("Teste 1: bloco conhecido (ABRACADABRA) roundtrip\n");
    const char *texto = "ABRACADABRA";
    roundtrip(texto, strlen(texto), "compress de ABRACADABRA");
}

/* Teste 2: folha unica. */
static void test_folha_unica(void)
{
    printf("Teste 2: folha unica (um unico simbolo repetido)\n");
    uint8_t entrada[50];
    memset(entrada, 'Z', sizeof(entrada));
    roundtrip(entrada, sizeof(entrada), "compress de folha unica");
}

/* Teste 3: bloco vazio. */
static void test_bloco_vazio(void)
{
    printf("Teste 3: bloco vazio\n");
    roundtrip(NULL, 0, "compress de bloco vazio");

    /* Chamada direta tambem deve dar sucesso com *out NULL. */
    uint8_t *saida = (uint8_t *)0x1;
    CHECK(block_decompress(NULL, NULL, 0, 0, &saida), "decompress vazio direto");
    CHECK(saida == NULL, "saida do bloco vazio deve ser NULL");
}

/* Teste 4: todos os 256 valores de byte. */
static void test_todos_os_bytes(void)
{
    printf("Teste 4: todos os 256 valores de byte\n");
    uint8_t entrada[256];
    for (int i = 0; i < 256; i++)
        entrada[i] = (uint8_t)i;
    roundtrip(entrada, sizeof(entrada), "compress dos 256 bytes");
}

/* Teste 5: bloco maior enviesado. */
static void test_bloco_grande(void)
{
    printf("Teste 5: bloco maior enviesado (1000 bytes)\n");
    uint8_t entrada[1000];
    for (int i = 0; i < 1000; i++)
        entrada[i] = (i % 16 == 0) ? (uint8_t)('A' + (i % 4)) : (uint8_t)'A';
    roundtrip(entrada, sizeof(entrada), "compress de 1000 bytes enviesados");
}

/* Teste 6: payload truncado -> false sem crash. */
static void test_truncado(void)
{
    printf("Teste 6: payload truncado -> false sem crash\n");

    const char *texto = "MISSISSIPPI"; /* gera payload com mais de 1 byte */
    BlockCompressed bc;
    CHECK(block_compress(texto, strlen(texto), &bc), "compress de MISSISSIPPI");
    CHECK(bc.compressed_size > 1, "payload precisa de mais de 1 byte para truncar");

    uint8_t *saida = (uint8_t *)0x1;
    /* So 1 byte de payload: faltam bits antes de reconstruir original_size. */
    CHECK(!block_decompress(bc.tree, bc.data, 1, bc.original_size, &saida),
          "payload truncado deve falhar");
    CHECK(saida == NULL, "saida deve ser NULL no erro");

    block_compressed_free(&bc);
}

/* Teste 7: bordas. */
static void test_bordas(void)
{
    printf("Teste 7: bordas (out NULL, tree NULL com dados)\n");

    uint8_t dummy = 0;
    CHECK(!block_decompress(NULL, &dummy, 1, 5, NULL), "out NULL deve falhar");

    uint8_t *saida = (uint8_t *)0x1;
    CHECK(!block_decompress(NULL, &dummy, 1, 5, &saida),
          "tree NULL com original_size > 0 deve falhar");
    CHECK(saida == NULL, "saida deve ser NULL no erro");
}

int main(void)
{
    printf("=== Testes da descompressao de bloco (Modulo 8) ===\n");
    test_bloco_conhecido();
    test_folha_unica();
    test_bloco_vazio();
    test_todos_os_bytes();
    test_bloco_grande();
    test_truncado();
    test_bordas();

    printf("\n%d verificacoes, %d falha(s).\n", tests_run, tests_failed);
    if (tests_failed == 0) {
        printf("TODOS OS TESTES PASSARAM.\n");
        return EXIT_SUCCESS;
    }
    printf("HOUVE FALHAS.\n");
    return EXIT_FAILURE;
}
