/*
 * huffman.c - Construcao da arvore de Huffman (Modulo 3).
 *
 * Reaproveita o min-heap do Modulo 1 como fila de prioridade: cada item do heap
 * guarda a frequencia (chave de ordenacao) e, no 'payload', o ponteiro para o
 * no da arvore. Combinamos repetidamente os dois nos de menor frequencia ate
 * sobrar um unico no (a raiz).
 *
 * Determinismo: as folhas sao empurradas em ordem crescente de byte (0..255) e
 * os nos internos depois delas. Como o heap desempata pela ordem de insercao
 * (Modulo 1), a arvore resultante e sempre a mesma para a mesma distribuicao de
 * frequencias - importante para que os testes sejam reproduziveis.
 */
#include "huffman.h"

#include "heap.h"

#include <stdlib.h>

/* ------------------------------------------------------------------ */
/* Construtores de no                                                  */
/* ------------------------------------------------------------------ */

/* Cria uma folha para 'symbol' com a frequencia dada. NULL se faltar memoria. */
static HuffNode *huff_new_leaf(uint8_t symbol, uint64_t frequency)
{
    HuffNode *node = malloc(sizeof(*node));
    if (node == NULL)
        return NULL;

    node->left      = NULL;
    node->right     = NULL;
    node->frequency = frequency;
    node->symbol    = symbol;
    node->is_leaf   = true;
    return node;
}

/* Cria um no interno com os filhos dados; frequencia = soma dos filhos. */
static HuffNode *huff_new_internal(HuffNode *left, HuffNode *right)
{
    HuffNode *node = malloc(sizeof(*node));
    if (node == NULL)
        return NULL;

    node->left      = left;
    node->right     = right;
    node->frequency = left->frequency + right->frequency;
    node->symbol    = 0; /* sem significado em no interno */
    node->is_leaf   = false;
    return node;
}

/* ------------------------------------------------------------------ */
/* Contagem de frequencias                                             */
/* ------------------------------------------------------------------ */

void huffman_count_frequencies(const void *buf, size_t len, uint64_t freq[256])
{
    for (int i = 0; i < 256; i++)
        freq[i] = 0;

    if (buf == NULL || len == 0)
        return;

    const unsigned char *bytes = (const unsigned char *)buf;
    for (size_t i = 0; i < len; i++)
        freq[bytes[i]]++;
}

/* ------------------------------------------------------------------ */
/* Construcao da arvore                                                */
/* ------------------------------------------------------------------ */

/*
 * Esvazia o heap liberando a (sub)arvore apontada por cada item restante e,
 * por fim, destroi o proprio heap. Usado tanto no caminho de erro (memoria)
 * quanto - implicitamente - ao garantir que nada vaze.
 */
static void heap_drain_and_free(MinHeap *heap)
{
    HeapItem it;
    while (heap_pop_min(heap, &it))
        huffman_free_tree((HuffNode *)it.payload);
    heap_destroy(heap);
}

HuffNode *huffman_build_tree(const uint64_t freq[256])
{
    if (freq == NULL)
        return NULL;

    MinHeap *heap = heap_create(256);
    if (heap == NULL)
        return NULL;

    /* 1) uma folha por byte distinto (freq > 0), em ordem crescente de byte. */
    for (int s = 0; s < 256; s++) {
        if (freq[s] == 0)
            continue;

        HuffNode *leaf = huff_new_leaf((uint8_t)s, freq[s]);
        if (leaf == NULL) {
            heap_drain_and_free(heap);
            return NULL;
        }

        HeapItem item = { freq[s], leaf };
        if (!heap_push(heap, item)) {
            huffman_free_tree(leaf);
            heap_drain_and_free(heap);
            return NULL;
        }
    }

    /* Bloco vazio: nenhum simbolo com frequencia > 0. */
    if (heap_is_empty(heap)) {
        heap_destroy(heap);
        return NULL;
    }

    /* 2) combina os dois menores ate sobrar um unico no.
     *    Com um unico byte distinto, o laco nao executa e a raiz e a folha. */
    while (heap_size(heap) > 1) {
        HeapItem a;
        HeapItem b;
        heap_pop_min(heap, &a);
        heap_pop_min(heap, &b);

        HuffNode *parent =
            huff_new_internal((HuffNode *)a.payload, (HuffNode *)b.payload);
        if (parent == NULL) {
            /* a e b ja sairam do heap: precisam ser liberados a parte. */
            huffman_free_tree((HuffNode *)a.payload);
            huffman_free_tree((HuffNode *)b.payload);
            heap_drain_and_free(heap);
            return NULL;
        }

        HeapItem item = { parent->frequency, parent };
        if (!heap_push(heap, item)) {
            huffman_free_tree(parent); /* libera parent, a e b (filhos) */
            heap_drain_and_free(heap);
            return NULL;
        }
    }

    /* 3) o ultimo no e a raiz. */
    HeapItem root_item;
    heap_pop_min(heap, &root_item);
    heap_destroy(heap);
    return (HuffNode *)root_item.payload;
}

void huffman_free_tree(HuffNode *root)
{
    if (root == NULL)
        return;
    huffman_free_tree(root->left);
    huffman_free_tree(root->right);
    free(root);
}
