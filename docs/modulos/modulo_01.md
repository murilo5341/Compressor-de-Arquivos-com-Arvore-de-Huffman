# Módulo 1 — Heap binário mínimo

Fila de prioridade (min-heap binário) que ordena elementos pela **menor frequência**.
É a estrutura que a construção da árvore de Huffman (Módulo 3) usa para, a cada passo,
retirar os dois nós de menor frequência e combiná-los.

## O que faz

- Implementa um **min-heap binário** sobre um vetor dinâmico, com operações em `O(log n)`.
- Ordena por `frequency`; em caso de **empate**, desempata pela **ordem de inserção**
  (determinístico e reproduzível).
- Cresce sozinho (`realloc` dobrando a capacidade) e a API é **segura contra `NULL`**.
- Vem com teste unitário cobrindo os casos obrigatórios da **RULES REGRA 10**.

## Por que existe

- **RULES REGRA 5 / modularizacao.md (Módulo 1):** a árvore de Huffman é construída
  via heap binário (fila de prioridade). Este módulo entrega essa fila isolada e testada
  antes de a árvore (Módulo 3) depender dela.
- **RULES REGRA 10:** exige testes de heap para inserção, extração em ordem, empates e
  heap vazio — todos cobertos aqui.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `include/heap.h` | API pública: `HeapItem`, tipo opaco `MinHeap` e as funções. |
| `src/heap.c` | Implementação do min-heap (sift up/down, crescimento, desempate). |
| `tests/test_heap.c` | Testes unitários (ordem crescente, heap vazio/NULL, empates). |

## Estruturas principais

```c
// include/heap.h — elemento visível ao usuário
typedef struct {
    uint64_t frequency;  // chave de ordenacao (menor sai primeiro)
    void    *payload;    // opaco: a partir do Modulo 3 aponta para o no de Huffman
} HeapItem;

typedef struct MinHeap MinHeap; // tipo opaco (definicao fica na .c)
```

```c
// src/heap.c — detalhes internos
typedef struct {
    HeapItem item;
    uint64_t seq;   // ordem de insercao -> criterio de desempate deterministico
} HeapEntry;

struct MinHeap {
    HeapEntry *entries;   // vetor que armazena o heap
    size_t     size;      // qtd. de elementos
    size_t     capacity;  // capacidade alocada (dobra quando enche)
    uint64_t   next_seq;  // proximo seq a atribuir no push
};
```

A propriedade de heap é mantida por `entry_less`: compara por `frequency` e, em empate,
pelo `seq` (quem entrou antes tem prioridade). Como cada `push` recebe um `seq` único, a
ordem é **estritamente total** — nunca há ambiguidade, o que torna os testes reproduzíveis.

## Funções principais

| Função | O que faz |
|--------|-----------|
| `heap_create(capacity)` | Cria heap vazio (`capacity == 0` usa padrão); retorna `NULL` se faltar memória. |
| `heap_push(heap, item)` | Insere e sobe o elemento (sift up); cresce o vetor se cheio. Retorna `false` em falha. |
| `heap_pop_min(heap, out)` | Remove o menor; copia para `*out` e desce o novo topo (sift down). `false` se vazio/NULL. |
| `heap_size(heap)` / `heap_is_empty(heap)` | Consultam o tamanho (0 / `true` se `NULL`). |
| `heap_destroy(heap)` | Libera o vetor e a struct. Seguro com `NULL`. |
| `sift_up` / `sift_down` (internas) | Restauram a propriedade de min-heap após push/pop. |

## Como compilar e testar

```sh
# Linux (ambiente oficial de validacao)
make test       # compila tests/test_heap.c + src/heap.c e roda test_heap
make asan       # (opcional) roda os testes/binarios com AddressSanitizer

# Windows (MinGW, em C:\MinGW\bin)
mingw32-make test
```

Saída esperada (critério de aprovação):

```
=== Testes do heap binario (Modulo 1) ===
Teste 1: extracao em ordem crescente de frequencia
Teste 2: extracao de heap vazio nao deve quebrar
Teste 3: empate de frequencia sai em ordem de insercao

53 verificacoes, 0 falha(s).
TODOS OS TESTES PASSARAM.
```

O teste retorna código `!= 0` se qualquer verificação falhar, fazendo o `make test` falhar.

## Como explicar na defesa

- **Por que um min-heap?** Huffman precisa repetidamente do **menor** elemento; um min-heap
  dá `push`/`pop_min` em `O(log n)`, contra `O(n)` de uma busca linear.
- **Como o heap funciona no vetor?** Para o índice `i`: pai `(i-1)/2`, filhos `2i+1` e `2i+2`.
  `push` insere no fim e sobe (sift up); `pop_min` tira a raiz, traz o último para o topo e desce
  (sift down).
- **Por que desempatar por ordem de inserção, e não por byte?** É determinístico para os testes
  e funciona para **nós internos** da árvore (Módulo 3), que não têm um único byte. Usar o byte
  ainda deixaria empates entre nós internos.
- **Pergunta provável: "e se o heap encher?"** → Ele cresce sozinho (`realloc` dobrando a
  capacidade), então `push` só falha se faltar memória de verdade.
- **Pergunta provável: "pop em heap vazio quebra?"** → Não; retorna `false` sem travar (testado),
  inclusive com ponteiro `NULL`.
- **O que é o `payload`?** Um `void*` opaco; no Módulo 3 ele passa a apontar para o nó da árvore
  de Huffman, mantendo o heap genérico e desacoplado.

## Decisões de projeto / referências

- **Desempate determinístico pela ordem de inserção** (campo `seq`), registrado no `DIARIO.md`.
- `MinHeap` é **tipo opaco** e o vetor **cresce dinamicamente**; toda a API é **NULL-safe**.
- `heap.c` ainda **não** entra em `COMMON_SRCS` do Makefile — será linkado ao `czip`/`cunzip`
  quando o Huffman (Módulo 3) passar a usá-lo.
- Ver também: `modularizacao.md` (Módulo 1), `RULES.md` (REGRAS 5 e 10), entrada do
  `DIARIO.md` de 2026-06-22 — Módulo 1.
