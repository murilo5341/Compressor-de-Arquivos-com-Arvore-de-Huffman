#!/bin/sh
# ============================================================================
# test_pipeline.sh - Teste de integracao do escritor REORDENADOR (Modulo 14).
#
# O pipeline concorrente (Modulo 13) comprime blocos em N threads, que terminam
# FORA DE ORDEM. O escritor reordenador (writer_thread em src/pipeline.c) precisa
# grava-los na ORDEM correta, de modo que a saida nao dependa do escalonador.
#
# Este teste comprova isso de duas formas:
#   (1) DETERMINISMO DO REORDER: comprimir o MESMO arquivo com N threads e com 1
#       thread deve gerar arquivos .cz BYTE A BYTE identicos. Se a reordenacao
#       falhasse, a versao paralela sairia com blocos trocados -> diff.
#   (2) ROUNDTRIP: descomprimir a saida paralela e comparar com o original.
#
# Usa block-size pequeno e entrada de ~1 MiB para forcar MUITOS blocos (centenas)
# e maximizar a chance de termino fora de ordem entre as codificadoras.
#
# Requer pthreads (czip ligado ao pipeline) -> executar em Linux. E invocado pelo
# alvo `make test_pipeline` (entra em CONC_TESTS so em Linux).
# ============================================================================
set -eu

CZIP=./czip
CUNZIP=./cunzip

if [ ! -x "$CZIP" ] || [ ! -x "$CUNZIP" ]; then
    echo "test_pipeline: czip/cunzip nao encontrados; rode 'make all' antes." >&2
    exit 1
fi

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT INT TERM

IN="$TMP/in.bin"
CZ_N="$TMP/par.cz"
CZ_1="$TMP/seq.cz"
OUT="$TMP/out.bin"

BLOCK=4096   # block-size pequeno -> muitos blocos
SIZE=1048576 # ~1 MiB de entrada (256 blocos de 4 KiB)

# Dados pseudo-aleatorios (incompressiveis): exercitam o pipeline com payloads
# de tamanhos variados, sem depender do conteudo.
head -c "$SIZE" /dev/urandom > "$IN"

# (1) reorder deterministico: paralelo (8 threads) deve sair igual ao sequencial.
"$CZIP" --threads 8 --block-size "$BLOCK" "$IN" "$CZ_N"
"$CZIP" --threads 1 --block-size "$BLOCK" "$IN" "$CZ_1"

if ! cmp -s "$CZ_N" "$CZ_1"; then
    echo "test_pipeline: FALHA - .cz com 8 threads difere do .cz com 1 thread" >&2
    echo "               (reordenacao incorreta: blocos fora de ordem na saida)." >&2
    exit 1
fi

# (2) roundtrip da saida paralela: deve reconstruir o original byte a byte.
"$CUNZIP" "$CZ_N" "$OUT"
if ! cmp -s "$IN" "$OUT"; then
    echo "test_pipeline: FALHA - roundtrip do .cz paralelo difere do original." >&2
    exit 1
fi

echo "test_pipeline: OK - reorder deterministico (8 vs 1 thread identicos) + roundtrip (${SIZE} bytes, block-size ${BLOCK})."
