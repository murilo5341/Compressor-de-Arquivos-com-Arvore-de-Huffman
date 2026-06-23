#!/usr/bin/env python3
# ============================================================================
# plot_results.py - Graficos do relatorio experimental (Modulo 18).
#
# Le o CSV gerado por scripts/run_bench.sh (Modulo 17) e produz os graficos
# exigidos pela RULES REGRA 7, confrontando teoria x pratica:
#
#   1. speedup_vs_threads.png    - speedup = tempo[1 thread] / tempo[N], com a
#                                  reta ideal y = x (speedup linear) de referencia.
#   2. tempo_vs_threads.png      - tempo de compressao por numero de threads.
#   3. throughput_vs_threads.png - vazao (MiB/s) por numero de threads.
#   4. taxa_por_tipo.png         - taxa de compressao (%) por tipo de arquivo.
#
# O tempo de cada ponto (arquivo, threads) e a MEDIANA das repeticoes (coluna rep),
# reduzindo o ruido de medicao. O speedup so sera "quase linear" quando o CSV vier
# de uma execucao CONCORRENTE real (pipeline com pthreads, Linux/WSL); num CSV
# sequencial (Windows) o speedup fica ~1, o que o proprio grafico evidencia.
#
# Uso:
#   python scripts/plot_results.py [csv] [dir_saida_dos_png]
#   (padrao: results/resultados.csv  ->  results/graphs/)
#
# Dependencia: matplotlib (RULES REGRA 7 permite matplotlib/gnuplot). Instale com
#   pip install matplotlib
# A leitura/agregacao do CSV usa so a biblioteca padrao (csv), entao a logica de
# dados e testavel mesmo sem matplotlib instalado.
# ============================================================================
import csv
import os
import sys
from collections import defaultdict


def load_rows(csv_path):
    """Le o CSV e devolve uma lista de dicts com os campos ja convertidos."""
    rows = []
    with open(csv_path, newline="") as fh:
        for r in csv.DictReader(fh):
            rows.append({
                "arquivo": r["arquivo"],
                "tipo": r["tipo"],
                "tamanho_bytes": int(r["tamanho_bytes"]),
                "block_size": int(r["block_size"]),
                "threads": int(r["threads"]),
                "rep": int(r["rep"]),
                "tempo_s": float(r["tempo_s"]),
                "tamanho_cz_bytes": int(r["tamanho_cz_bytes"]),
                "taxa_pct": float(r["taxa_pct"]),
                "throughput_mibs": float(r["throughput_mibs"]),
                "ok": int(r["ok"]),
            })
    return rows


def _median(values):
    s = sorted(values)
    n = len(s)
    if n == 0:
        return 0.0
    mid = n // 2
    if n % 2:
        return s[mid]
    return (s[mid - 1] + s[mid]) / 2.0


def aggregate(rows):
    """Agrega por (arquivo, threads): mediana de tempo/throughput.

    Devolve:
      per_file[arquivo] = {
          "tipo": str,
          "threads": [t ordenados],
          "tempo": {t: mediana_tempo},
          "throughput": {t: mediana_throughput},
          "speedup": {t: tempo[base] / tempo[t]},
          "taxa": float,            # taxa de compressao (independe de threads)
      }
    """
    tempos = defaultdict(list)        # (arquivo, threads) -> [tempo]
    thrs = defaultdict(list)          # (arquivo, threads) -> [throughput]
    tipo = {}
    taxa = {}
    threadset = defaultdict(set)
    for r in rows:
        key = (r["arquivo"], r["threads"])
        tempos[key].append(r["tempo_s"])
        thrs[key].append(r["throughput_mibs"])
        tipo[r["arquivo"]] = r["tipo"]
        taxa[r["arquivo"]] = r["taxa_pct"]
        threadset[r["arquivo"]].add(r["threads"])

    per_file = {}
    for arquivo, tset in threadset.items():
        ts = sorted(tset)
        tempo = {t: _median(tempos[(arquivo, t)]) for t in ts}
        throughput = {t: _median(thrs[(arquivo, t)]) for t in ts}
        base = ts[0]  # menor contagem de threads = baseline do speedup
        base_t = tempo[base] if tempo[base] > 0 else 1e-9
        speedup = {t: (base_t / tempo[t] if tempo[t] > 0 else 0.0) for t in ts}
        per_file[arquivo] = {
            "tipo": tipo[arquivo],
            "threads": ts,
            "tempo": tempo,
            "throughput": throughput,
            "speedup": speedup,
            "taxa": taxa[arquivo],
        }
    return per_file


