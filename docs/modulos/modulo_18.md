# Módulo 18 — Gráficos e relatório

Resumo: `scripts/plot_results.sh` agrega o `results/resultados.csv` (Módulo 17) com
`awk` e chama o **gnuplot** (`scripts/plot_results.gp`) para gerar os gráficos do
relatório (speedup, tempo, throughput, taxa por tipo); `relatorio/` guarda o
esqueleto do relatório técnico.

## O que faz

- O wrapper (`.sh`) lê o CSV do benchmark e **agrega** por (arquivo, threads),
  usando o **menor tempo** das repetições (melhor de N).
- Deriva o **speedup** = `tempo[baseline] / tempo[N]` (baseline = menor nº de
  threads) e escreve arquivos `.dat` intermediários + um manifesto.
- O gnuplot lê os `.dat` e gera 4 PNGs em `results/graphs/`:
  `speedup_vs_threads.png`, `tempo_vs_threads.png`, `throughput_vs_threads.png`,
  `taxa_por_tipo.png`.
- `relatorio/esboco.md` organiza as seções do relatório (8–15 páginas) e mapeia de
  onde sai cada gráfico/dado.

## Por que existe

Fecha a **RULES REGRA 7**: gráficos por **script reproduzível** (gnuplot,
versionado) a partir de dados reais, confrontando teoria × prática. É o material
visual da defesa e do relatório.

## Por que gnuplot (e não Python)

O **sistema** do Tema 11 é em C (czip/cunzip + estruturas de dados + mecanismos de
SO); o edital (`trabalho.txt`, Seção 2) lista *"scripts de geração dos gráficos do
relatório"* como artefato **separado** do sistema, e a RULES REGRA 7 permite
*"gnuplot, matplotlib, etc."*. Escolheu-se **gnuplot**: é a ferramenta clássica de
curso de SO, não acrescenta stack Python ao projeto, e o script declarativo é fácil
de **editar ao vivo** na defesa. A agregação usa `awk` (mesma ferramenta do
`run_bench.sh`), portável e sem dependências.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `scripts/plot_results.sh` | Agrega o CSV (awk) e invoca o gnuplot. |
| `scripts/plot_results.gp` | Script gnuplot que desenha os 4 PNGs. |
| `relatorio/esboco.md` | Esqueleto do relatório técnico + comandos de geração. |
| `Makefile` | Alvo `graficos` (chama o wrapper). |

## Fluxo de dados

```text
results/resultados.csv
   │  (awk no plot_results.sh: menor tempo por arquivo×threads, speedup, taxa)
   ▼
WORK/series_<i>.dat   (threads  tempo_s  throughput_mibs  speedup)  + manifestos
WORK/taxa.dat         (tipo  taxa_pct)
   │  (gnuplot -e "work=...; outdir=..." plot_results.gp)
   ▼
results/graphs/*.png
```

A separação **agregação (awk) × desenho (gnuplot)** mantém o gnuplot simples (só
plota) e deixa a matemática (menor tempo, speedup, throughput) num só lugar testável.

## Gráficos gerados (RULES REGRA 7)

| PNG | Eixo X | Eixo Y | Leitura |
|-----|--------|--------|---------|
| `speedup_vs_threads.png` | threads | speedup | curva medida vs. ideal linear (y=x) |
| `tempo_vs_threads.png` | threads | tempo (s) | queda do tempo com mais threads |
| `throughput_vs_threads.png` | threads | MiB/s | vazão de entrada por threads |
| `taxa_por_tipo.png` | tipo | taxa (%) | compressibilidade por tipo de arquivo |

## Como usar

```sh
sudo apt install gnuplot      # uma vez (no WSL)
# depois de 'make stress' ter gerado o CSV:
make graficos                 # -> results/graphs/*.png
# ou:
sh scripts/plot_results.sh results/resultados.csv results/graphs
```

Sem gnuplot o wrapper termina com mensagem clara e código 2 (não quebra o build);
sem CSV, código 1.

## Como explicar na defesa

- Por que a **reta ideal** no gráfico de speedup? É o referencial linear (N threads
  → N×); a distância da curva real até ela mostra o overhead/gargalo.
- Por que o **menor tempo** (e não a média)? Interferência do SO só adiciona tempo;
  o mínimo estima o custo intrínseco. Robusto a outliers de medição.
- Por que o baseline do speedup é o **menor** nº de threads do CSV? Normaliza por
  `tempo[base]`; se o CSV começar em 1 thread, é o tempo sequencial de referência.
- O que o `taxa_por_tipo` ensina? Liga a **teoria de Huffman** (entropia) à prática:
  aleatório ~0% confirma que não há almoço grátis para dados incompressíveis.
- Como editar um gráfico ao vivo? Abrir `plot_results.gp`, mudar `ylabel`/`title`/
  faixa e rodar `make graficos` de novo — é só o script declarativo do gnuplot.

## Decisões de projeto / referências

- **gnuplot** (RULES REGRA 7) em vez de Python: sistema é C, gráfico é artefato
  separado; gnuplot não traz stack Python e é editável na defesa.
- **awk** para a agregação (mawk-safe, sem `asort`): por isso "menor tempo" e não
  "mediana" (mediana exigiria ordenar, ausente no mawk do Ubuntu).
- Terminal `pngcairo` (anti-aliasing); `system("cat manifest")` no gnuplot para
  iterar uma série por arquivo via `plot for`.
- PNGs em `results/graphs/` (gitignored); são saídas geradas, regeneráveis do CSV.
- Ver também: `modularizacao.md` (Módulo 18), `RULES.md` (REGRA 7),
  `modulo_17.md` (geração do CSV), `relatorio/esboco.md`.
