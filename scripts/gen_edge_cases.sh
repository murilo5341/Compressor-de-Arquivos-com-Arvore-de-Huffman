#!/bin/sh
# ============================================================================
# gen_edge_cases.sh - Gera os arquivos de teste obrigatorios da RULES REGRA 10.
#
# Cria, no diretorio passado como argumento, um arquivo por caso de borda exigido
# pelo edital/RULES para o roundtrip de czip/cunzip. E consumido por
# tests/test_roundtrip.sh (Modulo 15), mas pode ser rodado isolado para inspecao:
#
#     sh scripts/gen_edge_cases.sh /tmp/casos
#     ls -l /tmp/casos
#
# Casos gerados (nome -> conteudo):
#   empty            -> arquivo vazio (0 bytes)
#   single_byte      -> um unico byte
#   repeated_symbol  -> um unico simbolo repetido (arvore vira folha unica)
#   all_256          -> todos os 256 valores possiveis de byte (0..255)
#   text             -> texto natural com repeticao tipica de linguagem
#   binary_small     -> binario pequeno com NULs e bytes de controle
#   random           -> dados pseudo-aleatorios (praticamente incompressiveis)
#
# Portabilidade: usa apenas printf (escapes octais), head e /dev/urandom, todos
# disponiveis no MSYS2 (Windows) e no Linux.
# ============================================================================
set -eu

OUTDIR="${1:?uso: gen_edge_cases.sh <diretorio_de_saida>}"
mkdir -p "$OUTDIR"

# --- arquivo vazio ----------------------------------------------------------
: > "$OUTDIR/empty"

# --- um unico byte ----------------------------------------------------------
printf 'A' > "$OUTDIR/single_byte"

# --- um unico simbolo repetido (forca o caso de FOLHA UNICA da arvore) ------
i=0
: > "$OUTDIR/repeated_symbol"
while [ "$i" -lt 5000 ]; do
    printf 'Z' >> "$OUTDIR/repeated_symbol"
    i=$((i + 1))
done

# --- todos os 256 valores de byte (0..255), cada um 4x para dar volume ------
# printf '\NNN' grava o byte de valor octal NNN (255 = 0377).
: > "$OUTDIR/all_256"
rep=0
while [ "$rep" -lt 4 ]; do
    i=0
    while [ "$i" -lt 256 ]; do
        oct=$(printf '%03o' "$i")
        printf "\\$oct" >> "$OUTDIR/all_256"
        i=$((i + 1))
    done
    rep=$((rep + 1))
done

# --- texto natural (repeticao tipica de linguagem -> boa compressao) --------
{
    n=0
    while [ "$n" -lt 200 ]; do
        printf 'O rato roeu a roupa do rei de Roma e a rainha raivosa rasgou o resto.\n'
        n=$((n + 1))
    done
} > "$OUTDIR/text"

# --- binario pequeno: NULs, bytes de controle e alto bit setado -------------
printf '\000\001\002\003\377\376\200\177\012\000\000\125\252\125\252' \
    > "$OUTDIR/binary_small"

# --- aleatorio (incompressivel): exercita o pior caso de Huffman ------------
head -c 65536 /dev/urandom > "$OUTDIR/random"

echo "gen_edge_cases: gerados 7 casos em '$OUTDIR'."
