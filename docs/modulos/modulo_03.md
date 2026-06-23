# Módulo 3 — Construção da Árvore de Huffman

Monta a **árvore de Huffman** de um bloco a partir das frequências dos bytes,
usando o min-heap do Módulo 1 como fila de prioridade. É o primeiro passo do
**núcleo de Estrutura de Dados** (E2): a árvore é o que dá códigos curtos aos
bytes frequentes e longos aos raros, viabilizando a compressão.

## O que faz

- **Conta as frequências** de cada valor de byte (0–255) de um bloco
  (`huffman_count_frequencies`).
- **Constrói a árvore** combinando repetidamente os dois nós de menor frequência
  até sobrar a raiz (`huffman_build_tree`), reaproveitando o min-heap do Módulo 1.
- Trata o **caso especial obrigatório**: bloco com um único byte distinto gera
  uma **folha única**.
- **Libera** a árvore recursivamente (`huffman_free_tree`), sem vazamentos.

## Por que existe

- **modularizacao.md (Módulo 3):** a árvore de Huffman é o núcleo de ED do tema;
  a tabela de códigos (Módulo 4), a serialização (Módulo 7) e a decodificação
  (Módulo 8) dependem dela.
- **RULES REGRA 5 / REGRA 10:** o caso de um único símbolo distinto precisa ser
  tratado desde já (evita código de comprimento zero na (de)codificação) e é um
  dos testes mínimos obrigatórios, junto com o arquivo de 256 valores de byte.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `include/huffman.h` | API pública: `HuffNode`, contagem, construção e liberação da árvore. |
| `src/huffman.c` | Implementação usando o min-heap (`heap.h`) como fila de prioridade. |
| `tests/test_huffman_tree.c` | Testes: contagem, árvore válida, folha única, bloco vazio, 256 bytes, profundidade × frequência. |

## Estruturas principais

```c
typedef struct HuffNode {
    struct HuffNode *left;   // filho esquerdo  (NULL em folha)
    struct HuffNode *right;  // filho direito   (NULL em folha)
    uint64_t         frequency; // folha: freq do byte; interno: soma dos filhos
    uint8_t          symbol;    // byte representado (so faz sentido em folha)
    bool             is_leaf;   // distingue folha de no interno
} HuffNode;
```

A `struct` é **pública** (não opaca) porque os módulos seguintes precisam
percorrer a árvore. Todo nó interno tem **sempre dois filhos** — é isso que
garante a **propriedade de prefixo** dos códigos (nenhum código é prefixo de
outro). A `frequency` de um nó interno é a soma das frequências dos filhos e
serve de chave no heap.

## Funções principais

| Função | O que faz |
|--------|-----------|
| `huffman_count_frequencies(buf, len, freq[256])` | Reinicia `freq` e conta quantas vezes cada byte aparece em `buf`. |
| `huffman_build_tree(freq[256])` | Cria uma folha por byte com freq > 0 e combina os dois menores no heap até sobrar a raiz. |
| `huffman_free_tree(root)` | Libera a árvore recursivamente (seguro com `NULL`). |
| `huff_new_leaf` / `huff_new_internal` (internas) | Constroem folha e nó interno (frequência = soma dos filhos). |
| `heap_drain_and_free` (interna) | Esvazia o heap liberando cada subárvore — usada no caminho de erro de memória. |

### Algoritmo (resumo)

1. Cria uma folha por byte com `freq > 0`, em ordem crescente de byte (0→255).
2. Empurra todas no min-heap (ordenado pela frequência).
3. Enquanto houver mais de um nó: retira os **dois menores**, cria um nó interno
   com `freq = soma`, devolve ao heap.
4. O último nó é a **raiz**.

Complexidade: **O(n log n)** no número de símbolos distintos (≤ 256), dominada
pelas operações de heap. Para um bloco de `B` bytes, a contagem é **O(B)**.

## Como compilar e testar

```sh
# Linux (ambiente oficial de validacao)
make test       # compila e roda test_heap, test_crc32 e test_huffman_tree
make asan       # (opcional) valida ausencia de vazamentos com AddressSanitizer

# Windows (MinGW)
mingw32-make test
```

Saída esperada (critério de aprovação):

```
=== Testes da arvore de Huffman (Modulo 3) ===
Teste 1: contagem de frequencias
Teste 2: construcao de arvore valida
Teste 3: bloco com um unico byte distinto -> folha unica
Teste 4: bloco vazio -> arvore NULL
Teste 5: todos os 256 valores de byte
Teste 6: simbolo mais frequente tem codigo mais curto

31 verificacoes, 0 falha(s).
TODOS OS TESTES PASSARAM.
```

O teste retorna código `!= 0` se qualquer verificação falhar, fazendo o
`make test` falhar.

## Como explicar na defesa

- **Como a árvore é montada?** Uma folha por byte distinto entra num min-heap
  pela frequência; combinam-se os dois menores num nó interno (freq = soma) e
  repete-se até sobrar a raiz. Bytes frequentes acabam mais perto da raiz.
- **Por que reusar o heap do Módulo 1?** O passo central de Huffman é "pegar os
  dois menores": exatamente o que um min-heap faz em O(log n). Mantém o código
  desacoplado e reaproveitado.
- **Por que o caso de um único símbolo é especial?** Com um só byte, o laço não
  executa e a árvore é uma folha única (profundidade 0). Se não tratássemos, o
  código teria comprimento 0 — por isso o Módulo 4 atribui o código `0` (1 bit)
  a esse byte. Aqui só garantimos a folha única.
- **Por que a árvore garante a propriedade de prefixo?** Porque os símbolos só
  ficam em folhas e todo nó interno tem dois filhos: o caminho até uma folha
  nunca passa por outra folha, então nenhum código é prefixo de outro.
- **É determinístico?** Sim: folhas entram em ordem de byte e o heap desempata
  pela ordem de inserção (Módulo 1), então a mesma distribuição gera sempre a
  mesma árvore — bom para testes reproduzíveis.
- **Pergunta provável: "e o bloco vazio?"** → `huffman_build_tree` retorna
  `NULL`; o formato `.cz` tratará blocos de tamanho 0 sem árvore.

## Decisões de projeto / referências

- **Folha única** para bloco com um só byte distinto (modularizacao.md, Módulo
  3); a atribuição do código `0` fica no Módulo 4.
- **Desempate determinístico** herdado do heap (ordem de inserção) + push das
  folhas em ordem de byte → árvore reproduzível.
- **`HuffNode` público** (não opaco): Módulos 4, 7 e 8 percorrem a árvore.
- **Tratamento de falha de memória**: qualquer `malloc` que falhe libera tudo o
  que já foi alocado (`heap_drain_and_free` + liberação dos nós já retirados) e
  retorna `NULL` — sem vazamentos no caminho de erro.
- `huffman.c`/`heap.c` ainda **não** entram em `COMMON_SRCS` do Makefile — só
  serão linkados ao `czip`/`cunzip` quando a compressão de bloco (Módulo 6+) os
  usar. Por ora são linkados apenas no teste.
- Ver também: `modularizacao.md` (Módulo 3), `RULES.md` (REGRAS 5 e 10),
  `include/heap.h` (Módulo 1) e a entrada do `DIARIO.md` de 2026-06-22 — Módulo 3.
