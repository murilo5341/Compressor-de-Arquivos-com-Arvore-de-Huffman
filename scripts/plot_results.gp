# ============================================================================
# plot_results.gp - Script gnuplot dos graficos do relatorio (Modulo 18).
#
# NAO chame este arquivo direto: ele e invocado por scripts/plot_results.sh, que
# agrega o CSV e passa duas variaveis:
#   work   - diretorio temporario com series_<i>.dat, manifest_files,
#            manifest_labels e taxa.dat (gerados pelo awk do wrapper)
#   outdir - diretorio onde salvar os PNGs
#
# Ex. (feito pelo wrapper):
#   gnuplot -e "work='/tmp/xxx'; outdir='results/graphs'" scripts/plot_results.gp
#
# Cada series_<i>.dat tem as colunas:  threads  tempo_s  throughput_mibs  speedup
# taxa.dat tem:                        tipo     taxa_pct
# ============================================================================

set datafile separator whitespace
set terminal pngcairo size 1000,650 font ",11"
set grid

# Listas (espaco-separadas) de arquivos .dat e seus rotulos, geradas pelo wrapper.
series = system(sprintf("cat %s/manifest_files",  work))
labels = system(sprintf("cat %s/manifest_labels", work))
n = words(series)

# ----------------------------------------------------------------------------
# 1) Speedup x threads  (com a reta ideal y = x = speedup linear)
# ----------------------------------------------------------------------------
set output sprintf("%s/speedup_vs_threads.png", outdir)
set title "Speedup x numero de threads"
set xlabel "threads"
set ylabel "speedup (tempo[base] / tempo[N])"
set key outside right top
plot for [i=1:n] word(series,i) using 1:4 with linespoints pt 7 lw 2 \
         title word(labels,i), \
     x with lines dt 2 lc rgb "black" title "ideal (linear)"

# ----------------------------------------------------------------------------
# 2) Tempo de compressao x threads
# ----------------------------------------------------------------------------
set output sprintf("%s/tempo_vs_threads.png", outdir)
set title "Tempo de compressao x numero de threads"
set xlabel "threads"
set ylabel "tempo (s)"
set key outside right top
plot for [i=1:n] word(series,i) using 1:2 with linespoints pt 7 lw 2 \
         title word(labels,i)

# ----------------------------------------------------------------------------
# 3) Throughput x threads
# ----------------------------------------------------------------------------
set output sprintf("%s/throughput_vs_threads.png", outdir)
set title "Vazao (throughput) x numero de threads"
set xlabel "threads"
set ylabel "throughput (MiB/s)"
set key outside right top
plot for [i=1:n] word(series,i) using 1:3 with linespoints pt 7 lw 2 \
         title word(labels,i)

# ----------------------------------------------------------------------------
# 4) Taxa de compressao por tipo de arquivo (barras)
# ----------------------------------------------------------------------------
set output sprintf("%s/taxa_por_tipo.png", outdir)
set title "Taxa de compressao por tipo de arquivo"
set xlabel "tipo de arquivo"
set ylabel "taxa de compressao (%)"
set style data histogram
set style fill solid 0.7 border -1
set boxwidth 0.8
unset key
set yrange [*:*]
plot sprintf("%s/taxa.dat", work) using 2:xtic(1) lc rgb "#4477AA"
