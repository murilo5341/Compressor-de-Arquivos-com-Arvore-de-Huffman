/*
 * heap.h - Fila de prioridade (min-heap binario) do Modulo 1.
 *
 * Esta e a estrutura usada pela construcao da arvore de Huffman (Modulo 3):
 * a cada passo, os dois nos de MENOR frequencia sao retirados e combinados.
 * Por isso o heap e um MIN-heap ordenado pela frequencia.
 *
 * Contrato de desempate (deterministico - exigido pela modularizacao):
 *   - chave primaria  : frequency (menor sai primeiro);
 *   - chave secundaria: ordem de insercao (quem entra primeiro sai primeiro).
 * Usar a ordem de insercao como desempate torna a saida reproduzivel nos testes
 * e funciona inclusive para nos internos da arvore (que nao tem um unico byte).
 */
#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * HeapItem - elemento armazenado na fila de prioridade.
 *
 * frequency: chave de ordenacao (min-heap pela menor frequencia).
 * payload:   ponteiro opaco. A partir do Modulo 3 aponta para o no da arvore
 *            de Huffman associado a essa frequencia. No Modulo 1 o teste pode
 *            usa-lo livremente (ou deixar NULL).
 */
typedef struct {
    uint64_t frequency;
    void    *payload;
} HeapItem;

/* Tipo opaco: o usuario manipula o heap apenas pelos ponteiros abaixo. */
typedef struct MinHeap MinHeap;

/* Cria um heap vazio. capacity == 0 usa uma capacidade inicial padrao.
 * O heap cresce sozinho conforme necessario. Retorna NULL se faltar memoria. */
MinHeap *heap_create(size_t capacity);

/* Insere um item. Retorna false se heap for NULL ou faltar memoria ao crescer. */
bool heap_push(MinHeap *heap, HeapItem item);

/* Remove e devolve (em *out) o item de menor frequencia.
 * Retorna false se o heap for NULL ou estiver vazio (sem travar). out pode ser NULL. */
bool heap_pop_min(MinHeap *heap, HeapItem *out);

/* Quantidade de itens no heap (0 se heap for NULL). */
size_t heap_size(const MinHeap *heap);

/* true se o heap estiver vazio ou for NULL. */
bool heap_is_empty(const MinHeap *heap);

/* Libera o heap. Seguro chamar com NULL. */
void heap_destroy(MinHeap *heap);

#endif /* HEAP_H */
