/*
 * format.h - Formato do arquivo .cz (Modulo 9).
 *
 * Define e (de)serializa os CABECALHOS do arquivo compactado. O .cz precisa
 * carregar metadados suficientes para que o `cunzip` reconstrua o arquivo sem
 * depender do processo que compactou (RULES REGRA 9): por isso cada bloco grava
 * no proprio cabecalho a sua arvore serializada e os tamanhos necessarios.
 *
 * Este modulo NAO comprime nem descomprime: ele apenas le/escreve os cabecalhos.
 * O `czip` (Modulo 10) e o `cunzip` (Modulo 11) usam estas funcoes em volta da
 * compressao/descompressao de bloco (Modulos 6/8) e da (de)serializacao da
 * arvore (Modulo 7).
 *
 * Decisao de projeto: a extensao .cz e o magic number "CZHF" sao escolhas da
 * equipe, NAO exigencias do edital (RULES REGRA 5). Podem ser alteradas desde
 * que documentadas e justificaveis na defesa oral.
 *
 * ENDIANESS (documentada - RULES REGRA 5 / modularizacao.md, Modulo 9):
 *   todos os inteiros sao gravados em LITTLE-ENDIAN, byte a byte, INDEPENDENTE
 *   da maquina. Nao gravamos a representacao nativa em memoria; cada uint32/
 *   uint64 e desmontado/remontado byte a byte (ver write_u32_le/read_u32_le em
 *   format.c). Assim o arquivo e portavel entre arquiteturas.
 *
 * LAYOUT DO ARQUIVO:
 *
 *   Cabecalho global (CZ_GLOBAL_HEADER_SIZE = 20 bytes):
 *     magic[4]      = 'C','Z','H','F'
 *     version       : uint32
 *     block_size    : uint32   (tamanho de bloco usado na compressao)
 *     block_count   : uint64   (quantidade total de blocos)
 *
 *   Para CADA bloco:
 *     Cabecalho do bloco (CZ_BLOCK_HEADER_SIZE = 24 bytes):
 *       block_index   : uint64
 *       original_size : uint32  (bytes do bloco antes de comprimir; criterio de
 *                                parada da decodificacao - Modulos 8/5)
 *       compressed_size: uint32 (bytes do payload comprimido)
 *       tree_size     : uint32  (bytes da arvore serializada - Modulo 7)
 *       original_crc32: uint32  (CRC32 do CONTEUDO ORIGINAL - Modulo 2)
 *     tree_bytes[tree_size]        (arvore serializada do bloco)
 *     compressed_bytes[compressed_size]  (payload comprimido em bits)
 *
 * ROBUSTEZ (premissa de design, nao apenas teste): tendo tree_size e
 * compressed_size NO CABECALHO do bloco, o `cunzip` consegue localizar o inicio
 * do PROXIMO bloco mesmo quando o bloco atual estiver corrompido, saltando
 * tree_size + compressed_size bytes (cz_skip_block_payload). A parada da
 * decodificacao do payload usa original_size, tratando o byte parcial final.
 */
#ifndef FORMAT_H
#define FORMAT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* Magic number e versao do formato .cz (decisao da equipe - RULES REGRA 5). */
#define CZ_MAGIC          "CZHF"
#define CZ_MAGIC_SIZE     4
#define CZ_FORMAT_VERSION 1u

/* Tamanhos fixos dos cabecalhos serializados, em bytes (uteis para o salto e
 * para validacao de tamanho minimo do arquivo). */
#define CZ_GLOBAL_HEADER_SIZE 20  /* 4 + 4 + 4 + 8 */
#define CZ_BLOCK_HEADER_SIZE  24  /* 8 + 4 + 4 + 4 + 4 */

/*
 * Cabecalho global do arquivo .cz. 'magic' guarda os 4 bytes lidos do arquivo
 * (NAO terminados em '\0'); cz_read_global_header valida que sao "CZHF".
 */
typedef struct {
    char     magic[CZ_MAGIC_SIZE];
    uint32_t version;
    uint32_t block_size;
    uint64_t block_count;
} CzGlobalHeader;

/*
 * Cabecalho de um bloco. Acompanha (no arquivo) a arvore serializada e o
 * payload comprimido, que NAO fazem parte desta struct - sao gravados/lidos
 * separadamente pelo czip/cunzip usando tree_size e compressed_size.
 */
typedef struct {
    uint64_t block_index;
    uint32_t original_size;
    uint32_t compressed_size;
    uint32_t tree_size;
    uint32_t original_crc32;
} CzBlockHeader;

/* ------------------------------------------------------------------ */
/* Cabecalho global                                                    */
/* ------------------------------------------------------------------ */

/*
 * Preenche 'h' com magic "CZHF", versao corrente e os parametros informados.
 * Conveniencia para o czip nao montar a struct a mao. No-op se 'h' for NULL.
 */
void cz_global_header_init(CzGlobalHeader *h, uint32_t block_size,
                           uint64_t block_count);

/*
 * Grava o cabecalho global em 'f' (little-endian). Retorna false se 'f'/'h'
 * forem NULL ou se a escrita falhar.
 */
bool cz_write_global_header(FILE *f, const CzGlobalHeader *h);

/*
 * Le o cabecalho global de 'f' e o VALIDA. Retorna false se:
 *   - 'f'/'h' forem NULL;
 *   - faltarem bytes (arquivo truncado);
 *   - o magic nao for "CZHF";
 *   - a versao nao for CZ_FORMAT_VERSION.
 * Em sucesso, 'h' fica preenchido (h->magic guarda os 4 bytes lidos).
 */
bool cz_read_global_header(FILE *f, CzGlobalHeader *h);

/* ------------------------------------------------------------------ */
/* Cabecalho de bloco                                                  */
/* ------------------------------------------------------------------ */

/*
 * Grava o cabecalho de um bloco em 'f' (little-endian). Retorna false se
 * 'f'/'h' forem NULL ou se a escrita falhar. A arvore serializada e o payload
 * devem ser gravados pelo chamador LOGO APOS, em ordem (tree_bytes, depois
 * compressed_bytes).
 */
bool cz_write_block_header(FILE *f, const CzBlockHeader *h);

/*
 * Le o cabecalho de um bloco de 'f'. Retorna false se 'f'/'h' forem NULL ou se
 * faltarem bytes (fim de arquivo / truncado) - o chamador usa isso para detectar
 * o fim da sequencia de blocos. NAO valida o conteudo (tamanhos), apenas le os
 * campos; a validacao por CRC32 ocorre depois de descomprimir (Modulo 11).
 */
bool cz_read_block_header(FILE *f, CzBlockHeader *h);

/*
 * Salta a arvore serializada e o payload comprimido do bloco atual, deixando 'f'
 * posicionado no inicio do PROXIMO cabecalho de bloco. Usado pelo cunzip para
 * pular um bloco corrompido sem perder a sincronizacao do arquivo (Modulo 11):
 * mesmo sem confiar no conteudo do bloco, tree_size + compressed_size dizem
 * quantos bytes saltar. Retorna false se 'f'/'h' forem NULL ou se o fseek falhar.
 */
bool cz_skip_block_payload(FILE *f, const CzBlockHeader *h);

#endif /* FORMAT_H */
