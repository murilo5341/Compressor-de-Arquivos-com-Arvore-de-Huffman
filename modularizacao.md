Plano modular recomendado
Módulo 0 — Estrutura inicial do projeto

Objetivo: criar a base.

Arquivos:

Makefile
README.md
DIARIO.md
include/
src/
tests/
scripts/

Entregas:

Makefile com all, test, stress, clean
binários czip e cunzip ainda simples
estrutura de pastas organizada

Commit sugerido:

git commit -m "Configura estrutura inicial do projeto"
Módulo 1 — Heap binário mínimo

Objetivo: criar a fila de prioridade usada pelo Huffman.

Arquivos:

include/heap.h
src/heap.c
tests/test_heap.c

Funções:

heap_create()
heap_push()
heap_pop_min()
heap_destroy()

Teste:

Inserir frequências fora de ordem e garantir que saem em ordem crescente.

Commit sugerido:

git commit -m "Implementa heap binario minimo"
Módulo 2 — Construção da Árvore de Huffman

Objetivo: montar a árvore a partir das frequências.

Arquivos:

include/huffman.h
src/huffman.c
tests/test_huffman_tree.c

Funções:

huffman_count_frequencies()
huffman_build_tree()
huffman_free_tree()

Teste:

Dado um vetor de bytes, contar frequências e montar uma árvore válida.

Commit sugerido:

git commit -m "Implementa construcao da arvore de Huffman"
Módulo 3 — Tabela de códigos de Huffman

Objetivo: gerar os códigos binários a partir da árvore.

Arquivos:

include/huffman.h
src/huffman.c
tests/test_huffman_codes.c

Funções:

huffman_generate_codes()

Estrutura:

typedef struct {
    uint64_t bits;
    uint8_t length;
} HuffCode;

Teste:

Verificar se bytes frequentes recebem códigos e se nenhum código é prefixo indevido de outro.

Commit sugerido:

git commit -m "Adiciona geracao da tabela de codigos Huffman"
Módulo 4 — Escrita e leitura bit a bit

Objetivo: permitir gravar códigos de Huffman como bits reais, não como caracteres '0' e '1'.

Arquivos:

include/bitio.h
src/bitio.c
tests/test_bitio.c

Funções:

bit_writer_init()
bit_writer_write_bit()
bit_writer_write_bits()
bit_writer_flush()

bit_reader_init()
bit_reader_read_bit()

Teste:

Escrever uma sequência de bits, ler novamente e conferir se é igual.

Commit sugerido:

git commit -m "Implementa leitura e escrita bit a bit"
Módulo 5 — Compressão de um bloco em memória

Objetivo: comprimir um bloco de bytes usando Huffman.

Arquivos:

include/block.h
src/block.c
tests/test_block_compress.c

Entrada:

buffer original em memória

Saída:

buffer comprimido
árvore de Huffman
tamanho original
tamanho comprimido

Teste:

Comprimir um pequeno bloco conhecido e verificar se gerou saída válida.

Commit sugerido:

git commit -m "Implementa compressao de bloco com Huffman"
Módulo 6 — Serialização da árvore

Objetivo: salvar a árvore no arquivo comprimido.

Arquivos:

include/tree_serialization.h
src/tree_serialization.c
tests/test_tree_serialization.c

Ideia:

0 = nó interno
1 = folha + byte da folha

Teste:

Serializar árvore, desserializar e conferir se a estrutura reconstruída decodifica igual.

Commit sugerido:

git commit -m "Implementa serializacao da arvore de Huffman"
Módulo 7 — Descompressão de um bloco em memória

Objetivo: reconstruir bytes originais a partir dos bits comprimidos e da árvore.

Arquivos:

src/block.c
tests/test_block_decompress.c

Funções:

block_decompress()

Teste:

Comprimir bloco → descomprimir bloco → comparar com original.

Commit sugerido:

git commit -m "Implementa descompressao de bloco Huffman"
Módulo 8 — Formato do arquivo .cz

