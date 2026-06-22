/*
 * test_crc32.c - Testes unitarios do CRC32 por bloco (Modulo 2, RULES REGRA 10).
 *
 * Cobre os casos obrigatorios:
 *   1) CRC32 de buffer vazio (e ponteiro NULL com len 0);
 *   2) CRC32 de strings conhecidas com valor esperado fixo (vetores publicos);
 *   3) CRC32 muda quando um unico byte e alterado;
 *   4) equivalencia entre o calculo de uma vez e o incremental (uso por bloco).
 *
 * Sai com codigo != 0 se qualquer verificacao falhar (para o `make test` falhar).
 */
#include "crc32.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/* Teste 1: buffer vazio tem CRC32 0x00000000 (init ^ xorout, sem dados). */
static void test_buffer_vazio(void)
{
    printf("Teste 1: CRC32 de buffer vazio\n");
    CHECK(crc32_buffer(NULL, 0) == 0x00000000u,
          "CRC32 de (NULL, 0) deve ser 0");
    CHECK(crc32_buffer("", 0) == 0x00000000u,
          "CRC32 de string vazia deve ser 0");
}

/* Teste 2: strings conhecidas batem com os vetores publicos do CRC-32/ISO-HDLC. */
static void test_valores_conhecidos(void)
{
    printf("Teste 2: CRC32 de strings conhecidas (valores fixos)\n");

    /* Vetor de teste canonico do CRC-32: "123456789" -> 0xCBF43926. */
    CHECK(crc32_buffer("123456789", 9) == 0xCBF43926u,
          "CRC32(\"123456789\") deve ser 0xCBF43926");

    /* Frase classica de teste -> 0x414FA339. */
    const char *frase = "The quick brown fox jumps over the lazy dog";
    CHECK(crc32_buffer(frase, strlen(frase)) == 0x414FA339u,
          "CRC32 da frase do raposo deve ser 0x414FA339");
}

/* Teste 3: alterar um unico byte muda o CRC32. */
static void test_sensibilidade_byte(void)
{
    printf("Teste 3: CRC32 muda ao alterar um byte\n");

    unsigned char bloco_a[] = { 'b', 'l', 'o', 'c', 'o', '!' };
    unsigned char bloco_b[sizeof(bloco_a)];
    memcpy(bloco_b, bloco_a, sizeof(bloco_a));
    bloco_b[3] = 'C'; /* altera apenas 1 byte ('c' -> 'C') */

    uint32_t crc_a = crc32_buffer(bloco_a, sizeof(bloco_a));
    uint32_t crc_b = crc32_buffer(bloco_b, sizeof(bloco_b));
    CHECK(crc_a != crc_b,
          "alterar um byte deve mudar o CRC32 (deteccao de corrupcao)");
}

/* Teste 4: calcular de uma vez == calcular em pedacos (uso por bloco). */
static void test_equivalencia_incremental(void)
{
    printf("Teste 4: calculo de uma vez equivale ao incremental\n");

    const char *dados = "compressor de arquivos com arvore de huffman";
    size_t n = strlen(dados);

    uint32_t de_uma_vez = crc32_buffer(dados, n);

    /* Mesmo conteudo, processado em tres pedacos consecutivos. */
    size_t corte1 = 10;
    size_t corte2 = 25;
    uint32_t c = crc32_init();
    c = crc32_update(c, dados, corte1);
    c = crc32_update(c, dados + corte1, corte2 - corte1);
    c = crc32_update(c, dados + corte2, n - corte2);
    uint32_t incremental = crc32_finalize(c);

    CHECK(de_uma_vez == incremental,
          "CRC32 em pedacos deve igualar o CRC32 de uma vez");

    /* Atualizar com (NULL, 0) ou len 0 nao altera o estado. */
    uint32_t c2 = crc32_init();
    c2 = crc32_update(c2, NULL, 0);
    c2 = crc32_update(c2, dados, n);
    c2 = crc32_update(c2, dados, 0);
    CHECK(crc32_finalize(c2) == de_uma_vez,
          "update com len 0 / NULL nao deve alterar o CRC32");
}

int main(void)
{
    printf("=== Testes do CRC32 por bloco (Modulo 2) ===\n");
    test_buffer_vazio();
    test_valores_conhecidos();
    test_sensibilidade_byte();
    test_equivalencia_incremental();

    printf("\n%d verificacoes, %d falha(s).\n", tests_run, tests_failed);
    if (tests_failed == 0) {
        printf("TODOS OS TESTES PASSARAM.\n");
        return EXIT_SUCCESS;
    }
    printf("HOUVE FALHAS.\n");
    return EXIT_FAILURE;
}
