/*
 * block.c - Implementacao da compressao de um bloco em memoria (Modulo 6).
 *
 * O fluxo segue exatamente o pipeline de compressao de Huffman, reaproveitando
 * os modulos anteriores:
 *
 *   frequencias (Modulo 3) -> arvore (Modulo 3) -> tabela de codigos (Modulo 4)
 *   -> escrita bit a bit dos codigos (Modulos 4 e 5) -> payload comprimido.
 *
 * Uma observacao de projeto: o BitWriter cresce sozinho, entao nao precisamos
 * saber o tamanho comprimido de antemao. Ao final, "doamos" o buffer do writer
 * para o BlockCompressed (sem copia extra), e por isso NAO chamamos
 * bit_writer_free() no caminho de sucesso - quem libera esse buffer passa a ser
 * block_compressed_free().
 *
 * O Modulo 8 adiciona block_decompress(): o caminho inverso, que desce a arvore
 * bit a bit (BitReader, Modulo 5) ate reconstruir 'original_size' bytes. Esse e
 * o nucleo que o cunzip (Modulo 11) usa, com a arvore vinda da desserializacao
 * (Modulo 7) e os tamanhos vindos do cabecalho do bloco (Modulo 9).
 */
#include "block.h"

#include <stdlib.h>

#include "bitio.h"

/* Zera a struct de saida (estado consistente em qualquer caminho de erro). */
static void block_compressed_zera(BlockCompressed *out)
{
    out->tree            = NULL;
    out->data            = NULL;
    out->compressed_size = 0;
    out->original_size   = 0;
}

bool block_compress(const void *input, size_t len, BlockCompressed *out)
{
    if (out == NULL)
        return false;

    block_compressed_zera(out);

    /* Bloco vazio: resultado valido e vazio (tree NULL, sem payload). */
    if (len == 0)
        return true;

    /* 1) Frequencias e 2) arvore de Huffman do bloco. */
    uint64_t freq[256];
    huffman_count_frequencies(input, len, freq);

    HuffNode *tree = huffman_build_tree(freq);
    if (tree == NULL)
        return false; /* len > 0 mas sem arvore => falta de memoria */

    /* 3) Tabela de codigos por travessia da arvore. */
    HuffCode table[256];
    if (!huffman_build_codes(tree, table)) {
        huffman_free_tree(tree);
        return false;
    }

    /* 4) e 5) Emite o codigo de cada byte, bit a bit, e fecha o ultimo byte. */
    BitWriter w;
    if (!bit_writer_init(&w)) {
        huffman_free_tree(tree);
        return false;
    }

    const uint8_t *bytes = (const uint8_t *)input;
    for (size_t i = 0; i < len; i++) {
        HuffCode c = table[bytes[i]];
        /* Todo byte presente no bloco tem um codigo (length > 0). */
        if (!bit_writer_write_bits(&w, c.bits, c.length)) {
            bit_writer_free(&w);
            huffman_free_tree(tree);
            return false;
        }
    }

    if (!bit_writer_flush(&w)) {
        bit_writer_free(&w);
        huffman_free_tree(tree);
        return false;
    }

    /* Sucesso: doamos o buffer do writer ao resultado (sem copia). */
    out->tree            = tree;
    out->data            = w.buffer;
    out->compressed_size = w.byte_count;
    out->original_size   = len;
    return true;
}

void block_compressed_free(BlockCompressed *bc)
{
    if (bc == NULL)
        return;
    huffman_free_tree(bc->tree);
    free(bc->data);
    bc->tree            = NULL;
    bc->data            = NULL;
    bc->compressed_size = 0;
    bc->original_size   = 0;
}

/* ------------------------------------------------------------------ */
/* Descompressao de um bloco (Modulo 8)                                */
/* ------------------------------------------------------------------ */

bool block_decompress(const HuffNode *tree, const uint8_t *data, size_t data_size,
                      size_t original_size, uint8_t **out)
{
    if (out == NULL)
        return false;

    *out = NULL;

    /* Bloco vazio: nada a decodificar, sucesso com *out NULL. */
    if (original_size == 0)
        return true;

    /* Bloco nao vazio precisa de arvore. */
    if (tree == NULL)
        return false;

    uint8_t *result = (uint8_t *)malloc(original_size);
    if (result == NULL)
        return false;

    BitReader r;
    bit_reader_init(&r, data, data_size);

    for (size_t i = 0; i < original_size; i++) {
        const HuffNode *node = tree;

        if (node->is_leaf) {
            /* Folha unica: cada byte gasta 1 bit (codigo "0" do Modulo 4). */
            if (bit_reader_read_bit(&r) < 0) {
                free(result);
                return false;
            }
            result[i] = node->symbol;
            continue;
        }

        /* Desce a arvore ate uma folha: 0 = esquerda, 1 = direita. */
        while (!node->is_leaf) {
            int bit = bit_reader_read_bit(&r);
            if (bit < 0) {              /* faltam bits => payload truncado */
                free(result);
                return false;
            }
            node = bit ? node->right : node->left;
            if (node == NULL) {         /* arvore inconsistente */
                free(result);
                return false;
            }
        }
        result[i] = node->symbol;
    }

    *out = result;
    return true;
}
