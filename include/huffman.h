/*
 * huffman.h - Construcao da arvore de Huffman (Modulo 3).
 *
 * A arvore de Huffman e o nucleo de Estrutura de Dados do projeto: a partir das
 * frequencias dos bytes de um bloco, montamos uma arvore binaria em que os
 * simbolos MAIS frequentes ficam mais perto da raiz (codigos curtos) e os menos
 * frequentes mais fundo (codigos longos). Isso e o que permite comprimir.
 *
 * Como a arvore e construida (algoritmo classico de Huffman):
 *   1) cria-se uma FOLHA por byte distinto (com freq > 0);
 *   2) usa-se o min-heap do Modulo 1 (fila de prioridade pela frequencia);
 *   3) enquanto houver mais de um no no heap, retiram-se os DOIS de menor
 *      frequencia e combinam-se num NO INTERNO cuja frequencia e a soma deles;
 *   4) o ultimo no que sobra e a RAIZ da arvore.
 *
 * Caso especial obrigatorio (modularizacao.md, Modulo 3): bloco com apenas um
 * byte distinto. Nesse caso o passo 3 nunca executa e a arvore e uma UNICA
 * FOLHA. A atribuicao do codigo "0" (um unico bit) para esse byte e
 * responsabilidade do Modulo 4 (tabela de codigos); aqui apenas garantimos que
 * a arvore correta (folha unica) e devolvida.
 *
 * A arvore tambem sera serializada no cabecalho de cada bloco (Modulo 7), para
 * que o `cunzip` reconstrua e decodifique sem depender do processo que compactou.
 */
#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * HuffNode - no da arvore de Huffman.
 *
 * A struct e publica (nao opaca) porque os modulos seguintes precisam percorrer
 * a arvore: a tabela de codigos (Modulo 4), a serializacao (Modulo 7) e a
 * descompressao bit a bit (Modulo 8).
 *
 * is_leaf : true em folhas. Numa folha, 'symbol' guarda o byte representado;
 *           num no interno, 'symbol' nao tem significado.
 * frequency: numa folha e a frequencia do byte; num no interno e a SOMA das
 *            frequencias dos dois filhos (usada como chave no heap).
 * left/right: filhos (NULL nas folhas). Todo no interno tem SEMPRE dois filhos
 *            (arvore binaria completa nesse sentido), o que garante a
 *            propriedade de prefixo dos codigos.
 */
typedef struct HuffNode {
    struct HuffNode *left;
    struct HuffNode *right;
    uint64_t         frequency;
    uint8_t          symbol;
    bool             is_leaf;
} HuffNode;

/*
 * Conta a frequencia de cada valor de byte (0..255) presente em 'buf'.
 *
 * O vetor 'freq' (256 posicoes) e REINICIADO para zero e depois preenchido:
 * freq[b] passa a conter quantas vezes o byte b aparece nos 'len' bytes de 'buf'.
 * Seguro se 'buf' for NULL com 'len' == 0 (resultado: tudo zero). 'freq' nao
 * pode ser NULL.
 */
void huffman_count_frequencies(const void *buf, size_t len, uint64_t freq[256]);

/*
 * Monta a arvore de Huffman a partir do vetor de frequencias (256 posicoes).
 *
 * Considera apenas os bytes com freq > 0. Retorna a RAIZ da arvore, que o
 * chamador deve liberar depois com huffman_free_tree().
 *
 * Casos de borda:
 *   - nenhum byte com freq > 0 (bloco vazio): retorna NULL;
 *   - exatamente um byte distinto: retorna uma UNICA FOLHA (sem filhos);
 *   - falta de memoria: libera tudo o que ja havia alocado e retorna NULL.
 */
HuffNode *huffman_build_tree(const uint64_t freq[256]);

/* Libera recursivamente a arvore. Seguro chamar com NULL. */
void huffman_free_tree(HuffNode *root);

#endif /* HUFFMAN_H */
