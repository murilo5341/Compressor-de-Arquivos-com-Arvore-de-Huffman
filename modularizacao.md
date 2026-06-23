Plano modular recomendado

Este plano foi revisado conforme as correcoes da sessao TerminalSession/claudeSession1.md,
incorporando as 5 lacunas criticas e as lacunas menores apontadas. O CRC32 foi movido para a
fundacao (E1), o formato .cz ja nasce com CRC32 e endianess, a CLI de threads/tamanho de bloco
entra no czip, os sanitizers entram no Makefile e foi criada uma suite de roundtrip com os
edge-cases obrigatorios.

Convencoes globais

📓 DIARIO.md (RULES REGRA 1): toda interacao com IA que gere ou modifique codigo deve ser
registrada no DIARIO.md ANTES do commit. Cada modulo abaixo repete esse lembrete no fim.

Compilacao obrigatoria: gcc -std=c11 -Wall -Wextra -Werror (0 warnings).

Sem bibliotecas externas alem da libc e pthreads.

Mapa de entregas (E1–E4)

E1 — Fundacao (15%): Modulo 0, Modulo 1, Modulo 2
E2 — Nucleo de ED (40%): Modulo 3, Modulo 4, Modulo 5, Modulo 6, Modulo 7, Modulo 8, Modulo 9, Modulo 10, Modulo 11
E3 — Nucleo de SO (10%): Modulo 12, Modulo 13, Modulo 14
E4 — Robustez e relatorio (35%): Modulo 15, Modulo 16, Modulo 17, Modulo 18

==================================================================
E1 — FUNDACAO
==================================================================

Módulo 0 — Estrutura inicial do projeto

Objetivo: criar a base do projeto e do build.

Arquivos:

Makefile
README.md
DIARIO.md
include/
src/
tests/
scripts/

Entregas:

Makefile com alvos all, test, stress, clean
Alvos extras de sanitizers: asan (AddressSanitizer), tsan (ThreadSanitizer)
Uso de Valgrind documentado no README/Makefile (ex.: make valgrind ou comando documentado)
binarios czip e cunzip ainda simples
estrutura de pastas organizada

Observacao: os alvos asan/tsan existem desde o inicio para que a validacao de vazamentos
(−10%) e data races (−15%) seja rotina, e nao deixada para o fim.

📓 DIARIO.md: registrar a criacao da estrutura e dos alvos de sanitizer.

Commit sugerido:

git commit -m "Configura estrutura inicial do projeto com alvos asan e tsan"

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

Detalhe: a comparacao principal e pela frequencia. Em caso de empate, usar um criterio
deterministico (ex.: menor byte ou menor ordem de criacao) para tornar os testes reproduziveis.

Teste:

Inserir frequências fora de ordem e garantir que saem em ordem crescente.
Extrair de heap vazio sem crash.
Testar empates.

📓 DIARIO.md: registrar a implementacao do heap.

Commit sugerido:

git commit -m "Implementa heap binario minimo"

Módulo 2 — CRC32 por bloco

Objetivo: detectar corrupcao por bloco. (Movido para a fundacao: o CRC32 pertence a E1 e
precisa existir antes de fechar o formato .cz, evitando retrabalho do cabecalho.)

Arquivos:

include/crc32.h
src/crc32.c
tests/test_crc32.c

Politica de CRC32 (documentar ANTES de implementar — RULES REGRA 5):

Decisao: calcular o CRC32 do CONTEUDO ORIGINAL restaurado de cada bloco. Na compressao, grava-se
o CRC32 do bloco original no cabecalho; na descompressao, recalcula-se o CRC32 do bloco restaurado
e compara-se. Essa escolha facilita validar o resultado apos a descompressao e deve permanecer
consistente em todo o projeto.

Uso:

Na compressão: calcular CRC32 do bloco original e salvar no cabeçalho.
Na descompressão: recalcular CRC32 do bloco restaurado e comparar.

Teste (RULES REGRA 10):

CRC32 de buffer vazio.
CRC32 de string conhecida (valor esperado fixo).
CRC32 muda quando um byte e alterado.

📓 DIARIO.md: registrar a politica de CRC32 escolhida e a implementacao.

Commit sugerido:

git commit -m "Adiciona CRC32 por bloco com politica documentada"

