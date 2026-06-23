# Módulo 18 — Gráficos e relatório

Resumo: `scripts/plot_results.py` lê o `results/resultados.csv` (Módulo 17) e gera
os gráficos do relatório (speedup, tempo, throughput, taxa por tipo); `relatorio/`
guarda o esqueleto do relatório técnico.

## O que faz

- Lê o CSV do benchmark e **agrega** por (arquivo, threads), usando a **mediana**
  das repetições para reduzir ruído.
- Deriva o **speedup** = `tempo[1 thread] / tempo[N]` e plota contra a reta ideal.
- Gera 4 PNGs em `results/graphs/`:
  `speedup_vs_threads.png`, `tempo_vs_threads.png`, `throughput_vs_threads.png`,
  `taxa_por_tipo.png`.
- `relatorio/esboco.md` organiza as seções do relatório (8–15 páginas) e mapeia de
  onde sai cada gráfico/dado.

## Por que existe

Fecha a **RULES REGRA 7**: gráficos gerados por **script reproduzível**
(matplotlib, versionado) a partir de dados reais, confrontando teoria × prática. É
o material visual da defesa e do relatório.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `scripts/plot_results.py` | Lê o CSV, agrega e gera os 4 PNGs. |
| `relatorio/esboco.md` | Esqueleto do relatório técnico + comandos de geração. |
| `Makefile` | Alvo `graficos` (chama o `plot_results.py`). |

## Estrutura do script

Separado em funções testáveis sem matplotlib:

| Função | O que faz |
|--------|-----------|
| `load_rows(csv)` | Lê o CSV (só `csv` da stdlib) e converte os tipos. |
| `aggregate(rows)` | Mediana de tempo/throughput por (arquivo, threads) + speedup + taxa. |
| `make_plots(per_file, outdir)` | Gera os 4 PNGs (importa matplotlib só aqui). |

A separação permite validar a **lógica de dados** (parsing, mediana, speedup) mesmo
sem matplotlib instalado; só `make_plots` exige a dependência.

## Gráficos gerados (RULES REGRA 7)

| PNG | Eixo X | Eixo Y | Leitura |
|-----|--------|--------|---------|
| `speedup_vs_threads.png` | threads | speedup | curva medida vs. ideal linear (y=x) |
| `tempo_vs_threads.png` | threads | tempo (s) | queda do tempo com mais threads |
| `throughput_vs_threads.png` | threads | MiB/s | vazão de entrada por threads |
| `taxa_por_tipo.png` | tipo | taxa (%) | compressibilidade por tipo de arquivo |

## Como usar

```sh
# depois de 'make stress' ter gerado o CSV:
pip install matplotlib        # uma vez (ou no WSL)
make graficos                 # -> results/graphs/*.png
# ou:
python scripts/plot_results.py results/resultados.csv results/graphs
```

Sem matplotlib o script termina com mensagem clara e código 2 (não quebra o build).

## Como explicar na defesa

- Por que a **reta ideal** no gráfico de speedup? É o referencial de speedup linear
  (N threads → N×); a distância da curva real até ela mostra o overhead/gargalo.
- Por que **mediana** e não média? É robusta a outliers de medição (um pico de GC
  do SO não distorce o ponto).
- Por que o baseline do speedup é o **menor** nº de threads do CSV? Normaliza por
  `tempo[1]`; se o CSV começar em 1 thread, é o tempo sequencial de referência.
- O que o `taxa_por_tipo` ensina? Liga a **teoria de Huffman** (entropia) à prática:
  aleatório ~0% confirma que não há almoço grátis para dados incompressíveis.

## Decisões de projeto / referências

- **matplotlib** (RULES REGRA 7 permite matplotlib/gnuplot); leitura do CSV só com a
  stdlib (`csv`) para manter a lógica testável e leve.
- Backend `Agg` (sem display) → roda em WSL/servidor/CI sem ambiente gráfico.
- PNGs em `results/graphs/` (gitignored); são saídas geradas, regeneráveis do CSV.
- Ver também: `modularizacao.md` (Módulo 18), `RULES.md` (REGRA 7),
  `modulo_17.md` (geração do CSV), `relatorio/esboco.md`.
