/*
 * main_cunzip.c - Descompressor sequencial "cunzip" (Modulo 11).
 *
 * Le um arquivo .cz produzido pelo czip (Modulo 10) e reconstroi o arquivo
 * original. Esta e a versao SEQUENCIAL (sem threads); o pipeline concorrente
 * fica nos Modulos 12-14. Fecha o lado da LEITURA do formato .cz, completando o
 * roundtrip byte a byte (czip -> cunzip -> cmp).
 *
 * Fluxo (modularizacao.md, Modulo 11):
 *   1) abrir o .cz e o arquivo de saida;
 *   2) ler e VALIDAR o cabecalho global (magic "CZHF" + versao) (Modulo 9);
 *   3) para cada um dos block_count blocos:
 *        a) ler o cabecalho do bloco                              (Modulo 9);
 *        b) ler a arvore serializada e reconstrui-la              (Modulo 7);
 *        c) ler o payload comprimido e descomprimir ate
 *           original_size bytes                                   (Modulo 8);
 *        d) recalcular o CRC32 do bloco restaurado e comparar com
 *           o gravado no cabecalho                                (Modulo 2);
 *        e) se valido, gravar o bloco na saida; se corrompido,
 *           reportar e PULAR para o proximo bloco.
 *
 * Robustez (RULES REGRA 5 / formato do Modulo 9): a posicao do proximo bloco e
 * sempre recalculada a partir do cabecalho (offset do payload + tree_size +
 * compressed_size). Assim, um bloco corrompido (CRC divergente, arvore ou
 * payload invalido) e detectado e PULADO sem perder a sincronizacao do arquivo:
 * os demais blocos continuam sendo descomprimidos.
 *
 * Codigo de saida: 0 quando todos os blocos foram restaurados; 1 em erro fatal
 * (nao abrir arquivos, magic/versao invalidos, cabecalho truncado ou falha de
 * escrita na saida) ou quando algum bloco foi detectado como corrompido (saida
 * parcial gravada, com aviso no stderr).
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "crc32.h"
#include "format.h"
#include "tree_serialization.h"

/* Opcoes da linha de comando do cunzip. */
typedef struct {
    const char *input_path;   /* arquivo .cz de entrada */
    const char *output_path;  /* arquivo restaurado de saida */
} CunzipOptions;

static void print_usage(const char *prog)
{
    fprintf(stderr, "Uso: %s <entrada.cz> <saida>\n", prog);
}

/*
 * Faz o parsing dos argumentos para 'opt': apenas os dois posicionais
 * <entrada.cz> <saida>. Retorna true em sucesso.
 */
static bool parse_args(int argc, char **argv, CunzipOptions *opt)
{
    opt->input_path  = NULL;
    opt->output_path = NULL;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (opt->input_path == NULL) {
            opt->input_path = arg;
        } else if (opt->output_path == NULL) {
            opt->output_path = arg;
        } else {
            fprintf(stderr, "cunzip: argumento inesperado: %s\n", arg);
            return false;
        }
    }

    if (opt->input_path == NULL || opt->output_path == NULL) {
        return false;
    }
    return true;
}

/* Le exatamente 'n' bytes de 'f' para 'buf'. Retorna true se leu todos. */
static bool read_exact(FILE *f, void *buf, size_t n)
{
    if (n == 0) {
        return true;
    }
    return fread(buf, 1, n, f) == n;
}

/*
 * Processa UM bloco ja com o cabecalho 'bh' lido: le a arvore serializada e o
 * payload comprimido (a partir da posicao atual de 'in'), reconstroi a arvore
 * (Modulo 7), descomprime (Modulo 8), valida o CRC32 do conteudo restaurado
 * (Modulo 2) e, se valido, grava os 'original_size' bytes em 'out'.
 *
 * Retorna true se o bloco foi restaurado e gravado com sucesso. Retorna false
 * se o bloco esta corrompido/ilegivel (arvore ou payload invalidos, CRC
 * divergente, leitura curta ou falta de memoria) - o chamador reporta e segue.
 *
 * Em caso de falha de ESCRITA na saida (ex.: disco cheio), '*write_error' vira
 * true e o chamador deve abortar (erro fatal, nao recuperavel pulando o bloco).
 *
 * A posicao de 'in' ao retornar NAO e garantida; o chamador reposiciona no
 * inicio do proximo bloco usando os tamanhos do cabecalho.
 */
