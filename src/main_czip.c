/*
 * main_czip.c - Compressor sequencial "czip" (Modulo 10).
 *
 * Le um arquivo de entrada, divide o conteudo em BLOCOS independentes de
 * tamanho fixo (--block-size) e grava um arquivo .cz comprimido com Huffman.
 * Esta e a versao SEQUENCIAL (sem threads): o pipeline concorrente entra nos
 * Modulos 12-14. Mesmo assim, a CLI ja aceita --threads para nao retrabalhar o
 * parsing depois (RULES REGRA 5/7) - na versao sequencial o valor e validado e
 * ignorado.
 *
 * Fluxo (modularizacao.md, Modulo 10):
 *   1) abrir arquivo de entrada e arquivo de saida .cz;
 *   2) gravar o cabecalho global (com block_count provisorio = 0);
 *   3) para cada bloco lido (ate --block-size bytes):
 *        a) calcular o CRC32 do conteudo ORIGINAL do bloco       (Modulo 2);
 *        b) comprimir o bloco com Huffman                        (Modulo 6);
 *        c) serializar a arvore do bloco                         (Modulo 7);
 *        d) gravar cabecalho do bloco + arvore + payload no .cz  (Modulo 9);
 *   4) ao fim, voltar ao inicio e reescrever o cabecalho global com a
 *      quantidade REAL de blocos.
 *
 * Por que block_count e reescrito no fim: so conhecemos a quantidade de blocos
 * depois de ler o arquivo inteiro. Em vez de exigir o tamanho do arquivo
 * adiantado (nem todo arquivo e "stat-avel" de forma portavel), gravamos um
 * placeholder, comprimimos contando os blocos e voltamos com fseek para
 * corrigir o campo. O arquivo de saida e regular (seekable), entao isso e
 * seguro e simples de explicar na defesa.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "crc32.h"
#include "format.h"
#include "tree_serialization.h"

/* Tamanho de bloco padrao quando --block-size nao e informado (64 KiB). */
#define DEFAULT_BLOCK_SIZE 65536u

/* Opcoes da linha de comando do czip. */
typedef struct {
    const char *input_path;
    const char *output_path;
    uint32_t    block_size;  /* bytes por bloco (--block-size) */
    long        threads;     /* aceito; ignorado na versao sequencial */
} CzipOptions;

static void print_usage(const char *prog)
{
    fprintf(stderr,
            "Uso: %s [--threads N] [--block-size BYTES] <entrada> <saida.cz>\n"
            "  --threads N         numero de threads (aceito; IGNORADO nesta\n"
            "                      versao sequencial - usado a partir do Modulo 13)\n"
            "  --block-size BYTES  tamanho do bloco em bytes (padrao %u)\n",
            prog, DEFAULT_BLOCK_SIZE);
}

/*
 * Converte 'str' em unsigned long, exigindo que seja inteiro positivo (> 0) e
 * que consuma a string inteira. Retorna true em sucesso, gravando em *out.
 */
static bool parse_positive_ulong(const char *str, unsigned long *out)
{
    if (str == NULL || *str == '\0') {
        return false;
    }
    char *end = NULL;
    unsigned long value = strtoul(str, &end, 10);
    if (*end != '\0' || value == 0UL) {
        return false;
    }
    *out = value;
    return true;
}

/*
 * Faz o parsing dos argumentos para 'opt'. Suporta as flags --threads e
 * --block-size (cada uma seguida do seu valor) em qualquer ordem antes dos dois
 * argumentos posicionais <entrada> <saida.cz>. Retorna true em sucesso.
 */
static bool parse_args(int argc, char **argv, CzipOptions *opt)
{
    opt->input_path  = NULL;
    opt->output_path = NULL;
    opt->block_size  = DEFAULT_BLOCK_SIZE;
    opt->threads     = 1;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];

        if (strcmp(arg, "--threads") == 0) {
            unsigned long value;
            if (i + 1 >= argc || !parse_positive_ulong(argv[i + 1], &value)) {
                fprintf(stderr, "czip: --threads exige um inteiro positivo.\n");
                return false;
            }
            opt->threads = (long)value;
            i++;  /* consome o valor */
        } else if (strcmp(arg, "--block-size") == 0) {
            unsigned long value;
            if (i + 1 >= argc || !parse_positive_ulong(argv[i + 1], &value)) {
                fprintf(stderr,
                        "czip: --block-size exige um inteiro positivo.\n");
                return false;
            }
            if (value > UINT32_MAX) {
                fprintf(stderr, "czip: --block-size excede o maximo (%u).\n",
                        UINT32_MAX);
                return false;
            }
            opt->block_size = (uint32_t)value;
            i++;  /* consome o valor */
        } else if (opt->input_path == NULL) {
            opt->input_path = arg;
        } else if (opt->output_path == NULL) {
            opt->output_path = arg;
        } else {
            fprintf(stderr, "czip: argumento inesperado: %s\n", arg);
            return false;
        }
    }

    if (opt->input_path == NULL || opt->output_path == NULL) {
        return false;
    }
    return true;
}

