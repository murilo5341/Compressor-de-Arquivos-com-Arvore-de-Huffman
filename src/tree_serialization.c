/*
 * tree_serialization.c - Implementacao da (de)serializacao da arvore (Modulo 7).
 *
 * Serializacao: travessia recursiva em pre-ordem escrevendo bits com o BitWriter
 * (Modulo 5). Desserializacao: travessia recursiva simetrica lendo bits com o
 * BitReader. O formato e auto-delimitado (ver tree_serialization.h), entao a
 * recursao da leitura para sozinha ao reconstruir a arvore inteira.
 */
#include "tree_serialization.h"

#include <stdlib.h>

#include "bitio.h"

/* ------------------------------------------------------------------ */
/* Serializacao                                                        */
/* ------------------------------------------------------------------ */

/*
 * Escreve um no em pre-ordem: 1 + symbol (folha) ou 0 + esquerda + direita
 * (interno). Nao verifica w->error a cada passo: o BitWriter vira no-op apos um
 * erro e o chamador (tree_serialize) checa w.error uma vez ao final.
 */
static void serialize_node(BitWriter *w, const HuffNode *n)
{
    if (n->is_leaf) {
        bit_writer_write_bit(w, 1);
        bit_writer_write_bits(w, n->symbol, 8);
    } else {
        bit_writer_write_bit(w, 0);
        serialize_node(w, n->left);
        serialize_node(w, n->right);
    }
}

bool tree_serialize(const HuffNode *root, uint8_t **out_bytes, size_t *out_size)
{
    if (out_bytes != NULL)
        *out_bytes = NULL;
    if (out_size != NULL)
        *out_size = 0;

    if (root == NULL || out_bytes == NULL || out_size == NULL)
        return false;

    BitWriter w;
    if (!bit_writer_init(&w))
        return false;

    serialize_node(&w, root);

    /* Fecha o ultimo byte parcial (padding de zeros) e checa erro de alocacao. */
    if (!bit_writer_flush(&w) || w.error) {
        bit_writer_free(&w);
        return false;
    }

    /* Doamos o buffer do writer ao chamador (sem copia), como no Modulo 6. */
    *out_bytes = w.buffer;
    *out_size  = w.byte_count;
    return true;
}

/* ------------------------------------------------------------------ */
/* Desserializacao                                                     */
/* ------------------------------------------------------------------ */

/*
 * Le um no em pre-ordem. Marca '*err' = true e retorna NULL se faltarem bits
 * (serializacao truncada) ou em falta de memoria. Em erro num filho, libera a
 * subarvore ja montada (huffman_free_tree e NULL-safe).
 */
static HuffNode *deserialize_node(BitReader *r, bool *err)
{
    int bit = bit_reader_read_bit(r);
    if (bit < 0) {            /* fim de fluxo no meio da arvore => corrompida */
        *err = true;
        return NULL;
    }

    HuffNode *n = calloc(1, sizeof *n);
    if (n == NULL) {
        *err = true;
        return NULL;
    }

    if (bit == 1) {
        /* Folha: os proximos 8 bits sao o byte do simbolo. */
        uint64_t sym;
        if (!bit_reader_read_bits(r, 8, &sym)) {
            *err = true;
            free(n);
            return NULL;
        }
        n->is_leaf = true;
        n->symbol  = (uint8_t)sym;
    } else {
        /* No interno: reconstroi esquerda e depois direita. */
        n->is_leaf = false;
        n->left    = deserialize_node(r, err);
        n->right   = deserialize_node(r, err);
        if (*err) {
            huffman_free_tree(n);
            return NULL;
        }
    }

    return n;
}

HuffNode *tree_deserialize(const uint8_t *bytes, size_t size)
{
    if (bytes == NULL || size == 0)
        return NULL;

    BitReader r;
    bit_reader_init(&r, bytes, size);

    bool err = false;
    HuffNode *root = deserialize_node(&r, &err);
    if (err) {
        huffman_free_tree(root);
        return NULL;
    }
    return root;
}
