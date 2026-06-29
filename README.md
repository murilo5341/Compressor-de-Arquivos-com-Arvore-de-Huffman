# Compressor de Arquivos com Árvore de Huffman

Projeto interdisciplinar **Estrutura de Dados (C) × Sistemas Operacionais** — Tema 11.
Ifes Cachoeiro de Itapemirim.

Dois utilitários finais:

- **`czip`** — comprime arquivos em blocos independentes usando Huffman por bloco.
- **`cunzip`** — descomprime arquivos `.cz`, valida CRC32 por bloco e restaura o conteúdo possível.

> **Decisão de projeto:** a extensão `.cz` e o magic number `CZHF` são escolhas da equipe
> (não exigências do edital), documentadas para defesa oral.

## Estado atual

Implementação funcional até os módulos de robustez, benchmark e gráficos:

- Compressão e descompressão byte a byte com `czip`/`cunzip`.
- Huffman por bloco com heap mínimo, tabela de códigos, escrita/leitura bit a bit e serialização da árvore.
- Formato `.cz` versionado (`CZHF`, versão 1), little-endian e autossuficiente para reconstrução.
- CRC32 do conteúdo original em cada bloco.
- Recuperação parcial: bloco corrompido é reportado e descartado; blocos seguintes continuam sendo lidos.
- Pipeline concorrente no Linux/WSL com pthreads: leitora → N codificadoras → escritora reordenadora.
- Fallback sequencial no Windows/MinGW, mantendo mesma CLI; `--threads` é aceito, mas ignorado sem pthreads.
- Testes unitários, roundtrip, corrupção, benchmark e geração de gráficos por script.

## Estrutura do projeto

```text
.
├── Makefile          # build, testes, sanitizers, benchmark, gráficos e clean
├── README.md
├── RULES.md          # regras obrigatórias do trabalho
├── DIARIO.md         # registro do uso de IA
├── modularizacao.md  # plano modular (Módulos 0 a 18)
├── docs/             # contexto, edital, implementação e sessões
├── include/          # cabeçalhos (.h) dos módulos
├── src/              # implementação (.c) e mains de czip/cunzip
├── tests/            # testes unitários e de integração
├── scripts/          # geração de entradas, benchmark e gráficos
└── relatorio/        # esboço/material do relatório
```

Artefatos gerados ficam fora do versionamento: binários, `inputs/`, `results/`, arquivos `.cz` e saídas restauradas.

## Requisitos

- `gcc` com suporte a **C11**.
- `make`.
- Shell POSIX (`sh`) para testes de integração e scripts.
- Linux/WSL para validação final com pthreads, ASan, TSan, Valgrind e speedup real.
- `gnuplot` apenas para `make graficos`.

Compilação obrigatória do trabalho:

```sh
gcc -std=c11 -Wall -Wextra -Werror
```

Configuração atual do `Makefile`:

- flags base: `-std=c11 -Wall -Wextra -Werror -O2 -Iinclude`;
- Linux/WSL: adiciona `-pthread` e `-DHAVE_PTHREAD`, compilando `src/pipeline.c` e `src/queue.c` no `czip`;
- Windows/MinGW: omite `-pthread`, não compila o pipeline e usa compressão sequencial.

## Compilação

```sh
make
```

Saídas esperadas:

- Linux/WSL: `czip` e `cunzip`.
- Windows/MinGW: `czip.exe` e `cunzip.exe`.

Limpeza:

```sh
make clean
```

## Alvos do Makefile

| Alvo | Descrição |
|------|-----------|
| `make` / `make all` | Compila `czip` e `cunzip`. |
| `make test` | Compila e executa testes unitários e de integração disponíveis no ambiente. |
| `make test_roundtrip` | Testa `czip -> cunzip -> cmp` em casos de borda. |
| `make test_corruption` | Corrompe payload e valida detecção por CRC32 com recuperação parcial. |
| `make stress` | Gera entradas e roda benchmark, gravando `results/resultados.csv`. |
| `make bench` | Atalho para `make stress`. |
| `make graficos` | Gera PNGs em `results/graphs/` a partir do CSV (requer `gnuplot`). |
| `make asan` | Build com AddressSanitizer (Linux). |
| `make tsan` | Build com ThreadSanitizer (Linux). |
| `make valgrind` | Mostra comando recomendado de Valgrind (Linux). |
| `make clean` | Remove binários e artefatos locais. |
| `make help` | Lista os alvos disponíveis. |

