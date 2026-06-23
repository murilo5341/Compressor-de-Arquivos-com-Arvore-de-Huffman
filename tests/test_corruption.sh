#!/bin/sh
# ============================================================================
# test_corruption.sh - Corrupcao de PAYLOAD + recuperacao parcial (Modulo 16).
#
# Cumpre a parte do teste de fogo que exige DETECCAO de bloco corrompido e
# RECUPERACAO PARCIAL (RULES REGRA 5/10): um unico byte do PAYLOAD comprimido de
# um bloco do meio do .cz e alterado e o cunzip deve:
#   (1) DETECTAR o bloco corrompido por CRC32 (CRC do conteudo restaurado diverge)
#       e descarta-lo, terminando com exit != 0 e SEM crash (exit < 128);
#   (2) continuar descomprimindo TODOS os demais blocos corretamente.
#
# A prova de (2) e forte: reconstroi-se o arquivo ESPERADO = original com os bytes
# do bloco corrompido removidos e compara-se byte a byte (cmp) com a saida real do
# cunzip. Se algum outro bloco tivesse sido afetado, o cmp falharia.
#
# Distincao do Modulo 15: la corrompe-se a ARVORE/METADADOS (block_decompress nem
# roda direito); aqui corrompe-se o PAYLOAD (decodifica, mas o CRC32 acusa).
#
# Usa apenas o czip/cunzip SEQUENCIAL -> roda no Windows/MSYS2 e no Linux.
# Layout do header de bloco (Modulo 9, 24 bytes a partir do inicio do bloco):
#   +0  block_index (u64)   +8  original_size (u32)   +12 compressed_size (u32)
#   +16 tree_size (u32)     +20 original_crc32 (u32)
#   depois: tree_bytes[tree_size], depois payload[compressed_size].
# ============================================================================
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
CZIP="$ROOT/czip"
CUNZIP="$ROOT/cunzip"

if { [ ! -x "$CZIP" ] && [ ! -x "$CZIP.exe" ]; } \
   || { [ ! -x "$CUNZIP" ] && [ ! -x "$CUNZIP.exe" ]; }; then
    echo "test_corruption: czip/cunzip nao encontrados; rode 'make all' antes." >&2
    exit 1
fi

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT INT TERM

IN="$TMP/in.txt"
CZ="$TMP/in.cz"
OUT="$TMP/out.txt"
EXP="$TMP/expected.txt"

# Le um uint32 little-endian no offset $2 do arquivo $1 (via od, portavel).
ru32() {
    set -- $(od -An -tu1 -j "$2" -N1 -- "$1") \
          $(od -An -tu1 -j "$(( $2 + 1 ))" -N1 -- "$1") \
          $(od -An -tu1 -j "$(( $2 + 2 ))" -N1 -- "$1") \
          $(od -An -tu1 -j "$(( $2 + 3 ))" -N1 -- "$1")
    echo $(( $1 + $2 * 256 + $3 * 65536 + $4 * 16777216 ))
}

# --- entrada: texto repetitivo grande -> muitos blocos ----------------------
BLOCK=1024
n=0
: > "$IN"
while [ "$n" -lt 2000 ]; do
    printf 'O rato roeu a roupa do rei de Roma e a rainha rasgou o resto da racao.\n' >> "$IN"
    n=$((n + 1))
done

"$CZIP" --block-size "$BLOCK" "$IN" "$CZ" >/dev/null

# --- localizar o bloco do MEIO percorrendo os cabecalhos --------------------
BS=$(ru32 "$CZ" 8)        # block_size do cabecalho global (offset 8)
BC=$(ru32 "$CZ" 12)       # block_count (low 32 bits bastam aqui)
if [ "$BC" -lt 4 ]; then
    echo "test_corruption: FALHA - esperados varios blocos, obtidos $BC." >&2
    exit 1
fi
TARGET=$((BC / 2))

off=20                    # fim do cabecalho global
idx=0
P_OFF=0; T_ORIG=0
while [ "$idx" -lt "$BC" ]; do
    orig=$(ru32 "$CZ" $((off + 8)))
    comp=$(ru32 "$CZ" $((off + 12)))
    tree=$(ru32 "$CZ" $((off + 16)))
    poff=$((off + 24 + tree))          # inicio do payload deste bloco
    if [ "$idx" -eq "$TARGET" ]; then
        P_OFF=$poff
        T_ORIG=$orig                   # tamanho original do bloco-alvo (= BS, bloco cheio)
        T_START=$((idx * BS))          # offset desse bloco no arquivo ORIGINAL
    fi
    off=$((poff + comp))               # inicio do proximo bloco
    idx=$((idx + 1))
done

# --- corromper o PRIMEIRO byte do payload do bloco-alvo (bits de codigo reais) ---
# Inverte o byte (val XOR 0xFF) garantindo alteracao; conv=notrunc nao muda o tamanho.
cur=$(od -An -tu1 -j "$P_OFF" -N1 -- "$CZ")
new=$(( cur ^ 255 ))
printf "\\$(printf '%03o' "$new")" | dd of="$CZ" bs=1 seek="$P_OFF" count=1 conv=notrunc 2>/dev/null

# --- rodar cunzip: deve detectar (exit != 0) sem crash (exit < 128) ---------
set +e
"$CUNZIP" "$CZ" "$OUT" >/dev/null 2>"$TMP/err"
code=$?
set -e

fail=0
if [ "$code" -ge 128 ]; then
    echo "test_corruption: FALHA - cunzip CRASHOU (exit=$code, sinal)." >&2
    fail=1
elif [ "$code" -eq 0 ]; then
    echo "test_corruption: FALHA - corrupcao NAO detectada (cunzip exit=0)." >&2
    fail=1
fi

# --- recuperacao parcial: saida == original SEM os bytes do bloco-alvo ------
# Bloco-alvo cobre [T_START, T_START + T_ORIG) no arquivo original.
head -c "$T_START" "$IN" > "$EXP"
tail -c +"$(( T_START + T_ORIG + 1 ))" "$IN" >> "$EXP"

if ! cmp -s "$EXP" "$OUT"; then
    echo "test_corruption: FALHA - demais blocos nao batem com o original (recuperacao parcial incorreta)." >&2
    echo "  esperado=$(wc -c < "$EXP") bytes, obtido=$(wc -c < "$OUT") bytes." >&2
    fail=1
fi

if [ "$fail" -ne 0 ]; then
    echo "--- stderr do cunzip ---" >&2
    cat "$TMP/err" >&2
    echo "test_corruption: FALHOU." >&2
    exit 1
fi

echo "test_corruption: OK - bloco $TARGET/$BC corrompido no payload detectado por CRC32;"
echo "                 demais $((BC - 1)) blocos restaurados byte a byte (exit=$code)."
