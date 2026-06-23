# Módulo 10 — `czip` sequencial

Primeiro **executável** de verdade: o compressor `czip`. Amarra toda a cadeia de
ED (Módulos 2–9) num programa de linha de comando que lê um arquivo, divide em
blocos e grava um `.cz` comprimido com Huffman — ainda **sem threads** (o
pipeline concorrente é dos Módulos 12–14).

## O que faz

- **CLI** (`parse_args`): aceita `--threads N` e `--block-size BYTES` em qualquer
  ordem antes dos posicionais `<entrada> <saida.cz>`. `--threads` é validado e
  **ignorado** nesta versão sequencial (existe para não retrabalhar o parsing no
  Módulo 13); `--block-size` define o tamanho do bloco (padrão 64 KiB).
- **Compressão por blocos** (`compress_file`): grava o cabeçalho global, lê a
  entrada em pedaços de `block_size` bytes e comprime cada um.
- **Um bloco** (`compress_one_block`): CRC32 do conteúdo original (Módulo 2) →
  `block_compress` (Módulo 6) → `tree_serialize` (Módulo 7) → grava cabeçalho do
  bloco + árvore + payload no layout do Módulo 9.
- **Correção do `block_count`**: o cabeçalho global é gravado com contagem
  provisória `0`; ao fim, `fseek` volta ao início e reescreve com a quantidade
  real de blocos.

## Por que existe

- **modularizacao.md (Módulo 10) / RULES REGRA 5 e 7:** o utilitário `czip` é
  entregável obrigatório e precisa de CLI configurável de **threads** e
  **tamanho de bloco** para viabilizar o teste de fogo de 1 GB e o relatório de
  speedup (Módulos 17–18).
- Fecha o lado da **escrita** do formato `.cz`: produz arquivos que o `cunzip`
  (Módulo 11) vai validar e restaurar.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `src/main_czip.c` | Parsing de CLI, laço de leitura por blocos, compressão de cada bloco e escrita do `.cz` (cabeçalho global + por bloco). |
| `Makefile` | `COMMON_SRCS` passa a linkar `block.c`, `huffman.c`, `heap.c`, `bitio.c`, `tree_serialization.c`, `crc32.c` e `format.c` ao `czip`/`cunzip`. |

## Estruturas principais

```c
typedef struct {
    const char *input_path;
    const char *output_path;
    uint32_t    block_size;  /* bytes por bloco (--block-size, padrao 64 KiB) */
    long        threads;     /* aceito; ignorado na versao sequencial */
} CzipOptions;
```

## Funções principais

| Função | O que faz |
|--------|-----------|
| `parse_args(argc, argv, opt)` | Lê flags (`--threads`, `--block-size`) e os dois posicionais; valida valores positivos. |
| `compress_file(opt)` | Abre arquivos, grava cabeçalho global, itera blocos e reescreve `block_count` no fim. |
| `compress_one_block(out, buf, n, index)` | Comprime e grava um bloco: CRC32 → Huffman → árvore serializada → payload. |
| `parse_positive_ulong(str, out)` | Conversão segura de string para inteiro positivo (consome a string toda). |

## Como compilar e testar

```sh
# Linux
make all                                   # compila czip e cunzip
./czip --block-size 65536 entrada.txt saida.cz

# Windows (MinGW)
mingw32-make all
./czip.exe --block-size 65536 entrada.txt saida.cz
```

Critério esperado: o `.cz` começa com o magic `CZHF`; um arquivo vazio gera só o
cabeçalho global (20 bytes, `block_count = 0`); um arquivo de 1 byte gera 47
bytes (20 global + 24 cabeçalho de bloco + 2 árvore de folha única + 1 payload).
O roundtrip byte a byte é fechado pelo `cunzip` no Módulo 11.

## Como explicar na defesa

- **Por que reescrever o `block_count` no fim?** Só sabemos quantos blocos
  existem depois de ler o arquivo todo. Em vez de exigir o tamanho do arquivo
  adiantado, gravamos `0`, contamos os blocos e voltamos com `fseek` ao offset 12
  do cabeçalho global para corrigir. A saída é um arquivo regular (seekable).
- **Por que `--threads` já existe se é ignorado?** Para a CLI ficar estável: o
  Módulo 13 liga as threads sem mudar a interface nem os scripts de benchmark.
- **Qual o critério de divisão em blocos?** Pedaços fixos de `block_size` bytes;
  o último bloco pode ser menor (é o `fread` que retorna menos). Cada bloco é
  independente (árvore + CRC32 próprios), o que habilita paralelismo e
  recuperação parcial.
- **Pergunta provável:** "o que acontece com um bloco de um único byte
  distinto?" → árvore de folha única, código "0" (1 bit/byte); o payload tem
  padding e a parada na descompressão é por `original_size`.

## Decisões de projeto / referências

- **`block_count` provisório + `fseek` de reescrita** no fim, em vez de
  pré-calcular o tamanho do arquivo (registrado no `DIARIO.md`).
- **CLI sequencial com `--threads` aceito e ignorado** (parsing pronto para o
  Módulo 13); `--block-size` padrão 64 KiB.
- **`COMMON_SRCS` agora não vazio**: a partir deste módulo `czip`/`cunzip`
  linkam toda a cadeia de ED. `cunzip` ainda é esqueleto até o Módulo 11.
- Ver também: `modularizacao.md` (Módulo 10), `RULES.md` (REGRAS 5 e 7),
  `include/format.h` (Módulo 9), `include/block.h` (Módulos 6/8),
  `include/tree_serialization.h` (Módulo 7), `include/crc32.h` (Módulo 2) e a
  entrada do `DIARIO.md` de 2026-06-22 — Módulo 10.