static bool decompress_one_block(FILE *in, const CzBlockHeader *bh, FILE *out,
                                 bool *write_error)
{
    /* Bloco de conteudo vazio: nada a gravar; valida o CRC do vazio (0). */
    if (bh->original_size == 0) {
        return crc32_buffer(NULL, 0) == bh->original_crc32;
    }

    uint8_t *tree_bytes = NULL;
    uint8_t *payload     = NULL;
    uint8_t *restored    = NULL;
    HuffNode *tree       = NULL;
    bool      ok         = false;

    /* Le a arvore serializada do bloco. */
    if (bh->tree_size == 0) {
        goto cleanup;  /* bloco nao-vazio precisa de arvore */
    }
    tree_bytes = malloc(bh->tree_size);
    if (tree_bytes == NULL || !read_exact(in, tree_bytes, bh->tree_size)) {
        goto cleanup;
    }

    /* Le o payload comprimido do bloco. */
    if (bh->compressed_size > 0) {
        payload = malloc(bh->compressed_size);
        if (payload == NULL || !read_exact(in, payload, bh->compressed_size)) {
            goto cleanup;
        }
    }

    /* Reconstroi a arvore (Modulo 7) e descomprime (Modulo 8). */
    tree = tree_deserialize(tree_bytes, bh->tree_size);
    if (tree == NULL) {
        goto cleanup;
    }
    if (!block_decompress(tree, payload, bh->compressed_size,
                          bh->original_size, &restored)) {
        goto cleanup;
    }

    /* Valida a integridade do conteudo restaurado (Modulo 2). */
    if (crc32_buffer(restored, bh->original_size) != bh->original_crc32) {
        goto cleanup;  /* CRC divergente -> bloco corrompido */
    }

    /* Bloco valido: grava na saida. */
    if (fwrite(restored, 1, bh->original_size, out) != bh->original_size) {
        if (write_error != NULL) {
            *write_error = true;
        }
        goto cleanup;
    }
    ok = true;

cleanup:
    free(tree_bytes);
    free(payload);
    free(restored);
    huffman_free_tree(tree);
    return ok;
}

/*
 * Executa a descompressao completa de opt->input_path para opt->output_path.
 * Retorna 0 em sucesso, 1 em erro fatal ou se houver bloco(s) corrompido(s).
 */
static int decompress_file(const CunzipOptions *opt)
{
    FILE *in = fopen(opt->input_path, "rb");
    if (in == NULL) {
        fprintf(stderr, "cunzip: nao foi possivel abrir a entrada '%s'.\n",
                opt->input_path);
        return 1;
    }

    CzGlobalHeader gh;
    if (!cz_read_global_header(in, &gh)) {
        fprintf(stderr,
                "cunzip: '%s' nao e um arquivo .cz valido (magic/versao).\n",
                opt->input_path);
        fclose(in);
        return 1;
    }

    FILE *out = fopen(opt->output_path, "wb");
    if (out == NULL) {
        fprintf(stderr, "cunzip: nao foi possivel criar a saida '%s'.\n",
                opt->output_path);
        fclose(in);
        return 1;
    }

    int rc        = 0;  /* 0 = ok; 1 = corrompido(s)/fatal */
    uint64_t corrupted = 0;

    for (uint64_t i = 0; i < gh.block_count; i++) {
        CzBlockHeader bh;
        if (!cz_read_block_header(in, &bh)) {
            fprintf(stderr,
                    "cunzip: arquivo truncado - esperava %llu blocos, parou no %llu.\n",
                    (unsigned long long)gh.block_count, (unsigned long long)i);
            rc = 1;
            break;
        }

        /* Posicao do inicio do payload (apos o cabecalho do bloco) e do inicio
         * do PROXIMO bloco, calculada a partir dos tamanhos do cabecalho. Isso
         * garante a sincronizacao mesmo quando o bloco atual esta corrompido. */
        long payload_start = ftell(in);
        if (payload_start < 0) {
            fprintf(stderr, "cunzip: falha ao localizar o payload do bloco %llu.\n",
                    (unsigned long long)i);
            rc = 1;
            break;
        }
        long next_block = payload_start + (long)bh.tree_size +
                          (long)bh.compressed_size;

        bool write_error = false;
        bool block_ok = decompress_one_block(in, &bh, out, &write_error);

        if (write_error) {
            fprintf(stderr, "cunzip: falha ao gravar o bloco %llu na saida.\n",
                    (unsigned long long)i);
            rc = 1;
            break;
        }
        if (!block_ok) {
            fprintf(stderr,
                    "cunzip: bloco %llu corrompido (descartado); seguindo para o proximo.\n",
                    (unsigned long long)bh.block_index);
            corrupted++;
            rc = 1;
        }

        /* Resincroniza no inicio do proximo bloco (salto de bloco corrompido). */
        if (fseek(in, next_block, SEEK_SET) != 0) {
            fprintf(stderr, "cunzip: falha ao avancar para o bloco %llu.\n",
                    (unsigned long long)(i + 1));
            rc = 1;
            break;
        }
    }

    fclose(in);
    if (fclose(out) != 0 && rc == 0) {
        fprintf(stderr, "cunzip: falha ao finalizar a saida '%s'.\n",
                opt->output_path);
        rc = 1;
    }

    if (corrupted > 0) {
        fprintf(stderr, "cunzip: %llu bloco(s) corrompido(s) descartado(s).\n",
                (unsigned long long)corrupted);
    } else if (rc == 0) {
        printf("cunzip: '%s' -> '%s' (%llu bloco(s)).\n",
               opt->input_path, opt->output_path,
               (unsigned long long)gh.block_count);
    }
    return rc;
}

int main(int argc, char **argv)
{
    CunzipOptions opt;
    if (!parse_args(argc, argv, &opt)) {
        print_usage(argv[0]);
        return 1;
    }
    return decompress_file(&opt);
}
