/*
 * bitio.h - Escrita e leitura bit a bit (Modulo 5).
 *
 * Os codigos de Huffman (Modulo 4) tem tamanho em BITS, nao em bytes: o codigo
 * de um byte pode ter 1, 3, 7, 11... bits. Para comprimir de verdade precisamos
 * gravar esses bits empacotados, e nao um caractere '0'/'1' por bit (o que nao
 * comprimiria nada). Este modulo faz essa ponte:
 *
 *   - BitWriter: acumula bits num buffer de bytes que cresce sozinho. Usado pela
 *     compressao de bloco (Modulo 6) e pela serializacao da arvore (Modulo 7).
 *   - BitReader: le bits de um buffer ja existente. Usado pela descompressao
 *     (Modulo 8) e pela desserializacao da arvore (Modulo 7).
 *
 * Ordem dos bits: MSB-first. O primeiro bit escrito ocupa o bit 7 (mais
 * significativo) do primeiro byte; o oitavo bit fecha o byte. A leitura segue a
 * mesma ordem. Isso casa com a tabela de codigos do Modulo 4, que guarda os bits
 * encostados a direita MSB-first: para escrever um codigo basta emitir os bits
 * da posicao length-1 ate 0 com bit_writer_write_bits().
 *
 * Byte parcial e padding (contrato exigido pela modularizacao, Modulo 5):
 *   - o ultimo byte do payload quase sempre sobra incompleto; bit_writer_flush()
 *     completa esse byte com ZEROS a direita;
 *   - esses bits de padding NAO carregam informacao. Na descompressao, a parada
 *     correta vem do criterio original_size do bloco (Modulos 8/9): decodifica-se
 *     ate reconstruir a quantidade certa de bytes e os bits de padding finais sao
 *     simplesmente ignorados.
 */
#ifndef BITIO_H
#define BITIO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/* Escritor de bits (BitWriter)                                        */
/* ------------------------------------------------------------------ */

/*
 * BitWriter - acumula bits num buffer dinamico de bytes (MSB-first).
 *
 * A struct e publica (nao opaca) porque o chamador, depois de bit_writer_flush(),
 * precisa acessar o resultado: 'buffer' (os bytes) e 'byte_count' (quantos bytes
 * validos ha). Os demais campos sao estado interno e nao devem ser alterados a mao.
 *
 * buffer    : bytes ja escritos (cresce com realloc dobrando a capacidade).
 * capacity  : capacidade alocada de 'buffer', em bytes.
 * byte_count: bytes COMPLETOS ja gravados em 'buffer'.
 * current   : byte parcial em construcao (bits ja colocados ocupam os mais altos).
 * bit_count : quantos bits ja ocupados em 'current' (0..7).
 * error     : true se alguma alocacao falhou; as operacoes viram no-op.
 */
typedef struct {
    uint8_t *buffer;
    size_t   capacity;
    size_t   byte_count;
    uint8_t  current;
    int      bit_count;
    bool     error;
} BitWriter;

/*
 * Inicializa o escritor com um buffer vazio (capacidade inicial padrao).
 * Retorna false se 'w' for NULL ou faltar memoria. Em caso de sucesso, o
 * chamador deve liberar com bit_writer_free() ao terminar.
 */
bool bit_writer_init(BitWriter *w);

/*
 * Escreve um unico bit (qualquer valor != 0 conta como 1) na posicao corrente,
 * MSB-first. Retorna false se 'w' for NULL ou se houver erro de alocacao
 * (que tambem marca w->error). Quando o byte corrente completa 8 bits, ele e
 * anexado ao buffer automaticamente.
 */
bool bit_writer_write_bit(BitWriter *w, int bit);

/*
 * Escreve 'count' bits (0..64) tirados de 'bits', do mais significativo
 * (posicao count-1) ao menos significativo (posicao 0) - MSB-first. E a forma
 * de emitir um HuffCode: bit_writer_write_bits(w, code.bits, code.length).
 * count == 0 e no-op; count > 64 falha. Retorna false em erro.
 */
bool bit_writer_write_bits(BitWriter *w, uint64_t bits, unsigned count);

/*
 * Fecha o byte parcial atual, completando-o com ZEROS a direita (padding), e o
 * anexa ao buffer. Se nao houver byte parcial pendente, e no-op. Apos o flush, o
 * resultado completo esta em w->buffer (w->byte_count bytes). Retorna false em erro.
 */
bool bit_writer_flush(BitWriter *w);

/* Libera o buffer interno e zera o estado. Seguro chamar com NULL. */
void bit_writer_free(BitWriter *w);

/* ------------------------------------------------------------------ */
/* Leitor de bits (BitReader)                                          */
/* ------------------------------------------------------------------ */

/*
 * BitReader - le bits de um buffer de bytes existente (MSB-first).
 *
 * Nao possui buffer proprio: aponta para a memoria fornecida em bit_reader_init
 * (que deve permanecer valida durante a leitura). Por isso nao ha funcao de
 * liberacao.
 *
 * buffer  : bytes de origem (somente leitura).
 * size    : total de bytes em 'buffer'.
 * byte_pos: indice do byte atual.
 * bit_pos : proximo bit a ler dentro do byte atual (0 = bit 7/MSB .. 7 = bit 0/LSB).
 */
typedef struct {
    const uint8_t *buffer;
    size_t         size;
    size_t         byte_pos;
    int            bit_pos;
} BitReader;

/*
 * Inicializa o leitor sobre 'buf' (com 'size' bytes). 'buf' pode ser NULL se
 * 'size' for 0 (leitura sempre devolvera fim de fluxo).
 */
void bit_reader_init(BitReader *r, const void *buf, size_t size);

/*
 * Le o proximo bit (MSB-first). Retorna 0 ou 1, ou -1 se nao houver mais bits
 * (fim do buffer) ou se 'r' for NULL.
 */
int bit_reader_read_bit(BitReader *r);

/*
 * Le 'count' bits (0..64) na mesma ordem da escrita (MSB-first) e os devolve em
 * *out encostados a direita: o primeiro bit lido vira o mais significativo dos
 * 'count'. Retorna false se faltar bits, se count > 64 ou se 'r'/'out' forem
 * invalidos; nesse caso *out fica indefinido. count == 0 devolve 0 com sucesso.
 */
bool bit_reader_read_bits(BitReader *r, unsigned count, uint64_t *out);

#endif /* BITIO_H */
