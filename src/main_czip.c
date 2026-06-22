/*
 * main_czip.c - Ponto de entrada do compressor "czip".
 *
 * Modulo 0 (estrutura inicial): este arquivo e apenas um esqueleto que
 * valida a quantidade minima de argumentos e imprime uma mensagem de uso.
 *
 * A logica real sera adicionada nos modulos seguintes:
 *   - parsing de --threads e --block-size (Modulo 10);
 *   - leitura por blocos, CRC32, Huffman e bit I/O (Modulos 2-9);
 *   - pipeline concorrente de compressao (Modulos 12-14).
 */
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr,
                "Uso: %s [--threads N] [--block-size BYTES] <entrada> <saida.cz>\n",
                argv[0]);
        return 1;
    }

    printf("czip: estrutura inicial (Modulo 0). "
           "Compressao ainda nao implementada.\n");
    return 0;
}
