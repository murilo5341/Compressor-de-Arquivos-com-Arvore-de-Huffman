/*
 * pipeline.h - Pipeline concorrente de compressao (Modulo 13).
 *
 * Implementa a parte de Sistemas Operacionais (E3): comprimir o arquivo em
 * BLOCOS independentes usando varias threads, ligadas por filas bloqueantes
 * (Modulo 12):
 *
 *   thread leitora --> [fila de blocos crus] --> N threads codificadoras
 *   N codificadoras --> [fila de blocos comprimidos] --> thread escritora
 *
 * - A LEITORA le o arquivo em pedacos de block_size bytes, numera cada bloco
 *   (indice sequencial) e o enfileira cru.
 * - Cada CODIFICADORA tira um bloco cru, calcula o CRC32 do conteudo original
 *   (Modulo 2), comprime com Huffman (Modulo 6) e serializa a arvore (Modulo 7),
 *   e enfileira o bloco comprimido.
 * - A ESCRITORA recebe os blocos comprimidos (que chegam FORA DE ORDEM, pois as
 *   codificadoras terminam em tempos diferentes) e os grava no .cz NA ORDEM
 *   CORRETA, usando um escritor REORDENADOR: mantem 'next_to_write' (proximo
 *   indice esperado) e guarda os blocos adiantados em 'pending' ate poderem ser
 *   gravados em sequencia. Assim que um bloco e gravado, sua memoria e liberada
 *   (memoria limitada, nao acumula o arquivo inteiro).
 *
 * O formato do .cz e identico ao do czip sequencial (Modulo 10): o arquivo
 * gerado com N threads e BYTE A BYTE igual ao gerado com 1 thread - o paralelismo
 * nao muda a saida, so a velocidade. Isso e o que permite o speedup do teste de
 * fogo (Modulo 17) sem quebrar o roundtrip.
 *
 * PORTABILIDADE: usa pthreads. O MinGW local (GCC 6.3.0, modelo win32) NAO tem
 * libpthread, entao este modulo so e compilado/validado em Linux. O czip no
 * Windows usa o caminho SEQUENCIAL (Modulo 10) via #ifndef HAVE_PTHREAD; no
 * Linux usa este pipeline. A ausencia de data races e validada com make tsan
 * (RULES REGRA 4, -15%).
 */
#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdint.h>

/*
 * Comprime 'input_path' para 'output_path' usando o pipeline concorrente, com
 * blocos de 'block_size' bytes e 'num_threads' threads codificadoras
 * (num_threads < 1 e tratado como 1). Escreve o cabecalho global, os blocos em
 * ordem e reescreve o block_count no fim (mesmo formato do Modulo 9/10).
 *
 * Retorna 0 em sucesso; 1 em erro (abrir/criar arquivo, leitura, compressao,
 * escrita ou falta de memoria) - a causa e impressa no stderr.
 */
int pipeline_compress_file(const char *input_path, const char *output_path,
                           uint32_t block_size, int num_threads);

#endif /* PIPELINE_H */