def make_plots(per_file, outdir):
    """Gera os 4 PNGs. Requer matplotlib."""
    import matplotlib
    matplotlib.use("Agg")  # backend sem display (roda em servidor/CI/WSL).
    import matplotlib.pyplot as plt

    os.makedirs(outdir, exist_ok=True)
    files = sorted(per_file)

    # --- 1. speedup vs threads -------------------------------------------
    fig, ax = plt.subplots()
    all_threads = sorted({t for f in files for t in per_file[f]["threads"]})
    for f in files:
        d = per_file[f]
        ax.plot(d["threads"], [d["speedup"][t] for t in d["threads"]],
                marker="o", label=f)
    if all_threads:
        ax.plot(all_threads, all_threads, "k--", alpha=0.5, label="ideal (linear)")
    ax.set_xlabel("threads")
    ax.set_ylabel("speedup (t1 / tN)")
    ax.set_title("Speedup x numero de threads")
    ax.legend(fontsize="small")
    ax.grid(True, alpha=0.3)
    fig.savefig(os.path.join(outdir, "speedup_vs_threads.png"), dpi=120,
                bbox_inches="tight")
    plt.close(fig)

    # --- 2. tempo vs threads ---------------------------------------------
    fig, ax = plt.subplots()
    for f in files:
        d = per_file[f]
        ax.plot(d["threads"], [d["tempo"][t] for t in d["threads"]],
                marker="o", label=f)
    ax.set_xlabel("threads")
    ax.set_ylabel("tempo de compressao (s)")
    ax.set_title("Tempo x numero de threads")
    ax.legend(fontsize="small")
    ax.grid(True, alpha=0.3)
    fig.savefig(os.path.join(outdir, "tempo_vs_threads.png"), dpi=120,
                bbox_inches="tight")
    plt.close(fig)

    # --- 3. throughput vs threads ----------------------------------------
    fig, ax = plt.subplots()
    for f in files:
        d = per_file[f]
        ax.plot(d["threads"], [d["throughput"][t] for t in d["threads"]],
                marker="o", label=f)
    ax.set_xlabel("threads")
    ax.set_ylabel("throughput (MiB/s)")
    ax.set_title("Vazao x numero de threads")
    ax.legend(fontsize="small")
    ax.grid(True, alpha=0.3)
    fig.savefig(os.path.join(outdir, "throughput_vs_threads.png"), dpi=120,
                bbox_inches="tight")
    plt.close(fig)

    # --- 4. taxa de compressao por tipo ----------------------------------
    fig, ax = plt.subplots()
    labels = [per_file[f]["tipo"] for f in files]
    valores = [per_file[f]["taxa"] for f in files]
    ax.bar(labels, valores)
    ax.set_xlabel("tipo de arquivo")
    ax.set_ylabel("taxa de compressao (%)")
    ax.set_title("Taxa de compressao por tipo")
    ax.grid(True, axis="y", alpha=0.3)
    fig.savefig(os.path.join(outdir, "taxa_por_tipo.png"), dpi=120,
                bbox_inches="tight")
    plt.close(fig)

    return ["speedup_vs_threads.png", "tempo_vs_threads.png",
            "throughput_vs_threads.png", "taxa_por_tipo.png"]


def main(argv):
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.dirname(here)
    csv_path = argv[1] if len(argv) > 1 else os.path.join(root, "results", "resultados.csv")
    outdir = argv[2] if len(argv) > 2 else os.path.join(root, "results", "graphs")

    if not os.path.exists(csv_path):
        print("plot_results: CSV nao encontrado: %s" % csv_path, file=sys.stderr)
        print("              rode scripts/run_bench.sh antes.", file=sys.stderr)
        return 1

    rows = load_rows(csv_path)
    if not rows:
        print("plot_results: CSV vazio (so cabecalho?).", file=sys.stderr)
        return 1

    per_file = aggregate(rows)
    try:
        pngs = make_plots(per_file, outdir)
    except ImportError:
        print("plot_results: matplotlib nao instalado. Rode: pip install matplotlib",
              file=sys.stderr)
        return 2

    print("plot_results: %d graficos gerados em '%s':" % (len(pngs), outdir))
    for p in pngs:
        print("  - %s" % p)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
