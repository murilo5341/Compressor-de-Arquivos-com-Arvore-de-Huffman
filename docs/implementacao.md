# Guia de Implementacao - Compressor Huffman

## Objetivo deste documento

Este documento descreve como implementar o Tema 11 de forma incremental, seguindo as regras do projeto. Ele nao substitui os testes nem a defesa oral: a equipe precisa entender cada modulo, cada decisao de formato e cada mecanismo de sincronizacao.

## Estrutura sugerida de diretorios

```text
.
|-- Makefile
|-- README.md
|-- RULES.md
|-- DIARIO.md
|-- docs/
|   |-- context.md
|   |-- implementacao.md
|   |-- trabalho.txt
|   `-- Whatbuild.md
|-- src/
|   |-- main_czip.c
|   |-- main_cunzip.c
|   |-- heap.c
|   |-- heap.h
|   |-- huffman.c
|   |-- huffman.h
|   |-- bitio.c
|   |-- bitio.h
|   |-- crc32.c
|   |-- crc32.h
|   |-- block.c
|   |-- block.h
|   |-- queue.c
|   |-- queue.h
|   |-- pipeline.c
|   `-- pipeline.h
`-- tests/
    |-- test_heap.c
    |-- test_crc32.c
    |-- test_huffman.c
    |-- test_bitio.c
    `-- test_roundtrip.c
```

A estrutura pode ser ajustada, mas a separacao por modulos ajuda na defesa oral e evita um arquivo unico dificil de manter.

## Ordem recomendada de implementacao

### 1. Fundacao do projeto

Criar os diretorios `src/` e `tests/`. Ajustar o `Makefile` para compilar os testes disponiveis sem depender de arquivos ainda inexistentes.

Primeiros modulos recomendados:

- `heap.h` e `heap.c`
- `crc32.h` e `crc32.c`
- testes unitarios de heap e CRC32

O objetivo dessa fase e ter uma base pequena que compile com:

```sh
gcc -std=c11 -Wall -Wextra -Werror
```

### 2. Heap binario minimo

O heap sera usado como fila de prioridade para construir a Arvore de Huffman.

Operacoes necessarias:

- criar heap com capacidade inicial;
- inserir no;
- extrair menor frequencia;
- consultar tamanho;
- destruir heap.

Cada item do heap deve apontar para um no de Huffman ou conter os dados necessarios para comparar frequencias. A comparacao principal deve ser pela frequencia. Em caso de empate, e recomendavel usar um criterio deterministico, como menor byte ou menor ordem de criacao, para facilitar testes reproduziveis.

Testes minimos:

- inserir varios valores e extrair em ordem crescente;
- extrair de heap vazio deve ser tratado sem crash;
- testar empates;
- testar crescimento de capacidade, se houver realocacao dinamica.

### 3. CRC32

O CRC32 sera usado para detectar corrupcao por bloco. A implementacao deve usar apenas libc, sem bibliotecas externas.

Decisao importante:

- CRC32 do conteudo original facilita validar o resultado apos descompressao.
- CRC32 do payload comprimido facilita detectar corrupcao antes da decodificacao.

Uma abordagem robusta e documentar uma escolha e mante-la consistente. Para a primeira versao, CRC32 do conteudo original e simples de validar depois que o bloco for descomprimido.

Testes minimos:

- CRC32 de buffer vazio;
- CRC32 de string conhecida;
- CRC32 muda quando um byte e alterado.

### 4. Nos e arvore de Huffman

Criar `huffman.h` e `huffman.c`.

Struct sugerida para no:

```c
typedef struct huffman_node {
    unsigned char byte;
    unsigned long freq;
    int is_leaf;
    struct huffman_node *left;
    struct huffman_node *right;
} huffman_node_t;
```

Funcoes esperadas:

- contar frequencias de um bloco;
- criar folhas para bytes com frequencia maior que zero;
- construir arvore usando heap binario minimo;
- gerar tabela de codigos por percurso da arvore;
- liberar todos os nos da arvore.

Caso especial importante: bloco com apenas um byte distinto. Nesse caso, a arvore tera uma unica folha e o codigo pode ser definido como `0`.

