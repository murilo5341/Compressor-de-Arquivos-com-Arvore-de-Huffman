/*
 * bitio.c - Implementacao da escrita e leitura bit a bit (Modulo 5).
 *
 * O empacotamento e MSB-first: dentro de cada byte, o primeiro bit escrito vai
 * para a posicao 7 (mais significativa) e o ultimo para a posicao 0. Assim o
 * byte 0xB0 (1011 0000) representa a sequencia de bits 1,0,1,1 seguida de quatro
 * zeros de padding. A leitura percorre os bits na mesma ordem.
 *
 * O escritor mantem um byte parcial 'current' enquanto ele nao completa 8 bits;
 * ao completar, o byte e anexado ao buffer dinamico. O buffer cresce dobrando a
 * capacidade (amortizado O(1) por byte). bit_writer_flush() descarrega o byte
 * parcial final, completando-o com zeros.
 */
#include "bitio.h"

#include <stdlib.h>
#include <string.h>

/* Capacidade inicial do buffer do escritor (bytes). Cresce dobrando. */
#define BITWRITER_CAPACIDADE_INICIAL 64

/* ------------------------------------------------------------------ */
/* BitWriter                                                           */
/* ------------------------------------------------------------------ */

bool bit_writer_init(BitWriter *w)
{
    if (w == NULL)
        return false;

    w->buffer = (uint8_t *)malloc(BITWRITER_CAPACIDADE_INICIAL);
    if (w->buffer == NULL) {
        /* Deixa o escritor num estado consistente mesmo na falha. */
        w->capacity   = 0;
        w->byte_count = 0;
        w->current    = 0;
        w->bit_count  = 0;
        w->error      = true;
        return false;
    }

    w->capacity   = BITWRITER_CAPACIDADE_INICIAL;
    w->byte_count = 0;
    w->current    = 0;
    w->bit_count  = 0;
    w->error      = false;
    return true;
}

/* Garante espaco para pelo menos mais um byte; cresce dobrando a capacidade. */
static bool bit_writer_ensure_capacity(BitWriter *w)
{
    if (w->byte_count < w->capacity)
        return true;

    size_t nova = (w->capacity == 0) ? BITWRITER_CAPACIDADE_INICIAL
                                     : w->capacity * 2;
    uint8_t *novo = (uint8_t *)realloc(w->buffer, nova);
    if (novo == NULL) {
        w->error = true;
        return false;
    }
    w->buffer   = novo;
    w->capacity = nova;
    return true;
}

/* Anexa o byte parcial completo ao buffer e reinicia o acumulador. */
static bool bit_writer_append_current(BitWriter *w)
{
    if (!bit_writer_ensure_capacity(w))
        return false;
    w->buffer[w->byte_count++] = w->current;
    w->current   = 0;
    w->bit_count = 0;
    return true;
}

bool bit_writer_write_bit(BitWriter *w, int bit)
{
    if (w == NULL || w->error)
        return false;

    /* Coloca o bit na posicao corrente (MSB-first): o primeiro bit do byte vai
     * para a posicao 7, o proximo para a 6, e assim por diante. */
    if (bit)
        w->current = (uint8_t)(w->current | (1u << (7 - w->bit_count)));
    w->bit_count++;

    if (w->bit_count == 8)
        return bit_writer_append_current(w);
    return true;
}

bool bit_writer_write_bits(BitWriter *w, uint64_t bits, unsigned count)
{
    if (w == NULL || w->error)
        return false;
    if (count == 0)
        return true;
    if (count > 64)
        return false;

    /* Emite do bit mais significativo (posicao count-1) ao menos significativo
     * (posicao 0), para casar com a tabela de codigos do Modulo 4. */
    for (unsigned i = count; i-- > 0; ) {
        int bit = (int)((bits >> i) & 1u);
        if (!bit_writer_write_bit(w, bit))
            return false;
    }
    return true;
}

bool bit_writer_flush(BitWriter *w)
{
    if (w == NULL || w->error)
        return false;

    /* Sem byte parcial pendente: nada a fazer. Os bits ja colocados em 'current'
     * ocupam as posicoes altas; as posicoes baixas restantes ja sao zero
     * (padding), pois 'current' foi reiniciado para 0 no ultimo append. */
    if (w->bit_count == 0)
        return true;

    return bit_writer_append_current(w);
}

void bit_writer_free(BitWriter *w)
{
    if (w == NULL)
        return;
    free(w->buffer);
    w->buffer     = NULL;
    w->capacity   = 0;
    w->byte_count = 0;
    w->current    = 0;
    w->bit_count  = 0;
    w->error      = false;
}

/* ------------------------------------------------------------------ */
/* BitReader                                                           */
/* ------------------------------------------------------------------ */

void bit_reader_init(BitReader *r, const void *buf, size_t size)
{
    if (r == NULL)
        return;
    r->buffer   = (const uint8_t *)buf;
    r->size     = (buf == NULL) ? 0 : size;
    r->byte_pos = 0;
    r->bit_pos  = 0;
}

int bit_reader_read_bit(BitReader *r)
{
    if (r == NULL || r->byte_pos >= r->size)
        return -1;

    /* Le o bit na posicao corrente (MSB-first): bit_pos 0 -> posicao 7. */
    int bit = (r->buffer[r->byte_pos] >> (7 - r->bit_pos)) & 1;

    r->bit_pos++;
    if (r->bit_pos == 8) {
        r->bit_pos = 0;
        r->byte_pos++;
    }
    return bit;
}

bool bit_reader_read_bits(BitReader *r, unsigned count, uint64_t *out)
{
    if (r == NULL || out == NULL || count > 64)
        return false;

    uint64_t valor = 0;
    for (unsigned i = 0; i < count; i++) {
        int bit = bit_reader_read_bit(r);
        if (bit < 0)
            return false; /* faltou bit: fim de fluxo no meio da leitura */
        valor = (valor << 1) | (uint64_t)bit;
    }
    *out = valor;
    return true;
}
