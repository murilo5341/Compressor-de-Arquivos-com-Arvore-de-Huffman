#!/bin/sh
# ============================================================================
# test_roundtrip.sh - Suite de roundtrip e edge-cases (Modulo 15, RULES REGRA 10).
#
# Prova, de ponta a ponta, que `czip` seguido de `cunzip` reconstroi cada arquivo
# BYTE A BYTE (cmp), cobrindo todos os casos de borda obrigatorios do edital:
#   arquivo vazio, um unico byte, um unico simbolo repetido, todos os 256 valores
#   de byte, texto, binario pequeno e aleatorio.
# Os arquivos sao gerados por scripts/gen_edge_cases.sh.
#
# Cada caso roda com MULTIPLOS block-sizes (incluindo um bem pequeno) para
# exercitar tanto o arquivo de um unico bloco quanto a divisao em muitos blocos.
#
# Inclui ainda o caso de CORRUPCAO DA ARVORE SERIALIZADA / METADADOS do bloco
# (distinto da corrupcao de PAYLOAD, que e o Modulo 16): um byte e alterado na
# regiao da arvore e, em outra rodada, num campo de metadados; em ambos o cunzip
# deve REPORTAR o erro e sair SEM CRASH (exit code de erro normal, nunca sinal).
#
# Diferente do test_pipeline.sh (so Linux, exige pthreads), este teste usa apenas
# o czip/cunzip SEQUENCIAL e roda tambem no Windows/MSYS2.
# ============================================================================
set -eu

# Diretorio raiz do projeto (este script vive em tests/).
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
CZIP="$ROOT/czip"
CUNZIP="$ROOT/cunzip"

if [ ! -x "$CZIP" ] && [ ! -x "$CZIP.exe" ]; then
    echo "test_roundtrip: czip nao encontrado; rode 'make all' antes." >&2
    exit 1
fi
if [ ! -x "$CUNZIP" ] && [ ! -x "$CUNZIP.exe" ]; then
    echo "test_roundtrip: cunzip nao encontrado; rode 'make all' antes." >&2
    exit 1
fi

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT INT TERM

CASES="$TMP/casos"
sh "$ROOT/scripts/gen_edge_cases.sh" "$CASES" >/dev/null

fail=0

# ---------------------------------------------------------------------------
# (1) ROUNDTRIP BYTE A BYTE de cada caso, com varios tamanhos de bloco.
# block-size 7 é proposital: pequeno e nao-potencia-de-2, forca muitos blocos
# e ultimo bloco parcial. 65536 é o padrao de producao.
# ---------------------------------------------------------------------------
for f in empty single_byte repeated_symbol all_256 text binary_small random; do
    src="$CASES/$f"
    for bs in 7 64 65536; do
        cz="$TMP/$f.$bs.cz"
        out="$TMP/$f.$bs.out"
        if ! "$CZIP" --block-size "$bs" "$src" "$cz" >/dev/null 2>"$TMP/err"; then
            echo "test_roundtrip: FALHA - czip falhou em '$f' (block-size=$bs)." >&2
            cat "$TMP/err" >&2
            fail=1
            continue
        fi
        if ! "$CUNZIP" "$cz" "$out" >/dev/null 2>"$TMP/err"; then
            echo "test_roundtrip: FALHA - cunzip falhou em '$f' (block-size=$bs)." >&2
            cat "$TMP/err" >&2
            fail=1
            continue
        fi
        if ! cmp -s "$src" "$out"; then
            echo "test_roundtrip: FALHA - roundtrip difere do original em '$f' (block-size=$bs)." >&2
            fail=1
            continue
        fi
    done
done
[ "$fail" -eq 0 ] && echo "test_roundtrip: OK - roundtrip byte a byte dos 7 casos x 3 block-sizes."

# ---------------------------------------------------------------------------
# (2) CORRUPCAO DA ARVORE / METADADOS -> erro reportado SEM CRASH (Modulo 15).
#
# Layout do .cz (Modulo 9), em bytes a partir do offset 0:
#   [0..19]  cabecalho global  (magic[4] + version + block_size + block_count)
#   [20..43] cabecalho do 1o bloco (block_index..original_crc32, 24 bytes), sendo
#            offset 32 = compressed_size, offset 36 = tree_size
#   [44..]   arvore serializada do 1o bloco, depois o payload
# Usa-se um arquivo de simbolo unico (arvore = folha unica, 2 bytes) para que a
# corrupcao caia com certeza na arvore/metadados, nao no payload.
# ---------------------------------------------------------------------------
single="$TMP/corrupt_src"
printf 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA' > "$single"

# Um cunzip "saudavel" em entrada corrompida deve: (a) terminar (sem travar),
# (b) sinalizar erro (exit != 0) e (c) NAO crashar (exit < 128; >=128 = sinal).
assert_graceful() {
    desc="$1"; cz="$2"; off="$3"; nbytes="$4"
    # Sobrescreve nbytes byte(s) com 0xFF a partir de off (corrompe sem truncar).
    j=0
    while [ "$j" -lt "$nbytes" ]; do
        printf '\377' | dd of="$cz" bs=1 seek=$((off + j)) count=1 conv=notrunc 2>/dev/null
        j=$((j + 1))
    done
    set +e
    "$CUNZIP" "$cz" "$TMP/corrupt.out" >/dev/null 2>"$TMP/err"
    code=$?
    set -e
    if [ "$code" -ge 128 ]; then
        echo "test_roundtrip: FALHA - $desc: cunzip CRASHOU (exit=$code, sinal)." >&2
        fail=1
    elif [ "$code" -eq 0 ]; then
        echo "test_roundtrip: FALHA - $desc: cunzip nao reportou erro (exit=0)." >&2
        fail=1
    fi
}

# (2a) corromper a ARVORE serializada (offset 44).
"$CZIP" "$single" "$TMP/c_tree.cz" >/dev/null 2>&1
assert_graceful "arvore serializada (off 44)" "$TMP/c_tree.cz" 44 1

# (2b) corromper METADADOS: tree_size (offset 36) com valor enorme.
"$CZIP" "$single" "$TMP/c_meta.cz" >/dev/null 2>&1
assert_graceful "metadados tree_size (off 36)" "$TMP/c_meta.cz" 36 4

[ "$fail" -eq 0 ] && echo "test_roundtrip: OK - corrupcao de arvore/metadados reportada sem crash."

# ---------------------------------------------------------------------------
if [ "$fail" -ne 0 ]; then
    echo "test_roundtrip: FALHOU." >&2
    exit 1
fi
echo "test_roundtrip: TODOS OS CASOS PASSARAM."