### 5. Tabela de codigos

A tabela deve mapear cada byte para uma sequencia de bits.

Representacao simples:

```c
typedef struct huffman_code {
    unsigned long long bits;
    unsigned int bit_count;
} huffman_code_t;
```

Essa representacao e simples, mas limita o tamanho maximo do codigo a 64 bits. Para blocos comuns com 256 simbolos, Huffman pode gerar codigos maiores em casos extremos. Uma alternativa mais robusta e armazenar os bits em vetor dinamico de bytes.

Para uma primeira implementacao academica, a equipe deve escolher conscientemente entre simplicidade e robustez, documentando a decisao. Se o objetivo for passar em casos gerais, a representacao com vetor de bytes e mais segura.

### 6. Escrita e leitura de bits

Criar `bitio.h` e `bitio.c`.

O compressor precisa escrever codigos com tamanho em bits, nao em bytes. O descompressor precisa ler esses bits na mesma ordem.

Funcoes recomendadas:

- iniciar escritor de bits sobre `FILE *` ou buffer;
- escrever um bit;
- escrever uma sequencia de bits;
- descarregar byte parcial no final;
- iniciar leitor de bits;
- ler proximo bit;
- detectar fim de fluxo.

O ultimo byte do payload comprimido quase sempre tera bits de preenchimento. Por isso, o formato do bloco deve armazenar o tamanho original do bloco ou a quantidade real de bits comprimidos. Armazenar o tamanho original e suficiente para parar a decodificacao apos reconstruir a quantidade correta de bytes.

### 7. Serializacao da arvore

A arvore precisa ser gravada no cabecalho de cada bloco.

Formato simples de pre-ordem:

- escrever `1` para folha e em seguida os 8 bits do byte;
- escrever `0` para no interno e serializar filho esquerdo e filho direito.

Exemplo conceitual:

```text
folha('A')        -> 1 + 01000001
interno(L, R)     -> 0 + serializacao(L) + serializacao(R)
```

Na leitura, `cunzip` consome os bits da serializacao e reconstrui a mesma arvore.

E importante armazenar no cabecalho o tamanho da arvore serializada, em bytes ou bits, para saber onde termina a arvore e onde comeca o payload comprimido.

### 8. Formato do arquivo `.cz`

Uma versao inicial do formato pode ser:

```text
Arquivo:
  magic[4] = "CZHF"
  version: uint32
  block_size: uint32
  block_count: uint64

Para cada bloco:
  block_index: uint64
  original_size: uint32
  compressed_size: uint32
  tree_size: uint32
  original_crc32: uint32
  tree_bytes[tree_size]
  compressed_bytes[compressed_size]
```

Todas as decisoes de endianess devem ser padronizadas. Para simplificar em ambiente academico, pode-se gravar em little-endian nativo, desde que isso seja documentado. Uma solucao mais portavel escreveria inteiros byte a byte em ordem definida.

### 9. `czip` single-thread

Antes do pipeline concorrente, implementar uma versao sequencial.

Fluxo:

1. Abrir arquivo de entrada.
2. Abrir arquivo de saida `.cz`.
3. Escrever cabecalho global.
4. Ler um bloco.
5. Calcular CRC32 do bloco original.
6. Construir arvore de Huffman do bloco.
7. Gerar tabela de codigos.
8. Serializar arvore.
9. Comprimir payload em bits.
10. Escrever cabecalho do bloco, arvore e payload.
11. Repetir ate EOF.
12. Fechar arquivos e liberar memoria.

Essa versao deve passar em teste de ida e volta antes de qualquer thread ser adicionada.

### 10. `cunzip` single-thread

Fluxo:

1. Abrir arquivo `.cz`.
2. Validar magic number e versao.
3. Ler cabecalho global.
4. Para cada bloco:
   - ler metadados;
   - ler arvore serializada;
   - reconstruir arvore;
   - ler payload comprimido;
   - decodificar ate gerar `original_size` bytes;
   - calcular CRC32 do bloco restaurado;
   - comparar com CRC32 armazenado;
   - gravar bloco se estiver valido ou registrar erro se estiver corrompido.

