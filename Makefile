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
else
    PTHREAD = -pthread
    RM      = rm -f
    EXE     =
    RUN     = ./
endif

CFLAGS  = $(CSTD) $(WARN) $(OPT) $(INCLUDE) $(PTHREAD)
LDFLAGS = $(PTHREAD)

# Modulos de codigo compartilhados entre czip e cunzip.
# Serao preenchidos conforme os modulos avancam (heap.c, crc32.c, huffman.c, ...).
COMMON_SRCS =

CZIP_SRCS   = src/main_czip.c   $(COMMON_SRCS)
CUNZIP_SRCS = src/main_cunzip.c $(COMMON_SRCS)

.PHONY: all test test_heap test_crc32 test_huffman_tree test_huffman_codes test_bitio stress clean asan tsan valgrind help

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
# ----------------------------------------------------------------------------
test: test_heap test_crc32 test_huffman_tree test_huffman_codes test_bitio

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
