# ============================================================================
# Makefile - Compressor de Arquivos com Arvore de Huffman (Tema 11)
#
# Compilacao obrigatoria (RULES REGRA 2): gcc -std=c11 -Wall -Wextra -Werror
# Tolerancia zero a warnings (0 warnings).
#
# Alvos principais (RULES REGRA 4): all, test, stress, clean
# Alvos extras de validacao (Modulo 0): asan, tsan, valgrind
# ============================================================================

CC      = gcc
CSTD    = -std=c11
WARN    = -Wall -Wextra -Werror
OPT     = -O2
INCLUDE = -Iinclude

# --- Deteccao de plataforma ---------------------------------------------------
# No Linux (ambiente oficial de validacao/correcao) as threads usam -pthread.
# O MinGW no Windows usa o modelo de threads win32 e NAO possui libpthread,
# portanto o -pthread e omitido localmente. A concorrencia (E3) e os sanitizers
# (asan/tsan) e o Valgrind devem ser validados em ambiente Linux.
ifeq ($(OS),Windows_NT)
    PTHREAD =
    RM      = del /Q /F
    EXE     = .exe
    RUN     =
    # MinGW (modelo win32) nao tem libpthread: o czip usa o caminho SEQUENCIAL
    # (Modulo 10) e os testes concorrentes (Modulos 12-14) so entram em Linux.
    CONC_TESTS    =
    PIPELINE_SRCS =
    PTHREAD_DEF   =
else
    PTHREAD = -pthread
    RM      = rm -f
    EXE     =
    RUN     = ./
    # Linux: define HAVE_PTHREAD (czip vai pelo pipeline concorrente) e linka
    # o pipeline (Modulos 13-14) + a fila (Modulo 12) ao czip. test_queue
    # (Modulo 12) e test_pipeline (Modulo 14) entram no `make test`.
    CONC_TESTS    = test_queue test_pipeline
    PIPELINE_SRCS = src/pipeline.c src/queue.c
    PTHREAD_DEF   = -DHAVE_PTHREAD
endif

CFLAGS  = $(CSTD) $(WARN) $(OPT) $(INCLUDE) $(PTHREAD) $(PTHREAD_DEF)
LDFLAGS = $(PTHREAD)

# Modulos de codigo compartilhados entre czip e cunzip.
# Linkados a partir do Modulo 10 (czip sequencial), que usa toda a cadeia:
# compressao de bloco (block.c -> huffman.c + heap.c + bitio.c), serializacao da
# arvore (tree_serialization.c), CRC32 (crc32.c) e o formato .cz (format.c).
COMMON_SRCS = src/block.c src/huffman.c src/heap.c src/bitio.c \
              src/tree_serialization.c src/crc32.c src/format.c

# No Linux, czip leva tambem o pipeline concorrente (pipeline.c) e a fila
# (queue.c). No Windows, PIPELINE_SRCS e vazio e czip usa so o caminho sequencial.
CZIP_SRCS   = src/main_czip.c   $(COMMON_SRCS) $(PIPELINE_SRCS)
CUNZIP_SRCS = src/main_cunzip.c $(COMMON_SRCS)

.PHONY: all test test_heap test_crc32 test_huffman_tree test_huffman_codes test_bitio test_block_compress test_tree_serialization test_block_decompress test_format test_queue test_pipeline stress clean asan tsan valgrind help

# ----------------------------------------------------------------------------
# Compilacao principal
# ----------------------------------------------------------------------------
all: czip cunzip

czip: $(CZIP_SRCS)
	$(CC) $(CFLAGS) $(CZIP_SRCS) -o czip$(EXE) $(LDFLAGS)

cunzip: $(CUNZIP_SRCS)
	$(CC) $(CFLAGS) $(CUNZIP_SRCS) -o cunzip$(EXE) $(LDFLAGS)

# ----------------------------------------------------------------------------
# Testes unitarios
# Cada modulo adiciona aqui a compilacao e execucao do seu teste.
#   Modulo 1 - heap binario (test_heap)
#   Modulo 2 - CRC32 por bloco (test_crc32)
#   Modulo 3 - arvore de Huffman (test_huffman_tree, linka huffman.c + heap.c)
#   Modulo 4 - tabela de codigos (test_huffman_codes, linka huffman.c + heap.c)
#   Modulo 5 - escrita/leitura bit a bit (test_bitio, linka bitio.c)
#   Modulo 6 - compressao de bloco (test_block_compress, linka block.c +
#              huffman.c + heap.c + bitio.c)
#   Modulo 7 - serializacao da arvore (test_tree_serialization, linka
#              tree_serialization.c + huffman.c + heap.c + bitio.c)
#   Modulo 8 - descompressao de bloco (test_block_decompress, linka block.c +
#              huffman.c + heap.c + bitio.c)
#   Modulo 9 - formato .cz (test_format, linka format.c)
#   Modulo 12 - fila bloqueante (test_queue, linka queue.c + -pthread; so Linux)
#   Modulo 14 - escritor reordenador (test_pipeline, roundtrip czip/cunzip; so Linux)
# ----------------------------------------------------------------------------
test: test_heap test_crc32 test_huffman_tree test_huffman_codes test_bitio test_block_compress test_tree_serialization test_block_decompress test_format $(CONC_TESTS)

test_heap: tests/test_heap.c src/heap.c
	$(CC) $(CFLAGS) tests/test_heap.c src/heap.c -o test_heap$(EXE) $(LDFLAGS)
	$(RUN)test_heap$(EXE)