/*
 * Comprime UM bloco (os 'n' bytes de 'buf', com indice 'index') e o grava em
 * 'out': cabecalho do bloco, arvore serializada e payload comprimido, nessa
 * ordem (layout do Modulo 9). Retorna 0 em sucesso, 1 em erro (ja imprime a
 * causa). 'n' e sempre > 0 aqui, logo a arvore nunca e NULL.
 */
static int compress_one_block(FILE *out, const uint8_t *buf, size_t n,
                              uint64_t index)
{
    uint32_t crc = crc32_buffer(buf, n);

    BlockCompressed bc;
    if (!block_compress(buf, n, &bc)) {
        fprintf(stderr, "czip: falha ao comprimir o bloco %llu.\n",
                (unsigned long long)index);
        return 1;
    }

    uint8_t *tree_bytes = NULL;
    size_t   tree_size  = 0;
    if (!tree_serialize(bc.tree, &tree_bytes, &tree_size)) {
        fprintf(stderr, "czip: falha ao serializar a arvore do bloco %llu.\n",
                (unsigned long long)index);
        block_compressed_free(&bc);
        return 1;
    }

    CzBlockHeader bh;
    bh.block_index     = index;
    bh.original_size   = (uint32_t)bc.original_size;
    bh.compressed_size = (uint32_t)bc.compressed_size;
    bh.tree_size       = (uint32_t)tree_size;
    bh.original_crc32  = crc;

    bool ok = cz_write_block_header(out, &bh);
    if (ok && tree_size > 0) {
        ok = fwrite(tree_bytes, 1, tree_size, out) == tree_size;
    }
    if (ok && bc.compressed_size > 0) {
        ok = fwrite(bc.data, 1, bc.compressed_size, out) == bc.compressed_size;
    }

    free(tree_bytes);
    block_compressed_free(&bc);

    if (!ok) {
        fprintf(stderr, "czip: falha ao gravar o bloco %llu no arquivo de saida.\n",
                (unsigned long long)index);
        return 1;
    }
    return 0;
}

/*
 * Executa a compressao completa de opt->input_path para opt->output_path.
 * Retorna 0 em sucesso, 1 em erro. Cuida da abertura/fechamento dos arquivos,
 * do cabecalho global (placeholder e reescrita) e da liberacao do buffer.
 */
static int compress_file(const CzipOptions *opt)
{
    FILE *in = fopen(opt->input_path, "rb");
    if (in == NULL) {
        fprintf(stderr, "czip: nao foi possivel abrir a entrada '%s'.\n",
                opt->input_path);
        return 1;
    }

    FILE *out = fopen(opt->output_path, "wb");
    if (out == NULL) {
        fprintf(stderr, "czip: nao foi possivel criar a saida '%s'.\n",
                opt->output_path);
        fclose(in);
        return 1;
    }

    /* Cabecalho global com block_count provisorio = 0 (corrigido no fim). */
    CzGlobalHeader gh;
    cz_global_header_init(&gh, opt->block_size, 0);
    if (!cz_write_global_header(out, &gh)) {
        fprintf(stderr, "czip: falha ao gravar o cabecalho global.\n");
        fclose(in);
        fclose(out);
        return 1;
    }

    uint8_t *buf = malloc(opt->block_size);
    if (buf == NULL) {
        fprintf(stderr, "czip: sem memoria para o buffer de bloco (%u bytes).\n",
                opt->block_size);
        fclose(in);
        fclose(out);
        return 1;
    }

    uint64_t block_index = 0;
    int      rc          = 0;
    size_t   n;
    while ((n = fread(buf, 1, opt->block_size, in)) > 0) {
        if (compress_one_block(out, buf, n, block_index) != 0) {
            rc = 1;
            break;
        }
        block_index++;
    }

    if (rc == 0 && ferror(in)) {
        fprintf(stderr, "czip: erro de leitura na entrada '%s'.\n",
                opt->input_path);
        rc = 1;
    }

    /* Reescreve o cabecalho global com a quantidade real de blocos. */
    if (rc == 0) {
        if (fseek(out, 0L, SEEK_SET) != 0) {
            fprintf(stderr, "czip: falha ao reposicionar para o cabecalho global.\n");
            rc = 1;
        } else {
            cz_global_header_init(&gh, opt->block_size, block_index);
            if (!cz_write_global_header(out, &gh)) {
                fprintf(stderr, "czip: falha ao reescrever o cabecalho global.\n");
                rc = 1;
            }
        }
    }

    free(buf);
    fclose(in);
    if (fclose(out) != 0 && rc == 0) {
        fprintf(stderr, "czip: falha ao finalizar o arquivo de saida.\n");
        rc = 1;
    }

    if (rc == 0) {
        printf("czip: '%s' -> '%s' (%llu bloco(s), block-size=%u).\n",
               opt->input_path, opt->output_path,
               (unsigned long long)block_index, opt->block_size);
    }
    return rc;
}

int main(int argc, char **argv)
{
    CzipOptions opt;
    if (!parse_args(argc, argv, &opt)) {
        print_usage(argv[0]);
        return 1;
    }

    if (opt.threads != 1) {
        fprintf(stderr,
                "czip: aviso - versao sequencial (Modulo 10); --threads %ld "
                "sera ignorado (usado a partir do Modulo 13).\n",
                opt.threads);
    }

    return compress_file(&opt);
}
