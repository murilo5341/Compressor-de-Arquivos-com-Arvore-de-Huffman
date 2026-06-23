#!/bin/sh
# ============================================================================
# gen_inputs.sh - Gera os arquivos de entrada do benchmark (Modulo 17).
#
# Cria, no diretorio de saida, um conjunto de arquivos de TIPOS diferentes para
# medir taxa de compressao e throughput (RULES REGRA 7), mais o arquivo grande do
# TESTE DE FOGO (~1 GiB) usado para medir speedup com 1/2/4/8/16 threads.
#
# Uso:
#   sh scripts/gen_inputs.sh <dir_saida> [tamanho_bench] [tamanho_fogo]
#
#   tamanho_bench : bytes de cada arquivo de tipo (padrao 33554432 = 32 MiB).
#   tamanho_fogo  : bytes do arquivo do teste de fogo (padrao 1073741824 = 1 GiB).
#                   Passe 0 para PULAR o arquivo de 1 GiB (gera so os de bench).
#
# Exemplos:
#   sh scripts/gen_inputs.sh inputs                 # 32 MiB cada + 1 GiB de fogo
#   sh scripts/gen_inputs.sh inputs 4194304 0       # so bench de 4 MiB, sem fogo
#
# Tipos gerados:
#   text.txt    -> texto natural repetitivo (boa compressao)
#   logs.txt    -> linhas de log sinteticas (compressao media)
#   binary.bin  -> binario estruturado (cabecalhos + ruido)
#   repeated.bin-> um unico byte repetido (melhor caso: folha unica)
#   random.bin  -> bytes pseudo-aleatorios (pior caso, incompressivel)
#   fire_1g.bin -> arquivo grande do teste de fogo (texto, compressivel)
#
# Portabilidade: printf/head/dd/cat/wc, presentes no MSYS2 e no Linux. Os arquivos
# grandes sao montados por DUPLICACAO (cat a$ a > b) para crescer em O(log n)
# passos, em vez de byte a byte.
# ============================================================================
set -eu

OUTDIR="${1:?uso: gen_inputs.sh <dir_saida> [tamanho_bench] [tamanho_fogo]}"
BENCH_SIZE="${2:-33554432}"     # 32 MiB
FIRE_SIZE="${3:-1073741824}"    # 1 GiB
mkdir -p "$OUTDIR"

# Cresce o arquivo $1 por duplicacao ate ter pelo menos $2 bytes, depois trunca
# (via head -c) para exatamente $2 bytes em $3.
grow_to() {
    seed="$1"; target="$2"; dest="$3"
    tmp="$OUTDIR/.grow.tmp"
    cp "$seed" "$tmp"
    while [ "$(wc -c < "$tmp")" -lt "$target" ]; do
        cat "$tmp" "$tmp" > "$tmp.2"
        mv "$tmp.2" "$tmp"
    done
    head -c "$target" "$tmp" > "$dest"
    rm -f "$tmp"
}

echo "gen_inputs: bench=$BENCH_SIZE bytes/arquivo, fogo=$FIRE_SIZE bytes -> '$OUTDIR'"

# --- texto natural (semente ~1 KiB, depois duplicada) -----------------------
seed="$OUTDIR/.seed_text"
: > "$seed"
n=0
while [ "$n" -lt 14 ]; do
    printf 'O rato roeu a roupa do rei de Roma enquanto a rainha raivosa rasgava o resto.\n' >> "$seed"
    n=$((n + 1))
done
grow_to "$seed" "$BENCH_SIZE" "$OUTDIR/text.txt"

# --- logs sinteticos --------------------------------------------------------
: > "$seed"
n=0
while [ "$n" -lt 12 ]; do
    printf '2026-06-23T10:%02d:00 INFO  worker=%d req=GET /api/v1/itens status=200 ms=%d\n' \
        "$n" "$((n % 8))" "$((n * 3 + 11))" >> "$seed"
    n=$((n + 1))
done
grow_to "$seed" "$BENCH_SIZE" "$OUTDIR/logs.txt"

# --- binario estruturado: blocos com cabecalho fixo + miolo pseudo-aleatorio -
seed="$OUTDIR/.seed_bin"
{
    printf '\211PNGfake\000\001\002\003'
    head -c 1024 /dev/urandom
    printf '\377\376\000\000DATA'
} > "$seed"
grow_to "$seed" "$BENCH_SIZE" "$OUTDIR/binary.bin"

# --- um unico byte repetido (melhor caso de Huffman: folha unica) -----------
# tr traduz os bytes do /dev/zero para 'Q'; head limita ao tamanho.
tr '\000' 'Q' < /dev/zero 2>/dev/null | head -c "$BENCH_SIZE" > "$OUTDIR/repeated.bin"

# --- aleatorio (incompressivel: pior caso) ----------------------------------
head -c "$BENCH_SIZE" /dev/urandom > "$OUTDIR/random.bin"

rm -f "$OUTDIR/.seed_text" "$OUTDIR/.seed_bin"

# --- arquivo do TESTE DE FOGO (~1 GiB de texto compressivel) ----------------
if [ "$FIRE_SIZE" -gt 0 ]; then
    echo "gen_inputs: gerando arquivo de fogo (${FIRE_SIZE} bytes) por duplicacao..."
    grow_to "$OUTDIR/text.txt" "$FIRE_SIZE" "$OUTDIR/fire_1g.bin"
else
    echo "gen_inputs: arquivo de fogo PULADO (tamanho_fogo=0)."
fi

echo "gen_inputs: concluido."
ls -l "$OUTDIR"