==================================================================
E2 — NUCLEO DE ED
==================================================================

Módulo 3 — Construção da Árvore de Huffman

Objetivo: montar a árvore a partir das frequências.

Arquivos:

include/huffman.h
src/huffman.c
tests/test_huffman_tree.c

Funções:

huffman_count_frequencies()
huffman_build_tree()
huffman_free_tree()

Caso especial obrigatorio: bloco com apenas um byte distinto. Nesse caso a arvore tera uma unica
folha e o codigo desse byte deve ser definido como 0 (um unico bit). Tratar esse caso desde ja
evita codigo de comprimento zero e divisao logica incorreta na (de)codificacao.

Teste:

Dado um vetor de bytes, contar frequências e montar uma árvore válida.
Bloco com um unico byte distinto gera folha unica com codigo 0.

📓 DIARIO.md: registrar a construcao da arvore e o tratamento da folha unica.

Commit sugerido:

git commit -m "Implementa construcao da arvore de Huffman com caso de folha unica"

Módulo 4 — Tabela de códigos de Huffman

Objetivo: gerar os códigos binários a partir da árvore.

Arquivos:

include/huffman.h
src/huffman.c
tests/test_huffman_codes.c

Estrutura:

typedef struct {
    uint64_t bits;
    uint8_t length;
} HuffCode;

