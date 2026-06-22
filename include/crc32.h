/*
 * crc32.h - Checksum CRC32 por bloco (Modulo 2).
 *
 * Serve para detectar corrupcao dos dados: cada bloco do arquivo tem o CRC32
 * do seu conteudo ORIGINAL gravado no cabecalho (na compressao). Na
 * descompressao, recalcula-se o CRC32 do bloco restaurado e compara-se com o
 * valor gravado; se diferirem, o bloco esta corrompido.
 *
 * Politica adotada (modularizacao.md, Modulo 2 / RULES REGRA 5):
 *   - calcula-se o CRC32 sobre o CONTEUDO ORIGINAL (descomprimido) do bloco;
 *   - a compressao grava esse valor no cabecalho do bloco;
 *   - a descompressao recalcula sobre o bloco restaurado e compara.
 *
 * Variante: CRC-32/ISO-HDLC (a mesma de zlib/gzip/PNG):
 *   - polinomio refletido 0xEDB88320;
 *   - valor inicial 0xFFFFFFFF e XOR final 0xFFFFFFFF.
 * Escolhida por ser o padrao de fato, com vetores de teste publicos
 * verificaveis (ex.: CRC32("123456789") == 0xCBF43926).
 */
#ifndef CRC32_H
#define CRC32_H

#include <stddef.h>
#include <stdint.h>

/*
 * Calcula o CRC32 de um buffer inteiro de uma so vez (caso mais comum).
 * 'buf' pode ser NULL se 'len' for 0. Retorna o CRC32 ja finalizado.
 */
uint32_t crc32_buffer(const void *buf, size_t len);

/*
 * API incremental, para acumular o CRC32 de um bloco em varias chamadas:
 *
 *   uint32_t c = crc32_init();
 *   c = crc32_update(c, parte1, n1);
 *   c = crc32_update(c, parte2, n2);
 *   uint32_t final = crc32_finalize(c);
 *
 * O valor que circula entre init/update/finalize e o estado interno (ainda nao
 * finalizado); use crc32_finalize para obter o CRC32 definitivo.
 */
uint32_t crc32_init(void);
uint32_t crc32_update(uint32_t crc, const void *buf, size_t len);
uint32_t crc32_finalize(uint32_t crc);

#endif /* CRC32_H */
