# Esboço do Relatório Experimental — Tema 11 (Compressor Huffman)

> Esqueleto do relatório técnico (8–15 páginas, RULES REGRA 7). O PDF final é
> produzido manualmente pela equipe; este arquivo organiza as seções e aponta de
> onde sai cada gráfico/dado. Os gráficos são gerados por `scripts/plot_results.py`
> a partir de `results/resultados.csv` (coletado por `scripts/run_bench.sh`).

## Como gerar os dados e os gráficos

```sh
# Em Linux/WSL (pipeline concorrente exige pthreads):
make all
make stress                 # gen_inputs + run_bench -> results/resultados.csv
python scripts/plot_results.py   # -> results/graphs/*.png
```

Para o teste de fogo de 1 GB explícito:

```sh
sh scripts/gen_inputs.sh inputs 33554432 1073741824   # bench 32MiB + fogo 1GiB
THREADS="1 2 4 8 16" sh scripts/run_bench.sh inputs results/resultados.csv
python scripts/plot_results.py
```

## Seções do relatório

### 1. Introdução
- Problema: compressão de arquivos grandes por blocos com Huffman + pipeline de threads.
- Objetivos: czip/cunzip corretos, speedup quase linear, integridade por CRC32.

### 2. Estrutura de dados central (Huffman via heap)
- Construção da árvore via min-heap (Módulos 1, 3).
- Tabela de códigos e propriedade de prefixo (Módulo 4).
- Serialização da árvore no cabeçalho do bloco (Módulo 7).
- **Teoria × prática**: profundidade média dos códigos vs. entropia do arquivo;
  tamanho da árvore serializada vs. nº de símbolos distintos.

### 3. Formato `.cz` e integridade
- Cabeçalho global e de bloco, endianess little-endian (Módulo 9).
- CRC32 por bloco: política e detecção de corrupção (Módulos 2, 11, 16).

### 4. Mecanismo de SO (pipeline concorrente)
- Leitor → N codificadores → escritor reordenador (Módulos 12–14).
- Filas limitadas com condvars; reordenação por índice de bloco.

### 5. Análise experimental (gráficos)
| Gráfico | Arquivo PNG | Fonte |
|---------|-------------|-------|
| Speedup × threads (1,2,4,8,16) + reta ideal | `speedup_vs_threads.png` | `resultados.csv` |
| Tempo de compressão × threads | `tempo_vs_threads.png` | `resultados.csv` |
| Throughput (MiB/s) × threads | `throughput_vs_threads.png` | `resultados.csv` |
| Taxa de compressão por tipo de arquivo | `taxa_por_tipo.png` | `resultados.csv` |

- **Speedup**: comparar curva medida com o ideal linear; apontar onde satura.
- **Gargalo do pipeline** (RULES REGRA 5): identificar quando o estágio de
  **escrita** serializa o pipeline (escritora reordenadora é sequencial por
  natureza); discutir leitura, CRC32, compressão e reordenação como candidatos.
- **Taxa por tipo**: texto/logs/repetido comprimem bem; aleatório ~0% (pior caso).
- **Overhead do CRC32**: medir comparando uma execução com e sem o cálculo do
  CRC (requer um build com o CRC desabilitado — medição complementar).

### 6. Teste de fogo (1 GB)
- Compactar `fire_1g.bin` (1 GiB) com sucesso.
- Speedup até o nº de núcleos (`nproc`).
- Corromper 1 byte de um bloco → detecção por CRC32 + demais blocos OK
  (`make test_corruption`, Módulo 16).

### 7. Conclusões
- O que o experimento confirmou da teoria; limites observados; trabalhos futuros.

## Observações
- **Dados reais (RULES REGRA 7)**: nenhum número é inventado; o `resultados.csv`
  é coletado pela equipe. `results/` e `inputs/` são gitignored (saídas geradas).
- **Ambiente de medição**: o speedup só aparece com o pipeline concorrente
  (pthreads), executado em Linux/WSL. Registrar CPU, nº de núcleos e RAM usados.