Decisao de projeto (documentar — lacuna critica #3): a representacao acima usa uint64_t bits, o
que LIMITA o comprimento maximo de um codigo a 64 bits. Distribuicoes de frequencia patologicas
(ex.: tipo Fibonacci) podem gerar codigos mais longos e estourar silenciosamente esse limite. A
equipe deve escolher conscientemente entre:

(a) manter uint64_t e documentar/assegurar (ex.: validar que nenhum codigo passa de 64 bits, ou
    limitar a profundidade da arvore); ou
(b) representar os bits em vetor dinamico de bytes, mais robusto para o caso geral de 256 simbolos.

A escolha precisa ficar registrada no DIARIO.md e ser defensavel oralmente.

Teste:

Verificar se bytes frequentes recebem códigos mais curtos.
Verificar a propriedade de prefixo: nenhum código é prefixo de outro.

📓 DIARIO.md: registrar a decisao da representacao de codigos (64 bits vs vetor de bytes).

Commit sugerido:

git commit -m "Adiciona geracao da tabela de codigos Huffman com decisao de representacao documentada"

Módulo 5 — Escrita e leitura bit a bit

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

Tratamento do ultimo byte parcial e padding: o ultimo byte do payload comprimido quase sempre tem
bits de preenchimento. O flush completa o byte parcial. Na leitura, a decodificacao para quando a
quantidade correta de bytes originais e reconstruida (criterio original_size do bloco), ignorando
os bits de padding finais. Esse contrato deve estar explicito.

Teste:

Escrever uma sequência de bits que NAO termina em multiplo de 8, ler novamente e conferir se é igual.

📓 DIARIO.md: registrar o bit I/O e o tratamento do byte parcial.

Commit sugerido:

git commit -m "Implementa leitura e escrita bit a bit com tratamento de byte parcial"

Módulo 6 — Compressão de um bloco em memória

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

📓 DIARIO.md: registrar a compressao de bloco em memoria.

Commit sugerido:

git commit -m "Implementa compressao de bloco com Huffman"

Módulo 7 — Serialização da árvore

Objetivo: salvar a árvore no arquivo comprimido.

Arquivos:

include/tree_serialization.h
src/tree_serialization.c
tests/test_tree_serialization.c

Ideia (pre-ordem):

1 = folha + 8 bits do byte da folha
0 = nó interno + serializacao(esquerda) + serializacao(direita)

Gravar no cabecalho do bloco o tamanho da arvore serializada (em bytes ou bits) para saber onde a
arvore termina e o payload comeca.

Teste:

Serializar árvore, desserializar e conferir se a estrutura reconstruída decodifica igual.

📓 DIARIO.md: registrar a serializacao da arvore.

Commit sugerido:

git commit -m "Implementa serializacao da arvore de Huffman"

Módulo 8 — Descompressão de um bloco em memória

Objetivo: reconstruir bytes originais a partir dos bits comprimidos e da árvore.

Arquivos:

src/block.c
tests/test_block_decompress.c

Funções:

block_decompress()

Teste:

Comprimir bloco → descomprimir bloco → comparar com original.

📓 DIARIO.md: registrar a descompressao de bloco.

Commit sugerido:

git commit -m "Implementa descompressao de bloco Huffman"

Módulo 9 — Formato do arquivo .cz

Objetivo: definir e implementar os cabeçalhos, ja com integridade e robustez como premissas.

Arquivos:

include/format.h
src/format.c
tests/test_format.c

Decisao de projeto: extensao .cz e magic number CZHF sao escolhas da equipe (nao exigencias do
edital), documentadas e justificaveis na defesa.

Cabeçalho global mínimo:

magic[4] = "CZHF"
version: uint32
block_size: uint32
block_count: uint64
Endianess DOCUMENTADA (ex.: little-endian nativo documentado, ou inteiros gravados byte a byte em
ordem definida).

Cabeçalho de cada bloco:

block_index: uint64
original_size: uint32
compressed_size: uint32
tree_size: uint32
original_crc32: uint32   (campo ja previsto aqui — definido no Modulo 2, sem retrabalho)
tree_bytes[tree_size]
compressed_bytes[compressed_size]

Premissa de design (nao apenas teste): o formato DEVE permitir localizar o inicio do proximo
bloco mesmo quando o bloco atual estiver corrompido, usando tree_size + compressed_size para
saltar o bloco. Tambem deve permitir parar a decodificacao do payload no ponto correto via
original_size, tratando o byte parcial final.

📓 DIARIO.md: registrar o formato, a endianess e a capacidade de salto de bloco corrompido.

Commit sugerido:

git commit -m "Define formato .cz com CRC32, endianess e salto de bloco corrompido"

Módulo 10 — czip sequencial

Objetivo: criar o compressor funcionando sem threads, com CLI configuravel.

Arquivos:

src/main_czip.c

CLI (RULES REGRA 5/7 — obrigatorio para o teste de fogo e o relatorio de speedup):

czip deve aceitar configuracao do numero de threads e do tamanho de bloco por linha de comando,
por exemplo:

./czip --threads N --block-size BYTES entrada.txt saida.cz

Na versao sequencial, --threads pode ser aceito e ignorado (ou validado), mas o parsing ja deve
existir para nao retrabalhar no pipeline.

Fluxo:

abrir arquivo original
escrever cabecalho global
ler bloco (tamanho conforme --block-size)
calcular CRC32 do bloco original
comprimir bloco
gravar cabecalho do bloco, arvore e payload no .cz
repetir até fim do arquivo

Teste:

./czip --block-size 65536 entrada.txt saida.cz

📓 DIARIO.md: registrar o czip sequencial e o parsing de CLI.

Commit sugerido:

git commit -m "Implementa czip sequencial com CLI de threads e tamanho de bloco"

Módulo 11 — cunzip sequencial

Objetivo: criar o descompressor funcionando sem threads.

Arquivos:

src/main_cunzip.c

Fluxo:

abrir .cz
validar magic number e versao
ler cabeçalho global
para cada bloco:
  ler metadados
  ler árvore de cada bloco e reconstruir
  ler dados comprimidos
  descomprimir ate original_size bytes
  recalcular CRC32 do bloco restaurado e comparar com o armazenado
  gravar bloco se valido; se corrompido, reportar e pular para o proximo bloco

Teste:

./czip entrada.txt saida.cz
./cunzip saida.cz restaurado.txt
cmp entrada.txt restaurado.txt

📓 DIARIO.md: registrar o cunzip sequencial e a validacao de CRC32.

Commit sugerido:

git commit -m "Implementa cunzip sequencial com validacao de CRC32"

==================================================================
E3 — NUCLEO DE SO
==================================================================

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
flag de fechamento (close)

Operacoes:

queue_push: espera enquanto a fila estiver cheia.
queue_pop: espera enquanto a fila estiver vazia e ainda aberta.
queue_close: acorda consumidores quando nao havera mais producao.

📓 DIARIO.md: registrar a fila bloqueante.

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
N threads codificadoras  (N vem de --threads, parsing reforcado aqui)
fila de blocos comprimidos
thread escritora

Validacao obrigatoria: executar com ThreadSanitizer (make tsan) e garantir ausencia de data races
(−15% se houver race reportada). Apos a Entrega 2, nao usar mutex global.

📓 DIARIO.md: registrar o pipeline e o resultado da execucao com TSan.

Commit sugerido:

git commit -m "Implementa pipeline paralelo de compressao com CLI de threads"

Módulo 14 — Escritor reordenador

Objetivo: garantir que blocos terminados fora de ordem sejam gravados em ordem correta.

Arquivos:

src/pipeline.c

Estratégia:

cada bloco tem block_id
escritor mantém next_to_write
blocos fora de ordem ficam em vetor/estrutura pending_blocks ate poderem ser gravados

Validacao obrigatoria: executar com AddressSanitizer (make asan) e/ou Valgrind para garantir
ausencia de vazamentos no caminho feliz (−10% se houver leak).

📓 DIARIO.md: registrar a reordenacao e o resultado de ASan/Valgrind.

Commit sugerido:

git commit -m "Adiciona reordenacao de blocos no escritor"

==================================================================
E4 — ROBUSTEZ E RELATORIO
==================================================================

Módulo 15 — Suíte de roundtrip e edge-cases

Objetivo: cobrir os casos de teste obrigatorios da RULES REGRA 10.

Arquivos:

tests/test_roundtrip.sh  (ou .c)
scripts/gen_edge_cases.sh

Casos obrigatorios:

arquivo vazio
arquivo com um único byte
arquivo com um único símbolo repetido
arquivo contendo todos os 256 valores possíveis de byte
arquivo de texto
arquivo binário pequeno
arquivo aleatório
roundtrip byte a byte: czip seguido de cunzip e comparacao (cmp) com o original
corrupção da ARVORE serializada ou METADADOS do bloco: erro reportado SEM crash
  (distinto de corromper o payload — ver Modulo 16)

📓 DIARIO.md: registrar a suite de roundtrip e os edge-cases cobertos.

Commit sugerido:

git commit -m "Adiciona suite de roundtrip com edge-cases obrigatorios"

Módulo 16 — Teste de corrupção de payload

Objetivo: cumprir parte do teste de fogo (deteccao + recuperacao parcial).

Arquivos:

tests/test_corruption.sh

Teste:

comprimir arquivo
alterar manualmente um byte do payload de um bloco do .cz
rodar cunzip
verificar se o bloco corrompido e detectado por CRC32
verificar se os demais blocos continuam sendo descomprimidos corretamente

📓 DIARIO.md: registrar o teste de corrupcao de payload.

Commit sugerido:

git commit -m "Adiciona teste de deteccao de bloco corrompido"

Módulo 17 — Stress e benchmarks (teste de fogo)

Objetivo: gerar dados reais para o relatório e executar o teste de fogo.

Arquivos:

scripts/gen_inputs.sh
scripts/run_bench.sh
results/resultados.csv

Teste de fogo (RULES REGRA 5 / trabalho.txt item 11):

arquivo de aproximadamente 1 GB comprimido com sucesso
medir tempo com 1, 2, 4, 8 e 16 threads
demonstrar speedup quase linear ate o numero de nucleos disponiveis
identificar quando o ESTAGIO DE ESCRITA passa a serializar o pipeline (analise do gargalo)

Medições:

tempo por numero de threads
taxa de compressão por tipo de arquivo
throughput
speedup
overhead do CRC32

📓 DIARIO.md: registrar o teste de fogo de 1 GB e a analise do gargalo de escrita.

Commit sugerido:

git commit -m "Adiciona scripts de stress, benchmark e teste de fogo de 1 GB"

Módulo 18 — Gráficos e relatório

Objetivo: parte final da entrega.

Arquivos:

scripts/plot_results.sh + scripts/plot_results.gp  (gnuplot; decisao registrada no DIARIO)
relatorio/

Gráficos (scripts reproduziveis versionados — gnuplot):

speedup por número de threads (1, 2, 4, 8, 16)
tempo de compressão
taxa de compressão por tipo de arquivo
throughput
overhead do CRC32

📓 DIARIO.md: registrar a geracao dos graficos e do relatorio.

Commit sugerido:

git commit -m "Adiciona scripts de graficos do relatorio"

Observacao: o video de ate 5 minutos do teste de fogo (entrega final) sera produzido e registrado
manualmente pela equipe em momento futuro, fora deste plano modular.
