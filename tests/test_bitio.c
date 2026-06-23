/*
 * test_bitio.c - Testes unitarios da escrita/leitura bit a bit (Modulo 5,
 * RULES REGRA 10).
 *
 * Cobre os casos obrigatorios e os contratos do modulo:
 *   1) roundtrip de uma sequencia que NAO termina em multiplo de 8 bits
 *      (escrever, ler de volta e conferir bit a bit);
 *   2) empacotamento MSB-first com bytes de valor conhecido (verifica a ordem);
 *   3) flush do byte parcial com padding de zeros (byte_count e conteudo);
 *   4) write_bits/read_bits com valores multi-bit (codigo de Huffman);
 *   5) bordas: fim de fluxo (-1), count 0, NULL;
 *   6) crescimento do buffer (muitos bits forcam realloc).
 *
 * Sai com codigo != 0 se qualquer verificacao falhar (para o `make test` falhar).
 */
#include "bitio.h"

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

/*
 * Teste 1: sequencia de 13 bits (nao multipla de 8). Escreve bit a bit, faz
 * flush, le de volta e confere que os 13 bits batem - e que o padding ocupou o
 * resto do segundo byte (13 bits -> 2 bytes).
 */
static void test_roundtrip_nao_alinhado(void)
{
    printf("Teste 1: roundtrip de sequencia nao alinhada (13 bits)\n");

    const int padrao[13] = { 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1 };

    BitWriter w;
    CHECK(bit_writer_init(&w), "bit_writer_init deve ter sucesso");

    for (int i = 0; i < 13; i++)
        bit_writer_write_bit(&w, padrao[i]);
    CHECK(bit_writer_flush(&w), "flush deve ter sucesso");

    /* 13 bits ocupam 2 bytes (8 + 5, o segundo com 3 bits de padding). */
    CHECK(w.byte_count == 2, "13 bits devem ocupar 2 bytes apos o flush");

    BitReader r;
    bit_reader_init(&r, w.buffer, w.byte_count);
    int ok = 1;
    for (int i = 0; i < 13; i++) {
        int bit = bit_reader_read_bit(&r);
        if (bit != padrao[i])
            ok = 0;
    }
    CHECK(ok, "os 13 bits lidos devem ser identicos aos escritos");

    /* Os bits restantes do 2o byte sao padding zero. */
    CHECK(bit_reader_read_bit(&r) == 0, "bit 14 (padding) deve ser 0");

    bit_writer_free(&w);
}

/*
 * Teste 2: empacotamento MSB-first. A sequencia 1,0,1,1,0 deve produzir o byte
 * 0xB0 (1011 0000): primeiro bit no MSB, padding de zeros a direita.
 */
static void test_ordem_msb_first(void)
{
    printf("Teste 2: empacotamento MSB-first (byte conhecido)\n");

    BitWriter w;
    CHECK(bit_writer_init(&w), "init");

    const int bits[5] = { 1, 0, 1, 1, 0 };
    for (int i = 0; i < 5; i++)
        bit_writer_write_bit(&w, bits[i]);
    bit_writer_flush(&w);

    CHECK(w.byte_count == 1, "5 bits -> 1 byte");
    CHECK(w.buffer[0] == 0xB0, "1,0,1,1,0 + padding deve formar 0xB0");

    bit_writer_free(&w);
}

/*
 * Teste 3: dois bytes cheios escritos via write_bits nao precisam de padding e
 * o conteudo deve casar exatamente. Tambem confere que sem byte parcial o flush
 * nao adiciona bytes.
 */
static void test_bytes_cheios(void)
{
    printf("Teste 3: bytes cheios e flush sem byte parcial\n");

    BitWriter w;
    CHECK(bit_writer_init(&w), "init");

    /* 0xCA = 1100 1010, 0x53 = 0101 0011: 16 bits exatos. */
    bit_writer_write_bits(&w, 0xCAu, 8);
    bit_writer_write_bits(&w, 0x53u, 8);

    CHECK(w.byte_count == 2, "16 bits ja sao 2 bytes completos antes do flush");
    CHECK(bit_writer_flush(&w), "flush sem byte parcial deve ter sucesso");
    CHECK(w.byte_count == 2, "flush nao deve adicionar byte quando nao ha parcial");
    CHECK(w.buffer[0] == 0xCA && w.buffer[1] == 0x53, "conteudo dos bytes deve casar");

    bit_writer_free(&w);
}

/*
 * Teste 4: write_bits/read_bits com um valor multi-bit (simula um codigo de
 * Huffman de 11 bits). O valor lido de volta deve ser igual ao escrito.
 */
static void test_write_read_bits(void)
{
    printf("Teste 4: write_bits / read_bits (codigo de 11 bits)\n");

    const uint64_t codigo = 0x5A3u; /* 101 1010 0011, 11 bits significativos */
    const unsigned tamanho = 11;

    BitWriter w;
    CHECK(bit_writer_init(&w), "init");
    bit_writer_write_bits(&w, codigo, tamanho);
    bit_writer_flush(&w);

    BitReader r;
    bit_reader_init(&r, w.buffer, w.byte_count);
    uint64_t lido = 0;
    CHECK(bit_reader_read_bits(&r, tamanho, &lido), "read_bits deve ter sucesso");
    CHECK(lido == codigo, "valor lido deve igualar o escrito (MSB-first)");

    bit_writer_free(&w);
}

