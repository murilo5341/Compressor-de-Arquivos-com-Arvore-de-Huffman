# Módulo 13 — Pipeline concorrente de compressão

**Núcleo de Sistemas Operacionais (E3).** Monta o pipeline multithread que liga
os estágios pelas filas bloqueantes do [Módulo 12](modulo_12.md): comprime o
arquivo em **blocos independentes** com várias threads, gerando um `.cz`
**byte a byte idêntico** ao do `czip` sequencial ([Módulo 10](modulo_10.md)).

## O que faz

- **Pipeline de 3 estágios** ligados por 2 filas limitadas:
  `leitora → raw_q → N codificadoras → done_q → escritora`.
- **Leitora:** lê o arquivo em pedaços de `block_size`, numera cada bloco
  (índice sequencial) e o enfileira cru; fecha a `raw_q` no EOF.
- **Codificadoras (N):** cada uma tira um bloco cru, calcula o **CRC32** do
  original ([Módulo 2](modulo_02.md)), comprime com **Huffman**
  ([Módulo 6](modulo_06.md)) e **serializa a árvore** ([Módulo 7](modulo_07.md));
  enfileira o bloco comprimido. A **última** a sair fecha a `done_q`.
- **Escritora REORDENADORA:** recebe os blocos **fora de ordem** (as
  codificadoras terminam em tempos diferentes) e grava no `.cz` **na ordem
  correta**, liberando a memória de cada bloco logo após gravá-lo.
- **Mesma saída do sequencial:** o paralelismo muda só a velocidade, não os
  bytes — preserva o roundtrip do `cunzip` ([Módulo 11](modulo_11.md)).

## Por que existe

- **modularizacao.md (Módulo 13):** é o pipeline em si, o produto central da
  parte de SO. Sem ele, as filas do Módulo 12 não têm uso real.
- **RULES REGRA 5 / context.md:** o edital exige compressão **concorrente por
  blocos** com leitora → codificadoras → escritora.
- **Speedup do teste de fogo de 1 GB ([Módulo 17](modulo_17.md)):** dividir em
  blocos independentes permite paralelizar a compressão sem alterar o formato.
- **Sem mutex global de I/O (RULES REGRA 9):** o arquivo de saída é tocado por
  uma thread de cada vez, com ordem garantida por `pthread_create`/`join`.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `include/pipeline.h` | API de uma função: `pipeline_compress_file(...)`. |
| `src/pipeline.c` | Estágios (leitora/codificadora/escritora) + orquestração. |

Reusa `block.c` (M6), `crc32.c` (M2), `format.c` (M9), `tree_serialization.c`
(M7) e `queue.c` (M12). Validado por roundtrip junto ao `cunzip`; sem teste
unitário próprio (a corretude é o `.cz` idêntico ao sequencial + TSan).

## Estruturas principais

```c
/* Bloco cru lido do arquivo (leitora é dona de 'data' até a codificadora). */
typedef struct {
    uint64_t index;
    uint8_t *data;   /* malloc; liberado pela codificadora após comprimir */
    size_t   size;
} RawBlock;

/* Bloco já comprimido, pronto para gravar (codificadora é dona até a escritora). */
typedef struct {
    uint64_t index;
    uint32_t original_size, compressed_size, tree_size, crc;
    uint8_t *tree_bytes;  /* árvore serializada (M7) */
    uint8_t *payload;     /* bits comprimidos (M6) */
} CompressedBlock;

/* Estado compartilhado: SÓ o contador de codificadoras e os flags de erro são
 * mutáveis entre threads, todos sob 'lock'. As filas têm trava própria. */
typedef struct {
    FILE *in, *out;
    uint32_t block_size;
    BlockingQueue *raw_q, *done_q;
    pthread_mutex_t lock;     /* protege os campos abaixo */
    int  encoders_active;     /* codificadoras que ainda não saíram */
    bool read_error, encode_error;
    bool write_error;         /* escrito pela escritora; lido pela main pós-join */
    uint64_t block_count;     /* blocos efetivamente gravados */
} Pipeline;
```

A **posse de memória** é explícita e segue o fluxo: leitora aloca `data` →
codificadora libera `data` e aloca `tree_bytes`/`payload` → escritora grava e
libera. Nenhum bloco é dono compartilhado; a fila só carrega ponteiros.