No caso de bloco corrompido, o programa deve detectar o erro e continuar nos proximos blocos, desde que o formato permita pular corretamente para o proximo bloco usando os tamanhos gravados no cabecalho.

### 11. Pipeline concorrente

Depois da versao single-thread funcionar, implementar o pipeline.

Componentes:

- Thread leitora: le blocos do arquivo e envia para uma fila de entrada.
- Threads trabalhadoras: recebem blocos, comprimem e enviam resultados para fila de saida.
- Thread escritora: recebe resultados e grava em ordem.

Cada bloco deve ter um indice sequencial:

```text
bloco 0, bloco 1, bloco 2, ...
```

Como trabalhadores podem terminar fora de ordem, a escritora deve manter temporariamente blocos futuros ate que o proximo indice esperado esteja disponivel.

Exemplo:

```text
esperado = 3
chegou bloco 5 -> guardar
chegou bloco 4 -> guardar
chegou bloco 3 -> escrever 3, depois 4, depois 5
```

### 12. Filas limitadas com condition variables

As filas entre os estagios devem ter capacidade fixa. Isso evita consumo ilimitado de memoria quando uma etapa e mais rapida que outra.

Uma fila bloqueante precisa de:

- vetor circular;
- capacidade;
- quantidade atual;
- indice de inicio;
- indice de fim;
- mutex;
- condvar `not_empty`;
- condvar `not_full`;
- flag de fechamento.

Operacoes:

- `queue_push`: espera enquanto a fila estiver cheia.
- `queue_pop`: espera enquanto a fila estiver vazia e ainda aberta.
- `queue_close`: acorda consumidores quando nao havera mais producao.

### 13. Testes obrigatorios

Testes unitarios:

- heap;
- CRC32;
- bit writer/reader;
- construcao de Huffman;
- serializacao e desserializacao da arvore.

Testes de integracao:

- arquivo vazio;
- arquivo com um unico byte;
- arquivo com bytes repetidos;
- arquivo de texto;
- arquivo binario pequeno;
- arquivo aleatorio;
- roundtrip: `czip entrada saida.cz` e `cunzip saida.cz restaurado`, seguido de comparacao byte a byte.

Testes de robustez:

- corromper manualmente um byte de um bloco;
- verificar se o erro e reportado;
- verificar se os demais blocos sao processados.

Testes de concorrencia:

- executar com 1, 2, 4, 8 e 16 threads;
- comparar saida com arquivo original;
- rodar com ThreadSanitizer;
- rodar com AddressSanitizer ou Valgrind.

### 14. Relatorio experimental

Coletar dados reais para:

- tempo de compressao por tamanho de arquivo;
- speedup por numero de threads;
- taxa de compressao por tipo de arquivo;
- overhead do CRC32;
- identificacao do gargalo do pipeline.

Tipos de entrada recomendados:

- texto natural;
- logs;
- arquivo binario;
- arquivo com bytes repetidos;
- arquivo aleatorio;
- arquivo grande de aproximadamente 1 GB para o teste de fogo.

### 15. Caminho incremental recomendado

Sequencia pratica:

1. Implementar heap e teste.
2. Implementar CRC32 e teste.
3. Implementar Huffman em memoria e teste da tabela de codigos.
4. Implementar bit I/O e teste.
5. Implementar serializacao da arvore e teste.
6. Implementar compressao e descompressao de um bloco em memoria.
7. Implementar `czip/cunzip` single-thread.
8. Implementar deteccao de corrupcao por bloco.
9. Implementar filas bloqueantes.
10. Implementar pipeline com threads.
11. Implementar reordenador.
12. Rodar testes de stress e gerar dados do relatorio.

## Principio de implementacao

O projeto deve ser feito em pequenas entregas. Cada etapa precisa compilar sem warnings, ter teste proprio e ser registrada no `DIARIO.md` quando houver uso de IA. A prioridade deve ser corretude e clareza, porque a defesa oral tem peso alto e qualquer integrante precisa conseguir explicar e modificar o codigo ao vivo.
