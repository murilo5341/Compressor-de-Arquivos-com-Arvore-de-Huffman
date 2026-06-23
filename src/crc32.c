/*
 * crc32.c - Implementacao do CRC32 por bloco (Modulo 2).
 *
 * Usa o algoritmo "table-driven": uma tabela de 256 entradas (uma por valor de
 * byte) permite processar um byte por iteracao sem recalcular o polinomio bit a
 * bit. A tabela e construida uma unica vez, na primeira chamada, a partir do
 * polinomio refletido 0xEDB88320 (CRC-32/ISO-HDLC, igual ao zlib/gzip).
 *
 * Convencao da variante:
 *   - estado inicial 0xFFFFFFFF (crc32_init);
 *   - processa cada byte refletido (LSB primeiro), por isso a tabela usa o
 *     polinomio ja refletido;
 *   - XOR final 0xFFFFFFFF (crc32_finalize).
 */
#include "crc32.h"

#include <stdbool.h>

#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif

#define CRC32_POLY      0xEDB88320u  /* polinomio refletido (LSB-first) */
#define CRC32_INITIAL   0xFFFFFFFFu  /* estado inicial / XOR final */

/* Tabela de 256 entradas, construida uma unica vez na primeira utilizacao. */
static uint32_t crc32_table[256];

/* Preenche crc32_table a partir do polinomio refletido. */
static void crc32_montar_tabela(void)
{
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 1u)
                crc = (crc >> 1) ^ CRC32_POLY;
            else
                crc >>= 1;
        }
        crc32_table[i] = crc;
    }
}

/*
 * Garante que crc32_table esteja pronta antes do primeiro uso.
 *
 * No pipeline concorrente (Modulo 13+) varias threads chamam crc32_update ao
 * mesmo tempo; a inicializacao preguicosa simples (escrever a tabela enquanto
 * outras threads a leem) e uma DATA RACE. Sob pthreads usamos pthread_once, que
 * executa a montagem exatamente uma vez e estabelece happens-before entre a
 * escrita da tabela e as leituras subsequentes de todas as threads. Sem
 * pthreads (build sequencial) cai numa flag booleana comum, sem custo de lock.
 */
#ifdef HAVE_PTHREAD
static pthread_once_t crc32_once = PTHREAD_ONCE_INIT;

static void crc32_garantir_tabela(void)
{
    pthread_once(&crc32_once, crc32_montar_tabela);
}
#else
static bool crc32_table_pronta = false;

static void crc32_garantir_tabela(void)
{
    if (!crc32_table_pronta) {
        crc32_montar_tabela();
        crc32_table_pronta = true;
    }
}
#endif

uint32_t crc32_init(void)
{
    return CRC32_INITIAL;
}

uint32_t crc32_update(uint32_t crc, const void *buf, size_t len)
{
    if (buf == NULL || len == 0)
        return crc;

    crc32_garantir_tabela();

    const unsigned char *bytes = (const unsigned char *)buf;
    for (size_t i = 0; i < len; i++) {
        uint8_t indice = (uint8_t)((crc ^ bytes[i]) & 0xFFu);
        crc = (crc >> 8) ^ crc32_table[indice];
    }
    return crc;
}

uint32_t crc32_finalize(uint32_t crc)
{
    return crc ^ CRC32_INITIAL;
}

uint32_t crc32_buffer(const void *buf, size_t len)
{
    return crc32_finalize(crc32_update(crc32_init(), buf, len));
}
