# Módulo 0 — Estrutura inicial do projeto

Base de build e organização do repositório. Cria a árvore de diretórios, o `Makefile`
com todos os alvos exigidos e os esqueletos dos binários `czip` e `cunzip`. Ainda não
há lógica de compressão — só a fundação para os módulos seguintes.

## O que faz

- Define a estrutura de pastas: `include/` (cabeçalhos `.h`), `src/` (fontes `.c`),
  `tests/` (testes) e `scripts/` (geração de entradas, benchmarks, gráficos).
- Fornece um `Makefile` com os alvos obrigatórios `all`, `test`, `stress`, `clean`
  e os alvos extras de validação `asan`, `tsan`, `valgrind` (+ `help`).
- Cria esqueletos de `czip` e `cunzip` que validam os argumentos e imprimem o uso.
- Compila limpo com `gcc -std=c11 -Wall -Wextra -Werror` (0 warnings — RULES REGRA 2).

## Por que existe

- **RULES REGRA 4** exige Makefile com `all`/`test`/`stress`/`clean`.
- **modularizacao.md (Módulo 0)** pede os alvos `asan`/`tsan` desde o início, para que a
  validação de **vazamentos (−10%)** e **data races (−15%)** seja rotina, e não deixada
  para o fim do projeto.
- Ter a separação por módulos (`include`/`src`/`tests`) desde já evita um arquivo único
  difícil de manter e de defender oralmente.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `Makefile` | Build, testes, sanitizers e limpeza. Detecta o SO. |
| `src/main_czip.c` | Esqueleto do compressor: valida argumentos, imprime uso/stub. |
| `src/main_cunzip.c` | Esqueleto do descompressor: valida argumentos, imprime uso/stub. |
| `.gitignore` | Ignora artefatos de build (`*.exe`, `*.o`, `*.cz`, `results/`, `.claude/`). |
| `README.md` | Visão geral, estrutura, instruções de build e dos sanitizers/Valgrind. |
| `include/ src/ tests/ scripts/` | Pastas dos módulos (vazias mantidas via `.gitkeep`). |

## Estruturas principais

Ainda não há `struct`s em C (entram a partir do Módulo 1). As "estruturas" deste módulo
estão no **Makefile**:

- **Alvos**: `all` (compila `czip` e `cunzip`), `test`, `stress`, `clean`,
  `asan`, `tsan`, `valgrind`, `help`.
- **Variáveis de flags**: `CFLAGS = -std=c11 -Wall -Wextra -Werror -O2 -Iinclude $(PTHREAD)`
  e `LDFLAGS = $(PTHREAD)`.
- **Sanitizers via variáveis target-specific**: em `asan`/`tsan`, `CFLAGS`/`LDFLAGS` recebem
  `-fsanitize=address|thread`; essas variáveis **propagam para o pré-requisito `all`**, então
  o build inteiro é recompilado instrumentado.
- **Detecção de plataforma** (ponto-chave):

```make
ifeq ($(OS),Windows_NT)
    PTHREAD =          # MinGW (modelo win32) NAO tem libpthread
    RM      = del /Q /F
    EXE     = .exe
else
    PTHREAD = -pthread # Linux: threads do pipeline (E3)
    RM      = rm -f
    EXE     =
endif
```

  No Windows o `-pthread` é omitido (senão o `ld` falha com `cannot find -lpthread`); no Linux
  ele é usado normalmente. `RM` e `EXE` também se ajustam ao SO.

## Funções principais

| Função | O que faz |
|--------|-----------|
| `main()` em `main_czip.c` | Se `argc < 3`, imprime uso em `stderr` e retorna 1; senão imprime mensagem de stub. |
| `main()` em `main_cunzip.c` | Mesmo contrato, com o uso do descompressor. |

## Como compilar e testar

```sh
# Linux (ambiente oficial de validacao)
make            # compila czip e cunzip
make test       # placeholder ate o Modulo 1
make clean      # remove binarios e artefatos
make asan       # build com AddressSanitizer (so Linux)
make tsan       # build com ThreadSanitizer  (so Linux)
valgrind --leak-check=full --error-exitcode=1 ./czip ENTRADA SAIDA.cz

# Windows (MinGW, em C:\MinGW\bin)
mingw32-make all
```

Comprovação atual (Módulo 0):

```sh
./czip                 # -> imprime uso e sai com codigo 1
./czip entrada saida   # -> "czip: estrutura inicial (Modulo 0)..."
./cunzip arq.cz saida  # -> "cunzip: estrutura inicial (Modulo 0)..."
```

> ⚠️ `asan`/`tsan`/`valgrind` exigem **Linux**. O MinGW local (GCC 6.3.0, modelo de threads
> win32) não suporta esses sanitizers nem o Valgrind, e não possui `libpthread` — por isso a
> concorrência (E3) e as validações de memória/race serão feitas em ambiente Linux.

## Como explicar na defesa

- **Por que `-Wall -Wextra -Werror`?** Tolerância zero a warnings; cada warning é −5%. `-Werror`
  transforma warning em erro de compilação, garantindo código limpo.
- **Por que `asan`/`tsan` já no Módulo 0?** Para validar vazamentos (−10%) e data races (−15%)
  de forma contínua desde o começo, não no fim.
- **Diferença ASan × TSan × Valgrind:** ASan detecta uso indevido/vazamento de memória; TSan
  detecta data races; Valgrind é alternativa para vazamentos. Todos rodam em Linux.
- **Por que detectar o SO no Makefile?** O MinGW (win32) não tem `libpthread`; sem a detecção, o
  `-pthread` quebra o build no Windows. No Linux, `-pthread` é necessário para o pipeline.
- **Por que `.cz` e `CZHF`?** São escolhas da equipe (não exigência do edital), documentadas e
  justificáveis; o edital exige apenas a árvore serializada no cabeçalho de cada bloco.
- **Por que separar `include`/`src`/`tests`/`scripts`?** Organização por módulo, facilita testes
  isolados e a defesa oral.

## Decisões de projeto / referências

- Cabeçalhos em `include/`, fontes em `src/` (conforme `modularizacao.md`).
- Makefile multiplataforma com detecção de `Windows_NT`.
- `.gitignore` corrigido: antes ignorava o próprio `Makefile` (entregável obrigatório).
- Registrado no `DIARIO.md` (entrada de 2026-06-22 — Módulo 0). Ver `modularizacao.md`
  (Módulo 0) e `RULES.md` (REGRAS 2 e 4).
