/*
 * block.h - Compressao de um bloco em memoria (Modulo 6).
 *
 * Este modulo amarra as pecas dos modulos anteriores para comprimir UM bloco de
 * bytes (um pedaco do arquivo) inteiramente em memoria:
 *
 *   1) conta as frequencias dos bytes do bloco          (Modulo 3);
 *   2) monta a arvore de Huffman a partir delas          (Modulo 3);
 *   3) gera a tabela de codigos por travessia da arvore  (Modulo 4);
 *   4) emite, bit a bit, o codigo de cada byte do bloco  (Modulos 4 e 5);
 *   5) faz o flush do ultimo byte parcial (padding de zeros).
 *
 * O resultado e um BlockCompressed contendo o payload comprimido (bytes), a
 * arvore de Huffman do bloco, o tamanho original e o tamanho comprimido. A
 * arvore acompanha o resultado porque os proximos modulos precisam dela: a
 * serializacao no cabecalho do bloco (Modulo 7) e a descompressao (Modulo 8),
 * para que o `cunzip` reconstrua e decodifique sem depender deste processo.
 *
 * Importante: o tamanho comprimido NAO indica onde os bits uteis terminam dentro
 * do ultimo byte (ha padding). A parada da decodificacao usa 'original_size'
 * (Modulos 8/9), conforme o contrato do bit I/O (Modulo 5).
 */
#ifndef BLOCK_H
#define BLOCK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "huffman.h"

/*
 * BlockCompressed - resultado da compressao de um bloco.
 *
 * tree           : raiz da arvore de Huffman do bloco. NULL apenas para um bloco
 *                  vazio (original_size == 0). O chamador NAO deve liberar a
 *                  arvore separadamente: block_compressed_free() cuida disso.
 * data           : payload comprimido (bytes empacotados MSB-first). NULL quando
 *                  compressed_size == 0.
 * compressed_size: numero de bytes validos em 'data' (inclui o padding do ultimo
 *                  byte parcial).
 * original_size  : numero de bytes do bloco original, criterio de parada da
 *                  decodificacao (Modulos 8/9).
 */
typedef struct {
    HuffNode *tree;
    uint8_t  *data;
    size_t    compressed_size;
    size_t    original_size;
} BlockCompressed;

/*
 * Comprime os 'len' bytes de 'input' e preenche '*out'.
 *
 * Em caso de sucesso, retorna true e o chamador deve liberar '*out' depois com
 * block_compressed_free(). Casos de borda:
 *   - bloco vazio (len == 0, 'input' pode ser NULL): sucesso, com tree == NULL,
 *     data == NULL, compressed_size == 0 e original_size == 0;
 *   - bloco de um unico byte distinto: a arvore e uma folha unica e cada byte
 *     vira 1 bit (codigo "0"), conforme Modulo 4;
 *   - falta de memoria: libera tudo o que ja havia alocado, deixa '*out' zerado
 *     e retorna false.
 *
 * Retorna false tambem se 'out' for NULL.
 */
bool block_compress(const void *input, size_t len, BlockCompressed *out);

/*
 * Libera a arvore e o payload de um BlockCompressed e zera a struct. Seguro
 * chamar com NULL ou com uma struct ja zerada.
 */
void block_compressed_free(BlockCompressed *bc);

#endif /* BLOCK_H */