No Linux/WSL, `make test` inclui também `test_queue` e `test_pipeline`. No Windows/MinGW, esses testes concorrentes são pulados porque não há pthreads no ambiente configurado.

## Uso

Linux/WSL:

```sh
./czip [--threads N] [--block-size BYTES] entrada.txt saida.cz
./cunzip saida.cz restaurado.txt
cmp entrada.txt restaurado.txt
```

Windows/MinGW ou PowerShell:

```powershell
.\czip.exe --block-size 65536 entrada.txt saida.cz
.\cunzip.exe saida.cz restaurado.txt
cmd /c fc /b entrada.txt restaurado.txt
```

Parâmetros do `czip`:

- `--threads N`: número de threads codificadoras no Linux/WSL; padrão `1`.
- `--block-size BYTES`: tamanho de cada bloco; padrão `65536` bytes (64 KiB).

Observação: no Windows/MinGW, `--threads` é aceito para manter compatibilidade de CLI, mas o programa avisa no `stderr` que vai usar caminho sequencial.

## Formato `.cz`

Todos os inteiros são gravados em **little-endian**, byte a byte.

Cabeçalho global (`20` bytes):

```text
magic[4]    = "CZHF"
version     = uint32
block_size  = uint32
block_count = uint64
```

Para cada bloco:

```text
block_index     = uint64
original_size   = uint32
compressed_size = uint32
tree_size       = uint32
original_crc32  = uint32
tree_bytes[tree_size]
compressed_bytes[compressed_size]
```

`tree_size` e `compressed_size` permitem ao `cunzip` saltar para o próximo bloco mesmo quando árvore, payload ou CRC do bloco atual estiverem inválidos.

## Testes

Rodar suíte principal:

```sh
make test
```

Cobertura atual:

- heap mínimo;
- CRC32;
- árvore de Huffman;
- tabela de códigos;
- escrita/leitura bit a bit;
- compressão e descompressão de bloco;
- serialização/desserialização da árvore;
- leitura/escrita do formato `.cz`;
- roundtrip com arquivo vazio, byte único, símbolo repetido, todos os bytes, texto, binário e aleatório;
- corrupção de payload com detecção por CRC32;
- fila bloqueante e pipeline concorrente no Linux/WSL.

## Benchmark e gráficos

Benchmark padrão do `Makefile`:

```sh
make stress
```

Configuração padrão:

- `BENCH_SIZE=33554432` (32 MiB por arquivo de teste);
- `FIRE_SIZE=0` no `make stress` para pular o arquivo de 1 GiB por padrão;
- `THREADS="1 2 4 8 16"` no script de benchmark;
- `BLOCK=1048576` (1 MiB) no script de benchmark;
- `REPS=3` repetições por ponto.

Teste de fogo completo com 1 GiB:

```sh
make stress FIRE_SIZE=1073741824 THREADS="1 2 4 8 16"
```

Gerar gráficos após o benchmark:

```sh
make graficos
```

Saídas:

- `results/resultados.csv`;
- `results/graphs/speedup_vs_threads.png`;
- `results/graphs/tempo_vs_threads.png`;
- `results/graphs/throughput_vs_threads.png`;
- `results/graphs/taxa_por_tipo.png`.

O speedup real só aparece em Linux/WSL, porque o pipeline concorrente depende de pthreads. No Windows/MinGW, os tempos por número de threads tendem a ficar próximos por usar fallback sequencial.

## Validação de memória e concorrência

Linux/WSL:

```sh
make asan
make tsan
make valgrind
```

Uso direto do Valgrind após compilar:

```sh
valgrind --leak-check=full --error-exitcode=1 ./czip entrada.txt saida.cz
```

ASan, TSan e Valgrind não são suportados pelo MinGW local configurado neste projeto.
