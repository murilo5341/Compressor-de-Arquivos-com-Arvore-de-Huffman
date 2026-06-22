/*
 * heap.c - Implementacao do min-heap binario (Modulo 1).
 *
 * O heap e armazenado num vetor: para o no no indice i,
 *   - pai          = (i - 1) / 2
 *   - filho esq.   = 2*i + 1
 *   - filho dir.   = 2*i + 2
 * Insercao sobe o elemento (sift up) e remocao desce o novo topo (sift down),
 * ambos em O(log n). A ordem e definida por entry_less (frequencia, depois
 * ordem de insercao para desempate deterministico).
 */
#include "heap.h"

#include <stdlib.h>

#define HEAP_DEFAULT_CAPACITY 16

/*
 * Cada elemento guarda o item do usuario e um numero de sequencia (seq)
 * atribuido na insercao. O seq e o criterio de desempate: menor seq = inserido
 * antes = sai antes. Como cada push usa um seq diferente, a ordem total e estrita.
 */
typedef struct {
    HeapItem item;
    uint64_t seq;
} HeapEntry;

struct MinHeap {
    HeapEntry *entries;
    size_t     size;
    size_t     capacity;
    uint64_t   next_seq;
};

/* Retorna true se 'a' tem prioridade maior que 'b' (deve sair antes). */
static bool entry_less(const HeapEntry *a, const HeapEntry *b)
{
    if (a->item.frequency != b->item.frequency)
        return a->item.frequency < b->item.frequency;
    return a->seq < b->seq; /* empate: quem entrou primeiro sai primeiro */
}

static void swap_entries(HeapEntry *a, HeapEntry *b)
{
    HeapEntry tmp = *a;
    *a = *b;
    *b = tmp;
}

/* Sobe o elemento no indice i ate restaurar a propriedade de min-heap. */
static void sift_up(MinHeap *heap, size_t i)
{
    while (i > 0) {
        size_t parent = (i - 1) / 2;
        if (!entry_less(&heap->entries[i], &heap->entries[parent]))
            break;
        swap_entries(&heap->entries[i], &heap->entries[parent]);
        i = parent;
    }
}

/* Desce o elemento no indice i ate restaurar a propriedade de min-heap. */
static void sift_down(MinHeap *heap, size_t i)
{
    for (;;) {
        size_t left     = 2 * i + 1;
        size_t right    = 2 * i + 2;
        size_t smallest = i;

        if (left < heap->size &&
            entry_less(&heap->entries[left], &heap->entries[smallest]))
            smallest = left;
        if (right < heap->size &&
            entry_less(&heap->entries[right], &heap->entries[smallest]))
            smallest = right;

        if (smallest == i)
            break;

        swap_entries(&heap->entries[i], &heap->entries[smallest]);
        i = smallest;
    }
}

MinHeap *heap_create(size_t capacity)
{
    MinHeap *heap = malloc(sizeof(*heap));
    if (heap == NULL)
        return NULL;

    if (capacity == 0)
        capacity = HEAP_DEFAULT_CAPACITY;

    heap->entries = malloc(capacity * sizeof(*heap->entries));
    if (heap->entries == NULL) {
        free(heap);
        return NULL;
    }

    heap->size     = 0;
    heap->capacity = capacity;
    heap->next_seq = 0;
    return heap;
}

/* Dobra a capacidade do vetor interno. Retorna false se faltar memoria. */
static bool heap_grow(MinHeap *heap)
{
    size_t new_capacity = heap->capacity * 2;
    HeapEntry *resized =
        realloc(heap->entries, new_capacity * sizeof(*resized));
    if (resized == NULL)
        return false;

    heap->entries  = resized;
    heap->capacity = new_capacity;
    return true;
}

bool heap_push(MinHeap *heap, HeapItem item)
{
    if (heap == NULL)
        return false;
    if (heap->size == heap->capacity && !heap_grow(heap))
        return false;

    heap->entries[heap->size].item = item;
    heap->entries[heap->size].seq  = heap->next_seq++;
    heap->size++;
    sift_up(heap, heap->size - 1);
    return true;
}

bool heap_pop_min(MinHeap *heap, HeapItem *out)
{
    if (heap == NULL || heap->size == 0)
        return false;

    if (out != NULL)
        *out = heap->entries[0].item;

    heap->size--;
    if (heap->size > 0) {
        heap->entries[0] = heap->entries[heap->size];
        sift_down(heap, 0);
    }
    return true;
}

size_t heap_size(const MinHeap *heap)
{
    return heap == NULL ? 0 : heap->size;
}

bool heap_is_empty(const MinHeap *heap)
{
    return heap_size(heap) == 0;
}

void heap_destroy(MinHeap *heap)
{
    if (heap == NULL)
        return;
    free(heap->entries);
    free(heap);
}
