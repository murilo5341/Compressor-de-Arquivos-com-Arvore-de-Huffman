#!/bin/sh
# ============================================================================
# run_bench.sh - Benchmark de compressao: speedup, taxa e throughput (Modulo 17).
#
# Para cada arquivo de entrada e cada numero de threads (1/2/4/8/16), roda o czip
# medindo o tempo, valida o roundtrip (cunzip + cmp) e grava uma linha no CSV:
#   results/resultados.csv
#
# Colunas do CSV:
#   arquivo,tipo,tamanho_bytes,block_size,threads,rep,tempo_s,tamanho_cz_bytes,taxa_pct,throughput_mibs,ok
# onde:
#   taxa_pct        = 100 * (1 - tamanho_cz / tamanho)         (quanto encolheu)
#   throughput_mibs = tamanho / tempo / 1048576                (MiB/s de entrada)
#   ok              = 1 se o roundtrip bateu byte a byte, senao 0
# O speedup NAO e gravado: e derivado no plot (tempo[1 thread] / tempo[N]).
#
# Uso:
#   sh scripts/run_bench.sh [dir_inputs] [csv_saida]
# Variaveis de ambiente (com padrao):
#   THREADS="1 2 4 8 16"   lista de contagens de threads a medir
#   BLOCK=1048576          tamanho de bloco (--block-size), em bytes (1 MiB)
#   REPS=3                 repeticoes por ponto (o plot usa a mediana)
#
# IMPORTANTE (RULES REGRA 7 - dados reais): o speedup so aparece quando o czip
# roda CONCORRENTE, o que exige o pipeline com pthreads (Linux/WSL). No Windows
# (MinGW sem libpthread) o czip cai no caminho SEQUENCIAL e todas as colunas de
# threads dao ~o mesmo tempo (speedup ~1) - util so para validar a mecanica do
# script. A coleta valida para o relatorio e feita em WSL/Linux.
# ============================================================================
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
INDIR="${1:-$ROOT/inputs}"
CSV="${2:-$ROOT/results/resultados.csv}"

THREADS="${THREADS:-1 2 4 8 16}"
BLOCK="${BLOCK:-1048576}"
REPS="${REPS:-3}"

CZIP="$ROOT/czip"
CUNZIP="$ROOT/cunzip"
if { [ ! -x "$CZIP" ] && [ ! -x "$CZIP.exe" ]; } \
   || { [ ! -x "$CUNZIP" ] && [ ! -x "$CUNZIP.exe" ]; }; then
    echo "run_bench: czip/cunzip nao encontrados; rode 'make all' antes." >&2
    exit 1
fi
if [ ! -d "$INDIR" ]; then
    echo "run_bench: dir de entradas '$INDIR' nao existe; rode gen_inputs.sh antes." >&2
    exit 1
fi

mkdir -p "$(dirname "$CSV")"

# Classifica o tipo do arquivo pelo nome (so para rotular os graficos).
tipo_de() {
    case "$1" in
        text*)     echo "texto" ;;
        logs*)     echo "logs" ;;
        binary*)   echo "binario" ;;
        repeated*) echo "repetido" ;;
        random*)   echo "aleatorio" ;;
        fire*)     echo "fogo" ;;
        *)         echo "outro" ;;
    esac
}

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT INT TERM

echo "arquivo,tipo,tamanho_bytes,block_size,threads,rep,tempo_s,tamanho_cz_bytes,taxa_pct,throughput_mibs,ok" > "$CSV"

NPROC="$(nproc 2>/dev/null || echo '?')"
echo "run_bench: nucleos=$NPROC, threads='$THREADS', block=$BLOCK, reps=$REPS"
echo "run_bench: entradas em '$INDIR' -> CSV '$CSV'"

for src in "$INDIR"/*; do
    [ -f "$src" ] || continue
    name="$(basename "$src")"
    case "$name" in .*) continue ;; esac   # ignora ocultos/temporarios
    tipo="$(tipo_de "$name")"
    size="$(wc -c < "$src")"
    [ "$size" -gt 0 ] || continue

    for t in $THREADS; do
        rep=1
        while [ "$rep" -le "$REPS" ]; do
            cz="$TMP/out.cz"
            out="$TMP/out.bin"

            t0="$(date +%s.%N)"
            "$CZIP" --threads "$t" --block-size "$BLOCK" "$src" "$cz" >/dev/null 2>&1
            t1="$(date +%s.%N)"

            czsize="$(wc -c < "$cz")"

            # roundtrip de correcao (nao entra no tempo medido).
            ok=0
            if "$CUNZIP" "$cz" "$out" >/dev/null 2>&1 && cmp -s "$src" "$out"; then
                ok=1
            fi

            # Matematica de ponto flutuante com awk (sem depender de bc).
            line="$(awk -v a="$t0" -v b="$t1" -v sz="$size" -v cz="$czsize" \
                        -v f="$name" -v tp="$tipo" -v bk="$BLOCK" -v th="$t" \
                        -v rp="$rep" -v ok="$ok" 'BEGIN{
                dt = b - a; if (dt <= 0) dt = 0.000001;
                taxa = 100.0 * (1.0 - cz / sz);
                thr  = (sz / dt) / 1048576.0;
                printf "%s,%s,%d,%d,%d,%d,%.6f,%d,%.2f,%.2f,%d",
                       f, tp, sz, bk, th, rp, dt, cz, taxa, thr, ok;
            }')"
            echo "$line" >> "$CSV"

            rep=$((rep + 1))
        done
    done
    echo "run_bench: $name ($tipo, $size bytes) OK"
done

echo "run_bench: concluido -> $CSV"
