/*
 * test_format.c - Testes do formato .cz (Modulo 9, modularizacao.md / RULES
 * REGRA 10).
 *
 * Verifica a (de)serializacao dos cabecalhos do arquivo .cz e as premissas de
 * design do formato: endianess little-endian documentada, validacao de magic e
 * versao, roundtrip dos campos e capacidade de SALTAR um bloco (corrompido ou
 * nao) usando tree_size + compressed_size.
 *
 * Os testes usam tmpfile() (C padrao, portavel) como arquivo .cz em disco
 * temporario: escreve-se, rebobina-se (rewind) e le-se de volta.
 *
 * Casos cobertos:
 *   1) roundtrip do cabecalho global (campos batem);
 *   2) layout little-endian do cabecalho global (bytes crus conhecidos);
 *   3) roundtrip do cabecalho de bloco;
 *   4) magic invalido -> cz_read_global_header falha;
 *   5) versao incompativel -> cz_read_global_header falha;
 *   6) salto de bloco: cz_skip_block_payload posiciona no proximo cabecalho;
 *   7) bordas: NULL e arquivo truncado.
 *
 * Sai com codigo != 0 se qualquer verificacao falhar (para o `make test` falhar).
 */
#include "format.h"

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

int main(void)
{
    printf("=== Testes do formato .cz (Modulo 9) ===\n");

    /* ---------------------------------------------------------------- */
    printf("Teste 1: roundtrip do cabecalho global\n");
    {
        FILE *f = tmpfile();
        CHECK(f != NULL, "tmpfile deve abrir");
        if (f != NULL) {
            CzGlobalHeader out;
            cz_global_header_init(&out, 65536u, 42u);
            CHECK(cz_write_global_header(f, &out), "escrita do global deve ter sucesso");

            rewind(f);
            CzGlobalHeader in;
            CHECK(cz_read_global_header(f, &in), "leitura do global deve ter sucesso");
            CHECK(memcmp(in.magic, CZ_MAGIC, CZ_MAGIC_SIZE) == 0, "magic deve ser CZHF");
            CHECK(in.version == CZ_FORMAT_VERSION, "versao deve casar");
            CHECK(in.block_size == 65536u, "block_size deve casar");
            CHECK(in.block_count == 42u, "block_count deve casar");
            fclose(f);
        }
    }

    /* ---------------------------------------------------------------- */
    printf("Teste 2: layout little-endian do cabecalho global\n");
    {
        FILE *f = tmpfile();
        CHECK(f != NULL, "tmpfile deve abrir");
        if (f != NULL) {
            CzGlobalHeader out;
            /* version=1, block_size=0x04030201, block_count=0x0807060504030201 */
            cz_global_header_init(&out, 0x04030201u, 0x0807060504030201ULL);
            CHECK(cz_write_global_header(f, &out), "escrita deve ter sucesso");

            rewind(f);
            uint8_t raw[CZ_GLOBAL_HEADER_SIZE];
            CHECK(fread(raw, 1, sizeof raw, f) == sizeof raw, "deve ler 20 bytes crus");

            /* magic */
            CHECK(raw[0] == 'C' && raw[1] == 'Z' && raw[2] == 'H' && raw[3] == 'F',
                  "magic em bytes deve ser 'CZHF'");
            /* version = 1 -> 01 00 00 00 */
            CHECK(raw[4] == 0x01 && raw[5] == 0x00 && raw[6] == 0x00 && raw[7] == 0x00,
                  "version little-endian");
            /* block_size = 0x04030201 -> 01 02 03 04 */
            CHECK(raw[8] == 0x01 && raw[9] == 0x02 && raw[10] == 0x03 && raw[11] == 0x04,
                  "block_size little-endian");
            /* block_count = 0x0807060504030201 -> 01 02 03 04 05 06 07 08 */
            CHECK(raw[12] == 0x01 && raw[13] == 0x02 && raw[14] == 0x03 && raw[15] == 0x04 &&
                  raw[16] == 0x05 && raw[17] == 0x06 && raw[18] == 0x07 && raw[19] == 0x08,
                  "block_count little-endian");
            fclose(f);
        }
    }

    /* ---------------------------------------------------------------- */
    printf("Teste 3: roundtrip do cabecalho de bloco\n");
    {
        FILE *f = tmpfile();
        CHECK(f != NULL, "tmpfile deve abrir");
        if (f != NULL) {
            CzBlockHeader out = {
                .block_index     = 7u,
                .original_size   = 65536u,
                .compressed_size = 40000u,
                .tree_size       = 320u,
                .original_crc32  = 0xCBF43926u
            };
            CHECK(cz_write_block_header(f, &out), "escrita do bloco deve ter sucesso");

            rewind(f);
            CzBlockHeader in;
            CHECK(cz_read_block_header(f, &in), "leitura do bloco deve ter sucesso");
            CHECK(in.block_index == 7u, "block_index deve casar");
            CHECK(in.original_size == 65536u, "original_size deve casar");
            CHECK(in.compressed_size == 40000u, "compressed_size deve casar");
            CHECK(in.tree_size == 320u, "tree_size deve casar");
            CHECK(in.original_crc32 == 0xCBF43926u, "crc32 deve casar");
            fclose(f);
        }
    }

    /* ---------------------------------------------------------------- */
    printf("Teste 4: magic invalido -> leitura falha\n");
    {
        FILE *f = tmpfile();
        CHECK(f != NULL, "tmpfile deve abrir");
        if (f != NULL) {
            /* Escreve 20 bytes com magic errado. */
            uint8_t bogus[CZ_GLOBAL_HEADER_SIZE] = { 'X', 'Z', 'H', 'F' };
            CHECK(fwrite(bogus, 1, sizeof bogus, f) == sizeof bogus, "escreve lixo");
            rewind(f);
            CzGlobalHeader in;
            CHECK(!cz_read_global_header(f, &in), "magic invalido deve falhar");
            fclose(f);
        }
    }

    /* ---------------------------------------------------------------- */
    printf("Teste 5: versao incompativel -> leitura falha\n");
    {
        FILE *f = tmpfile();
        CHECK(f != NULL, "tmpfile deve abrir");
        if (f != NULL) {
            CzGlobalHeader out;
            cz_global_header_init(&out, 1024u, 1u);
            out.version = CZ_FORMAT_VERSION + 1u;  /* versao futura desconhecida */
            CHECK(cz_write_global_header(f, &out), "escreve com versao adulterada");
            rewind(f);
            CzGlobalHeader in;
            CHECK(!cz_read_global_header(f, &in), "versao incompativel deve falhar");
            fclose(f);
        }
    }

    /* ---------------------------------------------------------------- */
    printf("Teste 6: salto de bloco (cz_skip_block_payload)\n");
    {
        FILE *f = tmpfile();
        CHECK(f != NULL, "tmpfile deve abrir");
        if (f != NULL) {
            /* Bloco 0: cabecalho + tree(3 bytes) + payload(5 bytes), depois
             * bloco 1: cabecalho. Pulamos o conteudo do bloco 0 e lemos o
             * cabecalho do bloco 1, simulando o salto de um bloco corrompido. */
            CzBlockHeader b0 = {
                .block_index = 0u, .original_size = 8u, .compressed_size = 5u,
                .tree_size = 3u, .original_crc32 = 0x11111111u
            };
            CzBlockHeader b1 = {
                .block_index = 1u, .original_size = 4u, .compressed_size = 2u,
                .tree_size = 2u, .original_crc32 = 0x22222222u
            };
            uint8_t tree0[3]    = { 0xAA, 0xBB, 0xCC };
            uint8_t payload0[5] = { 0x01, 0x02, 0x03, 0x04, 0x05 };

            CHECK(cz_write_block_header(f, &b0), "escreve cabecalho b0");
            CHECK(fwrite(tree0, 1, sizeof tree0, f) == sizeof tree0, "escreve tree b0");
            CHECK(fwrite(payload0, 1, sizeof payload0, f) == sizeof payload0, "escreve payload b0");
            CHECK(cz_write_block_header(f, &b1), "escreve cabecalho b1");

            rewind(f);
            CzBlockHeader r0;
            CHECK(cz_read_block_header(f, &r0), "le cabecalho b0");
            CHECK(r0.block_index == 0u, "b0 index");
            CHECK(cz_skip_block_payload(f, &r0), "salta conteudo de b0");
            CzBlockHeader r1;
            CHECK(cz_read_block_header(f, &r1), "le cabecalho b1 apos salto");
            CHECK(r1.block_index == 1u, "b1 index correto apos salto");
            CHECK(r1.tree_size == 2u, "b1 tree_size correto apos salto");
            CHECK(r1.original_crc32 == 0x22222222u, "b1 crc correto apos salto");
            fclose(f);
        }
    }

    /* ---------------------------------------------------------------- */
    printf("Teste 7: bordas (NULL e truncado)\n");
    {
        CzGlobalHeader gh;
        CzBlockHeader  bh;
        cz_global_header_init(NULL, 1u, 1u);  /* nao deve crashar */
        CHECK(!cz_write_global_header(NULL, &gh), "write global com FILE NULL falha");
        CHECK(!cz_read_global_header(NULL, &gh), "read global com FILE NULL falha");
        CHECK(!cz_write_block_header(NULL, &bh), "write bloco com FILE NULL falha");
        CHECK(!cz_read_block_header(NULL, &bh), "read bloco com FILE NULL falha");
        CHECK(!cz_skip_block_payload(NULL, &bh), "skip com FILE NULL falha");

        /* Arquivo truncado: so 3 bytes, leitura do global deve falhar. */
        FILE *f = tmpfile();
        CHECK(f != NULL, "tmpfile deve abrir");
        if (f != NULL) {
            uint8_t curto[3] = { 'C', 'Z', 'H' };
            CHECK(fwrite(curto, 1, sizeof curto, f) == sizeof curto, "escreve 3 bytes");
            rewind(f);
            CHECK(!cz_read_global_header(f, &gh), "global truncado deve falhar");
            fclose(f);
        }

        /* Cabecalho de bloco truncado. */
        FILE *g = tmpfile();
        CHECK(g != NULL, "tmpfile deve abrir");
        if (g != NULL) {
            uint8_t curto[10] = { 0 };
            CHECK(fwrite(curto, 1, sizeof curto, g) == sizeof curto, "escreve 10 bytes");
            rewind(g);
            CHECK(!cz_read_block_header(g, &bh), "bloco truncado deve falhar");
            fclose(g);
        }
    }

    printf("\n%d verificacoes, %d falha(s).\n", tests_run, tests_failed);
    if (tests_failed == 0) {
        printf("TODOS OS TESTES PASSARAM.\n");
        return 0;
    }
    printf("HOUVE FALHAS.\n");
    return 1;
}
