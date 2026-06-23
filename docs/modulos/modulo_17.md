# Módulo 17 — Stress, benchmarks e teste de fogo

Resumo: scripts que geram as entradas e medem **tempo, speedup, taxa de compressão
e throughput** do `czip`, produzindo `results/resultados.csv` — a base de dados
reais do relatório (RULES REGRA 7) e do teste de fogo de 1 GB (REGRA 5).

## O que faz

- `scripts/gen_inputs.sh` gera arquivos de **tipos** variados (texto, logs, binário,
  repetido, aleatório) e o arquivo grande do **teste de fogo** (~1 GiB).
- `scripts/run_bench.sh` roda o `czip` para cada arquivo × cada nº de threads
  (1/2/4/8/16), mede o tempo (`date +%s.%N`), valida o roundtrip (`cunzip` + `cmp`)
  e grava uma linha por medição no CSV.
- Alvos `make stress` / `make bench` automatizam gerar + medir.

## Por que existe

Cumpre a **RULES REGRA 7** (análise experimental com dados reais) e o **teste de
fogo** da REGRA 5 (1 GB, speedup quase linear, gargalo da escrita). É a fonte do
`resultados.csv` que o Módulo 18 transforma em gráficos. Sem inventar números: o
CSV é coletado pela própria equipe.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `scripts/gen_inputs.sh` | Gera entradas por tipo + arquivo de 1 GiB do teste de fogo. |
| `scripts/run_bench.sh` | Mede tempo/taxa/throughput por arquivo × threads → CSV. |
| `Makefile` | Alvos `stress`/`bench` (gerar+medir) e variáveis de tamanho/threads. |
| `results/resultados.csv` | Saída de dados (gitignored; gerada na execução). |

## Schema do `results/resultados.csv`

```text
arquivo,tipo,tamanho_bytes,block_size,threads,rep,tempo_s,tamanho_cz_bytes,taxa_pct,throughput_mibs,ok
```

- `taxa_pct = 100 * (1 - tamanho_cz / tamanho)` — quanto o arquivo encolheu.
- `throughput_mibs = tamanho / tempo / 1048576` — MiB/s de entrada processada.
- `ok = 1` se o roundtrip bateu byte a byte. O **speedup** não é gravado: é
  derivado no Módulo 18 como `tempo[1 thread] / tempo[N]`.

## Como usar

```sh
make all
# benchmark rapido (sem o arquivo de 1 GiB):
make stress BENCH_SIZE=33554432 FIRE_SIZE=0

# teste de fogo completo (1 GiB) com a varredura de threads:
make stress BENCH_SIZE=33554432 FIRE_SIZE=1073741824 THREADS="1 2 4 8 16"

# ou direto:
sh scripts/gen_inputs.sh inputs 33554432 1073741824
sh scripts/run_bench.sh inputs results/resultados.csv
```

## Ambiente de medição (importante)

O **speedup** só é real quando o `czip` roda **concorrente**, o que exige o
pipeline com pthreads (Módulos 13/14) — **Linux ou WSL**. No Windows/MinGW (sem
libpthread) o `czip` cai no caminho **sequencial** e as colunas de threads saem com
tempos ~iguais (speedup ~1); útil só para validar a mecânica dos scripts. A coleta
válida para o relatório é feita em WSL/Linux. Registrar CPU, núcleos (`nproc`) e RAM.

## Como explicar na defesa

- Por que medir vários **tipos** de arquivo? A taxa de Huffman depende da
  distribuição: texto/repetido comprimem muito, aleatório ~0% (pior caso).
- Por que o **menor tempo** (melhor de N) das repetições (no Módulo 18)?
  Interferência do SO só adiciona tempo; o mínimo é a estimativa mais limpa do
  custo intrínseco. Reduz ruído sem mascarar a tendência.
- Onde o pipeline **satura**? A escritora reordenadora é sequencial por natureza;
  acima de certo nº de threads ela vira o gargalo (analisado no relatório).
- Por que o roundtrip dentro do bench? Garante que a medição é de uma compressão
  **correta** (não adianta ser rápido e corromper).

## Decisões de projeto / referências

- `date +%s.%N` + `awk` para o tempo e a matemática de ponto flutuante (sem
  depender de `/usr/bin/time` GNU nem de `bc`, ausentes em alguns ambientes).
- Arquivos grandes montados por **duplicação** (`cat a a > b`) → cresce em O(log n)
  passos, viável para 1 GiB.
- `results/` e `inputs/` são **gitignored**: são saídas geradas, não dados versionados
  (coerente com RULES REGRA 7 — dados reais coletados, não commitados como fixos).
- Ver também: `modularizacao.md` (Módulo 17), `RULES.md` (REGRA 5/7),
  `modulo_18.md` (gráficos), `relatorio/esboco.md`.
