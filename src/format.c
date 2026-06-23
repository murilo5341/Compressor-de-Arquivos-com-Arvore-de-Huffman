/*
 * format.c - Implementacao do formato .cz (Modulo 9).
 *
 * (De)serializacao dos cabecalhos global e de bloco em LITTLE-ENDIAN portavel:
 * cada inteiro e desmontado byte a byte na escrita e remontado byte a byte na
 * leitura, sem depender do endianess nativo da maquina (ver format.h).
 */
#include "format.h"

#include <string.h>

/* ------------------------------------------------------------------ */
/* Helpers de endianess (little-endian, byte a byte)                   */
/* ------------------------------------------------------------------ */

/* Grava 'v' como 4 bytes little-endian. Retorna true se escreveu os 4 bytes. */
static bool write_u32_le(FILE *f, uint32_t v) {
    uint8_t b[4];
    b[0] = (uint8_t)(v);
    b[1] = (uint8_t)(v >> 8);
    b[2] = (uint8_t)(v >> 16);
    b[3] = (uint8_t)(v >> 24);
    return fwrite(b, 1, sizeof b, f) == sizeof b;
}

/* Grava 'v' como 8 bytes little-endian. Retorna true se escreveu os 8 bytes. */
static bool write_u64_le(FILE *f, uint64_t v) {
    uint8_t b[8];
    for (int i = 0; i < 8; i++) {
        b[i] = (uint8_t)(v >> (8 * i));
    }
    return fwrite(b, 1, sizeof b, f) == sizeof b;
}

/* Le 4 bytes little-endian em '*out'. Retorna false se faltarem bytes. */
static bool read_u32_le(FILE *f, uint32_t *out) {
    uint8_t b[4];
    if (fread(b, 1, sizeof b, f) != sizeof b) {
        return false;
    }
    *out = (uint32_t)b[0]
         | ((uint32_t)b[1] << 8)
         | ((uint32_t)b[2] << 16)
         | ((uint32_t)b[3] << 24);
    return true;
}

/* Le 8 bytes little-endian em '*out'. Retorna false se faltarem bytes. */
static bool read_u64_le(FILE *f, uint64_t *out) {
    uint8_t b[8];
    if (fread(b, 1, sizeof b, f) != sizeof b) {
        return false;
    }
    uint64_t v = 0;
    for (int i = 0; i < 8; i++) {
        v |= (uint64_t)b[i] << (8 * i);
    }
    *out = v;
    return true;
}

/* ------------------------------------------------------------------ */
/* Cabecalho global                                                    */
/* ------------------------------------------------------------------ */

void cz_global_header_init(CzGlobalHeader *h, uint32_t block_size,
                           uint64_t block_count) {
    if (h == NULL) {
        return;
    }
    memcpy(h->magic, CZ_MAGIC, CZ_MAGIC_SIZE);
    h->version     = CZ_FORMAT_VERSION;
    h->block_size  = block_size;
    h->block_count = block_count;
}

bool cz_write_global_header(FILE *f, const CzGlobalHeader *h) {
    if (f == NULL || h == NULL) {
        return false;
    }
    /* Magic gravado sempre como "CZHF", ignorando lixo eventual em h->magic. */
    if (fwrite(CZ_MAGIC, 1, CZ_MAGIC_SIZE, f) != CZ_MAGIC_SIZE) {
        return false;
    }
    if (!write_u32_le(f, h->version))     return false;
    if (!write_u32_le(f, h->block_size))  return false;
    if (!write_u64_le(f, h->block_count)) return false;
    return true;
}

bool cz_read_global_header(FILE *f, CzGlobalHeader *h) {
    if (f == NULL || h == NULL) {
        return false;
    }
    if (fread(h->magic, 1, CZ_MAGIC_SIZE, f) != CZ_MAGIC_SIZE) {
        return false;
    }
    if (memcmp(h->magic, CZ_MAGIC, CZ_MAGIC_SIZE) != 0) {
        return false;  /* nao e um arquivo .cz valido */
    }
    if (!read_u32_le(f, &h->version))     return false;
    if (h->version != CZ_FORMAT_VERSION) {
        return false;  /* versao incompativel */
    }
    if (!read_u32_le(f, &h->block_size))  return false;
    if (!read_u64_le(f, &h->block_count)) return false;
    return true;
}

/* ------------------------------------------------------------------ */
/* Cabecalho de bloco                                                  */
/* ------------------------------------------------------------------ */

bool cz_write_block_header(FILE *f, const CzBlockHeader *h) {
    if (f == NULL || h == NULL) {
        return false;
    }
    if (!write_u64_le(f, h->block_index))     return false;
    if (!write_u32_le(f, h->original_size))   return false;
    if (!write_u32_le(f, h->compressed_size)) return false;
    if (!write_u32_le(f, h->tree_size))       return false;
    if (!write_u32_le(f, h->original_crc32))  return false;
    return true;
}

bool cz_read_block_header(FILE *f, CzBlockHeader *h) {
    if (f == NULL || h == NULL) {
        return false;
    }
    if (!read_u64_le(f, &h->block_index))     return false;
    if (!read_u32_le(f, &h->original_size))   return false;
    if (!read_u32_le(f, &h->compressed_size)) return false;
    if (!read_u32_le(f, &h->tree_size))       return false;
    if (!read_u32_le(f, &h->original_crc32))  return false;
    return true;
}

bool cz_skip_block_payload(FILE *f, const CzBlockHeader *h) {
    if (f == NULL || h == NULL) {
        return false;
    }
    /* tree_size e compressed_size sao uint32; a soma cabe num long em qualquer
     * plataforma com long >= 32 bits porque os blocos sao limitados pelo
     * block_size configurado (tipicamente dezenas de KB). */
    long skip = (long)h->tree_size + (long)h->compressed_size;
    return fseek(f, skip, SEEK_CUR) == 0;
}