/*
 * Teste 5: bordas. Ler alem do fim devolve -1; read_bits sem bits suficientes
 * falha; count 0 e no-op; APIs sao seguras contra NULL.
 */
static void test_bordas(void)
{
    printf("Teste 5: bordas (fim de fluxo, count 0, NULL)\n");

    BitWriter w;
    CHECK(bit_writer_init(&w), "init");
    bit_writer_write_bits(&w, 0x5u, 3); /* 3 bits */
    bit_writer_flush(&w);

    BitReader r;
    bit_reader_init(&r, w.buffer, w.byte_count);
    /* Le os 3 bits validos, depois os 5 de padding; o 9o ja e fim de fluxo. */
    for (int i = 0; i < 8; i++)
        (void)bit_reader_read_bit(&r);
    CHECK(bit_reader_read_bit(&r) == -1, "ler alem do fim deve devolver -1");

    /* read_bits pedindo mais bits do que existem deve falhar. */
    BitReader r2;
    bit_reader_init(&r2, w.buffer, w.byte_count);
    uint64_t v = 123;
    CHECK(!bit_reader_read_bits(&r2, 64, &v),
          "read_bits sem bits suficientes deve falhar");

    /* count 0 devolve 0 com sucesso e nao consome bits. */
    uint64_t z = 7;
    CHECK(bit_reader_read_bits(&r2, 0, &z) && z == 0, "read_bits(count=0) -> 0");

    /* Buffer vazio (NULL, 0): leitura imediata e fim de fluxo. */
    BitReader vazio;
    bit_reader_init(&vazio, NULL, 0);
    CHECK(bit_reader_read_bit(&vazio) == -1, "leitor vazio deve devolver -1");

    /* Seguranca contra NULL. */
    CHECK(!bit_writer_init(NULL), "init(NULL) deve falhar");
    CHECK(!bit_writer_write_bit(NULL, 1), "write_bit(NULL) deve falhar");
    CHECK(!bit_writer_write_bits(NULL, 0, 1), "write_bits(NULL) deve falhar");
    CHECK(!bit_writer_flush(NULL), "flush(NULL) deve falhar");
    CHECK(bit_reader_read_bit(NULL) == -1, "read_bit(NULL) deve devolver -1");
    bit_writer_free(NULL); /* nao deve travar */

    /* count > 64 deve falhar nas duas pontas. */
    CHECK(!bit_writer_write_bits(&w, 0, 65), "write_bits count>64 deve falhar");
    uint64_t qualquer;
    CHECK(!bit_reader_read_bits(&r2, 65, &qualquer), "read_bits count>64 deve falhar");

    bit_writer_free(&w);
}

/*
 * Teste 6: crescimento do buffer. Escrevemos 10000 bits com padrao deterministico
 * (forca varios reallocs), lemos de volta e conferimos todos. Garante que o
 * empacotamento se mantem correto atravessando fronteiras de byte e de realloc.
 */
static void test_crescimento_buffer(void)
{
    printf("Teste 6: crescimento do buffer (10000 bits)\n");

    const int total = 10000;

    BitWriter w;
    CHECK(bit_writer_init(&w), "init");
    for (int i = 0; i < total; i++)
        bit_writer_write_bit(&w, (i * 7 + 3) & 1); /* padrao deterministico */
    CHECK(bit_writer_flush(&w), "flush apos muitos bits");
    CHECK(!w.error, "nao deve haver erro de alocacao");

    /* 10000 bits -> 1250 bytes exatos (multiplo de 8, sem padding). */
    CHECK(w.byte_count == 1250, "10000 bits devem ocupar 1250 bytes");

    BitReader r;
    bit_reader_init(&r, w.buffer, w.byte_count);
    int ok = 1;
    for (int i = 0; i < total; i++) {
        int esperado = (i * 7 + 3) & 1;
        if (bit_reader_read_bit(&r) != esperado)
            ok = 0;
    }
    CHECK(ok, "todos os 10000 bits devem ser lidos corretamente");

    bit_writer_free(&w);
}

int main(void)
{
    printf("=== Testes da escrita/leitura bit a bit (Modulo 5) ===\n");
    test_roundtrip_nao_alinhado();
    test_ordem_msb_first();
    test_bytes_cheios();
    test_write_read_bits();
    test_bordas();
    test_crescimento_buffer();

    printf("\n%d verificacoes, %d falha(s).\n", tests_run, tests_failed);
    if (tests_failed == 0) {
        printf("TODOS OS TESTES PASSARAM.\n");
        return EXIT_SUCCESS;
    }
    printf("HOUVE FALHAS.\n");
    return EXIT_FAILURE;
}