Objetivo: definir e implementar cabeçalhos.

Arquivos:

include/format.h
src/format.c
tests/test_format.c

Formato:

Cabeçalho geral do arquivo
Cabeçalho de cada bloco
Árvore serializada
Dados comprimidos

Commit sugerido:

git commit -m "Define formato do arquivo comprimido"
Módulo 9 — czip sequencial

Objetivo: criar o compressor funcionando sem threads.

Arquivos:

src/main_czip.c

Fluxo:

abrir arquivo original
ler bloco
comprimir bloco
gravar no .cz
repetir até fim do arquivo

Teste:

./czip entrada.txt saida.cz

Commit sugerido:

git commit -m "Implementa czip sequencial"
Módulo 10 — cunzip sequencial

Objetivo: criar o descompressor funcionando sem threads.

Arquivos:

src/main_cunzip.c

Fluxo:

abrir .cz
ler cabeçalho
ler árvore de cada bloco
ler dados comprimidos
descomprimir
gravar saída original

Teste:

./czip entrada.txt saida.cz
./cunzip saida.cz restaurado.txt
cmp entrada.txt restaurado.txt

Commit sugerido:

git commit -m "Implementa cunzip sequencial"
Módulo 11 — CRC32 por bloco

Objetivo: detectar corrupção.

Arquivos:

include/crc32.h
src/crc32.c
tests/test_crc32.c

Uso:

Na compressão: calcular CRC32 do bloco original e salvar no cabeçalho.
Na descompressão: recalcular CRC32 do bloco restaurado e comparar.

Commit sugerido:

git commit -m "Adiciona CRC32 por bloco"
Módulo 12 — Fila bloqueante com mutex e condvar

Objetivo: base da concorrência.

Arquivos:

include/queue.h
src/queue.c
tests/test_queue.c

Estrutura:

fila circular limitada
pthread_mutex_t
pthread_cond_t not_empty
pthread_cond_t not_full

Commit sugerido:

git commit -m "Implementa fila bloqueante com condition variables"
Módulo 13 — Pipeline de compressão com threads

Objetivo: implementar a parte de Sistemas Operacionais.

Arquivos:

include/pipeline.h
src/pipeline.c
src/main_czip.c

Fluxo:

thread leitora
fila de blocos crus
N threads codificadoras
fila de blocos comprimidos
thread escritora

Commit sugerido:

git commit -m "Implementa pipeline paralelo de compressao"
Módulo 14 — Escritor reordenador

Objetivo: garantir que blocos terminados fora de ordem sejam gravados em ordem correta.

Arquivos:

src/pipeline.c

Estratégia:

cada bloco tem block_id
escritor mantém next_to_write
blocos fora de ordem ficam em vetor pending_blocks

Commit sugerido:

git commit -m "Adiciona reordenacao de blocos no escritor"
Módulo 15 — Teste de corrupção

Objetivo: cumprir parte do teste de fogo.

Arquivos:

tests/test_corruption.sh

Teste:

comprimir arquivo
alterar manualmente um byte do .cz
rodar cunzip
verificar se detecta bloco corrompido
continuar processando os demais blocos

Commit sugerido:

git commit -m "Adiciona teste de deteccao de bloco corrompido"
Módulo 16 — Stress e benchmarks

Objetivo: gerar dados para relatório.

Arquivos:

scripts/gen_inputs.sh
scripts/run_bench.sh
results/resultados.csv

Medições:

tempo com 1, 2, 4, 8, 16 threads
taxa de compressão
throughput
speedup

Commit sugerido:

git commit -m "Adiciona scripts de stress e benchmark"
Módulo 17 — Gráficos e relatório

Objetivo: parte final da entrega.

Arquivos:

scripts/plot_results.py
relatorio/

Gráficos:

speedup por número de threads
tempo de compressão
taxa de compressão
throughput

Commit sugerido:

git commit -m "Adiciona scripts de graficos do relatorio"