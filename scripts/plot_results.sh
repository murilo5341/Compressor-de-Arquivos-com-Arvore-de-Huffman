#!/bin/sh
# ============================================================================
# plot_results.sh - Graficos do relatorio experimental via gnuplot (Modulo 18).
#
# Le o CSV gerado por scripts/run_bench.sh (Modulo 17), AGREGA os pontos com awk
# e chama o gnuplot (scripts/plot_results.gp) para gerar 4 PNGs em results/graphs:
#
#   1. speedup_vs_threads.png    - speedup = tempo[baseline] / tempo[N], com a reta
#                                  ideal y = x (speedup linear) de referencia.
#   2. tempo_vs_threads.png      - tempo de compressao por numero de threads.
#   3. throughput_vs_threads.png - vazao (MiB/s) por numero de threads.
#   4. taxa_por_tipo.png         - taxa de compressao (%) por tipo de arquivo.
#
# AGREGACAO (RULES REGRA 7): para cada (arquivo, threads) usa-se o MENOR tempo das
# repeticoes (coluna rep) - o "melhor de N". Justificativa: interferencia do SO
# (escalonador, outros processos) so ADICIONA tempo, nunca reduz abaixo do custo
# real; o minimo e a estimativa mais limpa do tempo intrinseco. O throughput e
# derivado desse mesmo tempo (consistencia) e o speedup usa o menor numero de
# threads presente como baseline.
#
# Por que gnuplot e nao Python: o SISTEMA do Tema 11 e em C (czip/cunzip + ED/SO);
# o edital lista "scripts de geracao dos graficos" como artefato SEPARADO e a RULES
# REGRA 7 permite gnuplot. gnuplot e a ferramenta classica e nao acrescenta stack
# Python ao projeto. A agregacao usa awk (mawk-safe, ja usado no run_bench.sh).
#
# Uso:
#   sh scripts/plot_results.sh [csv] [dir_saida_dos_png]
#   (padrao: results/resultados.csv -> results/graphs/)
#
# Dependencia: gnuplot (apt install gnuplot). Sem ele, sai com codigo 2 e mensagem.
# ============================================================================
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
CSV="${1:-$ROOT/results/resultados.csv}"
OUTDIR="${2:-$ROOT/results/graphs}"
GP="$ROOT/scripts/plot_results.gp"

if [ ! -f "$CSV" ]; then
    echo "plot_results: CSV nao encontrado: $CSV" >&2
    echo "              rode scripts/run_bench.sh (ou make stress) antes." >&2
    exit 1
fi
if ! command -v gnuplot >/dev/null 2>&1; then
    echo "plot_results: gnuplot nao instalado. Rode: sudo apt install gnuplot" >&2
    exit 2
fi

mkdir -p "$OUTDIR"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT INT TERM

# --- AGREGACAO: CSV -> series_<i>.dat (threads tempo throughput speedup) + ------
# manifest_files / manifest_labels (para o gnuplot iterar) + taxa.dat (barras).
awk -v work="$WORK" -F, '
NR == 1 { next }                      # pula cabecalho
{
    arq = $1; tipo = $2; size = $3 + 0; th = $5 + 0; tempo = $7 + 0; taxa = $9 + 0;
    key = arq SUBSEP th;
    # menor tempo (melhor de N) por (arquivo, threads)
    if (!(key in tmin) || tempo < tmin[key]) tmin[key] = tempo;
    tipo_[arq] = tipo; size_[arq] = size; taxa_[arq] = taxa;
    # lista de arquivos na ordem de aparicao
    if (!(arq in seenF)) { seenF[arq] = 1; F[++nf] = arq; }
    # lista de threads (unica) por arquivo, na ordem de aparicao
    if (!(key in seenT)) { seenT[key] = 1; tc[arq]++; T[arq, tc[arq]] = th; }
}
END {
    mf = ""; ml = "";
    taxafile = work "/taxa.dat";
    for (i = 1; i <= nf; i++) {
        arq = F[i];
        # copia e ordena (insertion sort) os threads deste arquivo
        n = tc[arq];
        for (k = 1; k <= n; k++) a[k] = T[arq, k];
        for (x = 2; x <= n; x++) {
            kv = a[x]; y = x - 1;
            while (y >= 1 && a[y] > kv) { a[y+1] = a[y]; y--; }
            a[y+1] = kv;
        }
        base = a[1];
        bt = tmin[arq SUBSEP base];
        if (bt <= 0) bt = 1e-9;

        dat = work "/series_" i ".dat";
        printf "# threads tempo_s throughput_mibs speedup  (%s)\n", arq > dat;
        for (k = 1; k <= n; k++) {
            th = a[k];
            t = tmin[arq SUBSEP th];
            if (t <= 0) t = 1e-9;
            thr = (size_[arq] / t) / 1048576.0;
            sp = bt / t;
            printf "%d %.6f %.3f %.4f\n", th, t, thr, sp > dat;
        }
        close(dat);
        mf = mf (mf == "" ? "" : " ") dat;
        ml = ml (ml == "" ? "" : " ") arq;

        # uma barra por arquivo, rotulada pelo tipo
        printf "%s %.2f\n", tipo_[arq], taxa_[arq] > taxafile;
    }
    close(taxafile);
    print mf > (work "/manifest_files");
    print ml > (work "/manifest_labels");
}
' "$CSV"

if [ ! -s "$WORK/manifest_files" ]; then
    echo "plot_results: CSV sem dados (so cabecalho?)." >&2
    exit 1
fi

gnuplot -e "work='$WORK'; outdir='$OUTDIR'" "$GP"

echo "plot_results: graficos gerados em '$OUTDIR':"
for p in speedup_vs_threads tempo_vs_threads throughput_vs_threads taxa_por_tipo; do
    if [ -f "$OUTDIR/$p.png" ]; then
        echo "  - $p.png"
    fi
done
