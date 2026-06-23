/*
 * tree_serialization.h - Serializacao da arvore de Huffman (Modulo 7).
 *
 * Cada bloco do arquivo .cz tem a SUA arvore de Huffman. Para que o `cunzip`
 * reconstrua e decodifique sem depender do processo que compactou (RULES REGRA
 * 9), a arvore precisa ser gravada no cabecalho de cada bloco. Este modulo faz
 * essa (de)serializacao.
 *
 * Formato (pre-ordem, bit a bit, MSB-first - reusa o bit I/O do Modulo 5):
 *
 *   - FOLHA      : escreve o bit 1, seguido dos 8 bits do byte (symbol);
 *   - NO INTERNO : escreve o bit 0, seguido da serializacao do filho ESQUERDO
 *                  e depois do filho DIREITO.
 *
 * Exemplo conceitual:
 *   folha('A')    -> 1 + 01000001
 *   interno(L, R) -> 0 + serializacao(L) + serializacao(R)
 *
 * A travessia em pre-ordem e auto-delimitada: a desserializacao consome
 * exatamente os bits de cada no, entao nao e preciso um marcador de fim. Os
 * bits de padding do ultimo byte (flush do Modulo 5) sao simplesmente ignorados,
 * pois a recursao para sozinha ao completar a arvore.
 *
 * O cabecalho do bloco (Modulo 9) grava o tamanho da arvore serializada em BYTES
 * (out_size), para saber onde a arvore termina e o payload comprimido comeca.
 */
#ifndef TREE_SERIALIZATION_H
#define TREE_SERIALIZATION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "huffman.h"

/*
 * Serializa a arvore 'root' em pre-ordem para um buffer de bytes recem-alocado.
 *
 * Em caso de sucesso, retorna true; '*out_bytes' recebe o buffer (que o chamador
 * deve liberar com free()) e '*out_size' o numero de bytes uteis (ja com o
 * ultimo byte fechado por padding de zeros).
 *
 * Casos de borda / erro (retorna false e deixa '*out_bytes' = NULL,
 * '*out_size' = 0):
 *   - 'root', 'out_bytes' ou 'out_size' NULL (uma arvore vazia nao existe: bloco
 *     vazio nao chega aqui, ele e tratado pelo formato no Modulo 9);
 *   - falta de memoria durante a escrita.
 *
 * Observacao: uma FOLHA UNICA (bloco com um unico byte distinto) e serializada
 * como "1 + 8 bits do byte" - a raiz e a propria folha.
 */
bool tree_serialize(const HuffNode *root, uint8_t **out_bytes, size_t *out_size);

/*
 * Reconstroi a arvore a partir dos 'size' bytes de 'bytes' (formato pre-ordem
 * descrito acima). Retorna a RAIZ, que o chamador deve liberar com
 * huffman_free_tree(), ou NULL em erro:
 *   - 'bytes' NULL ou 'size' == 0;
 *   - bits insuficientes (serializacao truncada/corrompida);
 *   - falta de memoria.
 *
 * A arvore reconstruida preserva a estrutura e os simbolos das folhas; o campo
 * 'frequency' nao e restaurado (fica 0), pois a decodificacao nao precisa dele.
 */
HuffNode *tree_deserialize(const uint8_t *bytes, size_t size);

#endif /* TREE_SERIALIZATION_H */
