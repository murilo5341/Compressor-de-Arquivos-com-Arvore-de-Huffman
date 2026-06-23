# Módulo 12 — Fila bloqueante (mutex + condvars)

Início do **núcleo de Sistemas Operacionais** (E3). Implementa a fila limitada
e bloqueante que vai ligar os estágios do pipeline concorrente de compressão
(Módulos 13–14). Ainda **não** monta o pipeline — entrega só a fila e seu teste.

## O que faz

- **Fila FIFO limitada** (`BlockingQueue`) sobre um **buffer circular** de
  capacidade fixa, guardando ponteiros opacos (`void*`).
- **`queue_push`**: bloqueia enquanto a fila está cheia; acorda quando abre vaga.
- **`queue_pop`**: bloqueia enquanto a fila está vazia **e** aberta; quando a
  fila está vazia **e** fechada, retorna `false` (sinal de fim de produção).
- **`queue_close`**: marca a fila como fechada e acorda todos os bloqueados;
  os itens já enfileirados ainda saem (a fila **drena**).

## Por que existe

- **modularizacao.md (Módulo 12):** é a "base da concorrência". O pipeline
  (RULES REGRA 5 / context.md) é leitora → fila → N codificadoras → fila →
  escritora; essas filas precisam ser **limitadas** e **bloqueantes**.
- **Contrapressão (backpressure):** capacidade fixa impede que um estágio rápido
  consuma memória sem limite quando o outro é mais lento — importante no teste de
  fogo de 1 GB (Módulo 17).
- **Coordenação sem espera ativa:** mutex + condvars deixam as threads
  *dormirem* quando não há trabalho, em vez de girar em busy-wait.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `include/queue.h` | API da fila bloqueante (tipo opaco `BlockingQueue`). |
| `src/queue.c` | Buffer circular + mutex + 2 condvars + flag de fechamento. |
| `tests/test_queue.c` | Testes single-thread e concorrentes (Linux/pthreads). |

## Estruturas principais

```c
struct BlockingQueue {
    void          **items;     /* buffer circular de ponteiros opacos */
    size_t          capacity;  /* numero maximo de itens */
    size_t          head;      /* indice do proximo item a sair */
    size_t          count;     /* itens atualmente na fila */
    bool            closed;    /* true apos queue_close */
    pthread_mutex_t mutex;
    pthread_cond_t  not_empty; /* consumidores esperam aqui */
    pthread_cond_t  not_full;  /* produtores esperam aqui */
};
```

Guardar **`head + count`** (em vez de `head + tail`) elimina a ambiguidade
clássica "fila cheia × fila vazia": a posição de entrada é `(head+count)%cap`.

## Funções principais

| Função | O que faz |
|--------|-----------|
| `queue_create(capacity)` | Cria a fila (capacity > 0), inicia mutex/condvars. |
| `queue_push(q, item)` | Enfileira; bloqueia se cheia; `false` se fechada. |
| `queue_pop(q, &out)` | Desenfileira; bloqueia se vazia+aberta; `false` se vazia+fechada. |
| `queue_close(q)` | Fecha e faz `broadcast` para acordar todos. |
| `queue_size(q)` | Quantidade atual (sob o mutex). |
| `queue_destroy(q)` | Libera buffer e recursos (não libera os itens). |

## Como compilar e testar

```sh
# Linux (onde ha pthreads)
make test            # inclui test_queue (CONC_TESTS = test_queue)
make tsan            # valida ausencia de data races (RULES REGRA 4, -15%)

# Windows (MinGW sem libpthread)
mingw32-make test    # test_queue NAO entra (CONC_TESTS vazio); demais rodam
```

Critério esperado: no `test_queue`, o teste concorrente (1 produtor × N
consumidores) confere que a **soma de tudo que foi consumido == soma do que foi
produzido** (1..N) e que **nada se perde nem duplica**; o teste de backpressure
repete isso com fila de capacidade 2 e consumidor lento. Sob ThreadSanitizer,
zero races reportadas.

## Como explicar na defesa

- **Por que `while` e não `if` na espera?** `pthread_cond_wait` pode acordar
  espuriamente e vários threads podem disputar o mutex; ao readquirir, a
  condição (cheia/vazia) precisa ser **reavaliada** no laço.
- **Por que `broadcast` no close e `signal` no push/pop?** No fluxo normal, um
  item libera **um** parceiro (`signal`). No fechamento, é preciso acordar
  **todos** os bloqueados (consumidores para verem o fim, produtores para
  desistirem) → `broadcast`.
- **Como o consumidor sabe parar sem item sentinela?** `queue_pop` retorna
  `false` quando a fila está **vazia e fechada**; o laço `while (queue_pop(...))`
  termina sozinho.
- **Por que fila limitada?** Backpressure: sem limite, o leitor encheria a RAM
  num arquivo de 1 GB se as codificadoras não acompanhassem.
- **Pergunta provável:** "e se fizer push depois do close?" → é **rejeitado**
  (`false`); após fechar, nenhuma produção nova é aceita.

## Decisões de projeto / referências

- **Buffer circular com `head + count`** (não `head/tail`) para distinguir cheia
  de vazia sem desperdiçar um slot.
- **A fila NÃO é dona dos itens:** `queue_destroy` não libera o que ficou
  enfileirado — a posse dos blocos é do pipeline (Módulos 13/14).
- **Push após close é rejeitado** (contrato explícito), e a fila **drena** os
  itens pendentes antes de `queue_pop` sinalizar fim.
- **Portabilidade:** usa pthreads; o MinGW local (GCC 6.3.0, modelo win32) não
  tem libpthread, então `test_queue` só entra no `make test` em Linux
  (`CONC_TESTS` no Makefile). A validação de data races (`make tsan`) e a
  ausência de mutex global (RULES REGRA 9) são feitas em ambiente Linux.
- `queue.c` ainda **não** entra em `COMMON_SRCS` — só será linkado ao `czip`
  quando o pipeline (Módulo 13) passar a usá-lo. Por ora, apenas no teste.
- Ver também: `modularizacao.md` (Módulo 12), `RULES.md` (REGRAS 4, 5 e 9),
  `docs/context.md` (pipeline concorrente) e a entrada do `DIARIO.md` de
  2026-06-23 — Módulo 12.