test_crc32: tests/test_crc32.c src/crc32.c
	$(CC) $(CFLAGS) tests/test_crc32.c src/crc32.c -o test_crc32$(EXE) $(LDFLAGS)
	$(RUN)test_crc32$(EXE)

test_huffman_tree: tests/test_huffman_tree.c src/huffman.c src/heap.c
	$(CC) $(CFLAGS) tests/test_huffman_tree.c src/huffman.c src/heap.c -o test_huffman_tree$(EXE) $(LDFLAGS)
	$(RUN)test_huffman_tree$(EXE)

test_huffman_codes: tests/test_huffman_codes.c src/huffman.c src/heap.c
	$(CC) $(CFLAGS) tests/test_huffman_codes.c src/huffman.c src/heap.c -o test_huffman_codes$(EXE) $(LDFLAGS)
	$(RUN)test_huffman_codes$(EXE)

test_bitio: tests/test_bitio.c src/bitio.c
	$(CC) $(CFLAGS) tests/test_bitio.c src/bitio.c -o test_bitio$(EXE) $(LDFLAGS)
	$(RUN)test_bitio$(EXE)

test_block_compress: tests/test_block_compress.c src/block.c src/huffman.c src/heap.c src/bitio.c
	$(CC) $(CFLAGS) tests/test_block_compress.c src/block.c src/huffman.c src/heap.c src/bitio.c -o test_block_compress$(EXE) $(LDFLAGS)
	$(RUN)test_block_compress$(EXE)

test_tree_serialization: tests/test_tree_serialization.c src/tree_serialization.c src/huffman.c src/heap.c src/bitio.c
	$(CC) $(CFLAGS) tests/test_tree_serialization.c src/tree_serialization.c src/huffman.c src/heap.c src/bitio.c -o test_tree_serialization$(EXE) $(LDFLAGS)
	$(RUN)test_tree_serialization$(EXE)

test_block_decompress: tests/test_block_decompress.c src/block.c src/huffman.c src/heap.c src/bitio.c
	$(CC) $(CFLAGS) tests/test_block_decompress.c src/block.c src/huffman.c src/heap.c src/bitio.c -o test_block_decompress$(EXE) $(LDFLAGS)
	$(RUN)test_block_decompress$(EXE)

test_format: tests/test_format.c src/format.c
	$(CC) $(CFLAGS) tests/test_format.c src/format.c -o test_format$(EXE) $(LDFLAGS)
	$(RUN)test_format$(EXE)

# Modulo 12 - fila bloqueante concorrente. Requer pthreads (-pthread), entao so
# e compilado/rodado em Linux; no Windows (MinGW sem libpthread) CONC_TESTS e
# vazio e este alvo nao e invocado pelo `make test`.
test_queue: tests/test_queue.c src/queue.c
	$(CC) $(CFLAGS) tests/test_queue.c src/queue.c -o test_queue$(EXE) $(LDFLAGS)
	$(RUN)test_queue$(EXE)

# Modulo 14 - escritor reordenador. Teste de integracao (shell) que comprime com
# N e 1 thread e exige saidas .cz IDENTICAS (reorder deterministico) + roundtrip
# byte a byte. Depende de czip/cunzip ja compilados (pipeline ligado). So Linux.
test_pipeline: all tests/test_pipeline.sh
	sh tests/test_pipeline.sh

# ----------------------------------------------------------------------------
# Teste de stress / carga (sera implementado no Modulo 17 - teste de fogo)
# ----------------------------------------------------------------------------
stress: all
	@echo Stress test sera implementado a partir do Modulo 17.

# ----------------------------------------------------------------------------
# AddressSanitizer - detecta vazamentos e acesso indevido de memoria (-10%).
# ThreadSanitizer - detecta data races no pipeline concorrente (-15%).
#
# OBS: requerem Linux com gcc/clang que suportem -fsanitize.
#      O MinGW no Windows NAO suporta estes sanitizers; rode em ambiente Linux.
# As variaveis target-specific abaixo propagam para o pre-requisito (all).
# ----------------------------------------------------------------------------
asan: CFLAGS  += -fsanitize=address -g -fno-omit-frame-pointer
asan: LDFLAGS += -fsanitize=address
asan: clean all
	@echo Build com AddressSanitizer concluido.

tsan: CFLAGS  += -fsanitize=thread -g -fno-omit-frame-pointer
tsan: LDFLAGS += -fsanitize=thread
tsan: clean all
	@echo Build com ThreadSanitizer concluido.

# ----------------------------------------------------------------------------
# Valgrind - alternativa para validar vazamentos de memoria (requer Linux).
# Uso apos compilar:
#   valgrind --leak-check=full --error-exitcode=1 ./czip ENTRADA SAIDA.cz
# ----------------------------------------------------------------------------
valgrind: all
	@echo Use: valgrind --leak-check=full --error-exitcode=1 ./czip ENTRADA SAIDA.cz

# ----------------------------------------------------------------------------
clean:
	-$(RM) czip$(EXE) cunzip$(EXE) test_* *.o saida.cz restaurado*

help:
	@echo Alvos disponiveis:
	@echo   all       - compila czip e cunzip
	@echo   test      - compila e roda os testes unitarios
	@echo   stress    - roda o teste de stress/carga
	@echo   asan      - build com AddressSanitizer (Linux)
	@echo   tsan      - build com ThreadSanitizer (Linux)
	@echo   valgrind  - instrucoes de uso do Valgrind (Linux)
	@echo   clean     - remove binarios e artefatos
