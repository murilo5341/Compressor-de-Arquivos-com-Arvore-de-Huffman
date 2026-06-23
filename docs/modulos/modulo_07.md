# Módulo 7 — Serialização da árvore de Huffman

Grava a árvore de Huffman de um bloco como uma sequência compacta de **bits** e a
reconstrói de volta. É o que permite ao `cunzip` montar a mesma árvore e
decodificar **sem depender do processo** que compactou (RULES REGRA 9).

## O que faz

- **`tree_serialize(root, &bytes, &size)`** percorre a árvore em **pré-ordem**
  escrevendo bits com o `BitWriter` (Módulo 5):
  - **folha** → bit `1` seguido dos **8 bits** do byte (`symbol`);
  - **nó interno** → bit `0` seguido da serialização do filho **esquerdo** e
    depois do **direito**.
- **`tree_deserialize(bytes, size)`** faz a travessia recursiva simétrica lendo
  bits com o `BitReader` e devolve a raiz reconstruída.
- O formato é **auto-delimitado**: a leitura consome exatamente os bits de cada
  nó, então a recursão para sozinha — os bits de padding do último byte são
  ignorados.

## Por que existe

- **modularizacao.md (Módulo 7):** a árvore precisa ser salva no cabeçalho de
  cada bloco. Cada bloco tem a **sua** árvore, então a serialização é obrigatória
  para a descompressão independente.
- O cabeçalho do bloco (Módulo 9) grava o **tamanho da árvore serializada em
  bytes** (`size`), para saber onde a árvore termina e o payload começa — e para
  **saltar** um bloco corrompido somando `tree_size + compressed_size`.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `include/tree_serialization.h` | Declara `tree_serialize` / `tree_deserialize` e documenta o formato pré-ordem. |
| `src/tree_serialization.c` | Travessias recursivas de escrita/leitura sobre o bit I/O do Módulo 5. |
| `tests/test_tree_serialization.c` | Roundtrip estrutural + igualdade da tabela de códigos, folha única, 256 bytes, truncamento e bordas. |

## Estruturas principais

Não cria struct nova — opera sobre o `HuffNode` (Módulo 3) e o `BitWriter`/
`BitReader` (Módulo 5). A saída da serialização é apenas um buffer de bytes
recém-alocado (doado ao chamador) e seu tamanho.

## Funções principais

| Função | O que faz |
|--------|-----------|
| `tree_serialize(root, &bytes, &size)` | Serializa em pré-ordem para buffer novo. `false` em NULL/falta de memória (deixando saídas zeradas). Chamador libera `bytes` com `free`. |
| `tree_deserialize(bytes, size)` | Reconstrói a árvore (liberar com `huffman_free_tree`); `NULL` se truncada/corrompida ou sem memória. |

### Custo

Pré-ordem visita cada nó uma vez: **O(k)** nós, com k = símbolos distintos ≤ 256
(no máximo 511 nós). Tamanho serializado = `nº_folhas*9 + nº_internos*1` bits.

## Decisões de projeto

- **Pré-ordem 1=folha/0=interno**: formato mínimo e auto-delimitado, sem precisar
  de contador de nós nem marcador de fim — a estrutura da árvore basta.
- **Tamanho em bytes** (não em bits) no resultado: casa com o `tree_size` do
  cabeçalho (Módulo 9); os bits de padding finais são inofensivos porque a
  recursão da leitura para ao completar a árvore.
- **`frequency` não é restaurado** na desserialização (fica 0): a decodificação
  só precisa de `is_leaf`, `symbol` e dos filhos.
- **Doação do buffer** do `BitWriter` ao chamador (sem cópia), mesmo padrão do
  Módulo 6.
- **Robustez**: fim de fluxo no meio da árvore → `tree_deserialize` retorna
  `NULL` sem crash (suporte ao salto de bloco corrompido do Módulo 9/11).

## Como compilar e testar

```sh
# Linux
make test       # roda toda a suite, incluindo test_tree_serialization

# Windows (MinGW)
mingw32-make test
```

Saída esperada:

```
=== Testes da serializacao da arvore (Modulo 7) ===
Teste 1: roundtrip de arvore comum (estrutura + tabela de codigos)
Teste 2: folha unica (um unico byte distinto)
Teste 3: todos os 256 valores de byte
Teste 4: serializacao truncada/corrompida -> NULL sem crash
Teste 5: bordas (NULL / size 0)

30 verificacoes, 0 falha(s).
TODOS OS TESTES PASSARAM.
```

## Como explicar na defesa

- **Por que serializar a árvore?** Cada bloco tem a sua; sem ela no cabeçalho, o
  `cunzip` não teria como decodificar de forma independente.
- **Por que o formato não precisa de marcador de fim?** A pré-ordem é
  auto-delimitada: cada folha consome 9 bits e cada interno 1 bit; ao montar a
  árvore inteira, a recursão para — o padding final é ignorado.
- **Como prova que está correto?** Serializa, desserializa e compara a estrutura
  recursivamente **e** as tabelas de códigos geradas pelas duas árvores: iguais ⇒
  (de)codificação idêntica.
- **Pergunta provável: "e se os bytes estiverem truncados?"** → A leitura detecta
  fim de fluxo no meio e retorna `NULL` sem crash; o salto de bloco corrompido
  (Módulo 9/11) usa os tamanhos do cabeçalho.

## Decisões de projeto / referências

- Serialização **pré-ordem** bit a bit reusando o bit I/O do Módulo 5.
- Ver também: `modularizacao.md` (Módulo 7), `RULES.md` (REGRAS 5, 9 e 10),
  `include/huffman.h` (Módulo 3), `include/bitio.h` (Módulo 5) e a entrada do
  `DIARIO.md` de 2026-06-22 — Módulo 7.