## Funções principais

| Função | O que faz |
|--------|-----------|
| `pipeline_compress_file(in, out, block_size, num_threads)` | Orquestra tudo: cria threads, espera (`join`) e reescreve o `block_count`. |
| `reader_thread` | Lê blocos, numera, enfileira na `raw_q`; fecha-a no fim. |
| `encoder_thread` | CRC32 + Huffman + serializa árvore; última fecha a `done_q`. |
| `writer_thread` | Reordenador: grava blocos em ordem por `next`/`pending`. |
| `encode_block` | Comprime um `RawBlock` num `CompressedBlock` (doa o payload, sem cópia). |
| `write_block` | Grava cabeçalho + árvore + payload de 1 bloco (layout do M9). |

## Como compilar e testar

```sh
# Linux (onde há pthreads)
make            # compila o pipeline no czip
make tsan       # valida ausência de data races (RULES REGRA 4, -15%)
# roundtrip: czip arquivo -> cunzip -> diff bate, qualquer num_threads

# Windows (MinGW sem libpthread)
mingw32-make all   # czip usa o caminho SEQUENCIAL (M10); pipeline NÃO entra
```

Critério esperado: o `.cz` gerado com **N threads** é **byte a byte igual** ao
gerado com 1 thread (`diff`/`cmp` zerado), o `cunzip` recompõe o original
(CRC32 confere) e o ThreadSanitizer não reporta corrida.

## Como explicar na defesa

- **Por que blocos independentes?** Cada bloco tem sua própria árvore de Huffman
  e CRC32, então as codificadoras não compartilham estado → paralelismo sem
  travas no caminho quente.
- **Como a saída fica idêntica ao sequencial com threads fora de ordem?** A
  escritora **reordena**: guarda blocos adiantados em `pending[index]` e só grava
  quando chega `next`, na sequência. Ordem dos bytes não depende do escalonador.
- **Como cada estágio sabe que acabou?** Fechando a fila: a leitora fecha a
  `raw_q` no EOF; a **última** codificadora (contador `encoders_active` sob mutex)
  fecha a `done_q`. `queue_pop` retornando `false` encerra o laço de quem consome.
- **Por que não há mutex no arquivo de saída?** A ordem
  `main grava cabeçalho → cria threads → escritora grava blocos → join → main
  reescreve block_count` garante acesso por **uma** thread de cada vez
  (happens-before via `pthread_create`/`join`) — RULES REGRA 9.
- **Como evita estourar RAM no arquivo de 1 GB?** Filas **limitadas**
  (backpressure do M12) + liberação de cada bloco logo após gravado: nunca segura
  o arquivo inteiro em memória.
- **Pergunta provável:** "e se uma codificadora falhar no meio?" → marca
  `encode_error`, segue **drenando** para não travar a leitora; a escritora libera
  os blocos adiantados que ficaram órfãos e a `main` aborta com erro (sem `.cz`
  válido).

## Decisões de projeto / referências

- **Reordenador na escritora** (e não ordenação na fila): mantém as filas simples
  (FIFO de ponteiros) e concentra a lógica de ordem num só lugar.
- **Doação do payload** (`bc.data` → `cb->payload`, anulando o original) evita
  copiar os bytes comprimidos entre estágios.
- **Capacidade das filas = `2 × num_threads`** (mínimo `QUEUE_MIN_CAPACITY = 8`):
  folga para as threads sem segurar memória demais.
- **Portabilidade:** usa pthreads; o MinGW local (GCC 6.3.0, modelo win32) não
  tem libpthread, então o pipeline só compila/valida em Linux. No Windows o
  `czip` usa o caminho sequencial (M10) via `#ifndef HAVE_PTHREAD`.
- **Tratamento de erro completo:** falha de `malloc`, de criação de thread, de
  leitura, de compressão e de escrita são todas detectadas e propagadas; em erro
  não fica `.cz` válido (o `block_count` no cabeçalho não é reescrito).
- Ver também: `modularizacao.md` (Módulo 13), `RULES.md` (REGRAS 4, 5 e 9),
  `docs/context.md` (pipeline concorrente), `modulo_12.md` (fila) e a entrada do
  `DIARIO.md` de 2026-06-23 — Módulo 13.
