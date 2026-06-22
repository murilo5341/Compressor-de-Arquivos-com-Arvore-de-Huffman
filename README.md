# Compressor de Arquivos com Árvore de Huffman

Projeto interdisciplinar **Estrutura de Dados (C) × Sistemas Operacionais** — Tema 11.
Ifes Cachoeiro de Itapemirim.

Dois utilitários:

- **`czip`** — comprime arquivos grandes em blocos independentes usando codificação de Huffman.
- **`cunzip`** — descomprime arquivos `.cz`, valida a integridade por bloco (CRC32) e restaura o conteúdo original.

> **Decisão de projeto:** a extensão `.cz` e o magic number `CZHF` são escolhas da equipe
> (não exigências do edital), documentadas e justificáveis na defesa oral.

## Estrutura do projeto

```text
.
├── Makefile          # build, testes, sanitizers e clean
├── README.md
├── RULES.md          # regras obrigatórias do trabalho
├── DIARIO.md         # registro do uso de IA (RULES REGRA 1)
├── modularizacao.md  # plano modular (Módulos 0 a 18)
├── docs/             # contexto, guia de implementação e edital
├── include/          # cabeçalhos (.h) dos módulos
├── src/              # implementação (.c) e os mains de czip/cunzip
├── tests/            # testes unitários e de integração
└── scripts/          # geração de entradas, benchmarks e gráficos
```

## Compilação

Requisito: **C11** com `gcc`. A compilação é feita sem warnings (RULES REGRA 2):

```sh
gcc -std=c11 -Wall -Wextra -Werror
```

Alvos do `Makefile`:

| Alvo            | Descrição                                                        |
|-----------------|------------------------------------------------------------------|
| `make` / `make all` | Compila `czip` e `cunzip`.                                   |
| `make test`     | Compila e executa os testes unitários.                           |
| `make stress`   | Executa o teste de stress/carga (teste de fogo).                 |
| `make asan`     | Build com **AddressSanitizer** (vazamentos / uso indevido).      |
| `make tsan`     | Build com **ThreadSanitizer** (data races).                      |
| `make valgrind` | Instruções de uso do **Valgrind**.                              |
| `make clean`    | Remove binários e artefatos.                                     |
| `make help`     | Lista os alvos.                                                  |

### Validação de memória e concorrência

Estes alvos garantem que a validação de vazamentos (−10%) e data races (−15%) seja rotina:

```sh
make asan     # rebuild com AddressSanitizer
make tsan     # rebuild com ThreadSanitizer

# Valgrind (alternativa ao ASan para vazamentos):
make
valgrind --leak-check=full --error-exitcode=1 ./czip <entrada> <saida.cz>
```

> ⚠️ **Ambiente:** `-fsanitize=address/thread` e o Valgrind **requerem Linux**
> (com `gcc`/`clang` recentes e `valgrind` instalado). O **MinGW no Windows não
> suporta** esses sanitizers nem o Valgrind — rode essas validações em um ambiente Linux.

## Uso (a partir dos módulos seguintes)

```sh
./czip --threads N --block-size BYTES entrada.txt saida.cz
./cunzip saida.cz restaurado.txt
cmp entrada.txt restaurado.txt
```

## Estado atual

**Módulo 0 — Estrutura inicial concluída:** árvore de diretórios, `Makefile` com alvos
`all`/`test`/`stress`/`clean` + `asan`/`tsan`/`valgrind`, e binários `czip`/`cunzip` ainda
como esqueletos. A implementação dos módulos segue o plano em `modularizacao.md`.
