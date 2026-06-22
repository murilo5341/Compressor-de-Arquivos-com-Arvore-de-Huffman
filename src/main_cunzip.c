/*
 * main_cunzip.c - Ponto de entrada do descompressor "cunzip".
 *
 * Modulo 0 (estrutura inicial): esqueleto que valida a quantidade minima
 * de argumentos e imprime uma mensagem de uso.
 *
 * A logica real sera adicionada nos modulos seguintes:
 *   - validacao de magic number e versao (Modulo 11);
 *   - leitura dos cabecalhos do .cz, reconstrucao da arvore e bit I/O;
 *   - validacao de CRC32 por bloco e salto de bloco corrompido (Modulo 11).
 */
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <entrada.cz> <saida>\n", argv[0]);
        return 1;
    }

    printf("cunzip: estrutura inicial (Modulo 0). "
           "Descompressao ainda nao implementada.\n");
    return 0;
}
