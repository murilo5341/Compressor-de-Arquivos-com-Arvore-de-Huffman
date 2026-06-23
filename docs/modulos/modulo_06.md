# Módulo 6 — Compressão de um bloco em memória

Comprime **um bloco de bytes** inteiramente em memória, amarrando as peças dos
módulos anteriores: frequências e árvore (Módulo 3), tabela de códigos (Módulo
4) e escrita bit a bit (Módulo 5). É o primeiro módulo que produz, de fato, um
**payload comprimido** — a partir daqui o projeto realmente reduz tamanho.

## O que faz

- **`block_compress(input, len, out)`** executa o pipeline clássico de Huffman:
  1. conta as frequências dos bytes do bloco (Módulo 3);
  2. monta a árvore de Huffman (Módulo 3);
  3. gera a tabela de códigos por travessia (Módulo 4);
  4. emite, bit a bit, o código de cada byte do bloco (Módulos 4 e 5);
  5. faz o `flush` do último byte parcial (padding de zeros).
- Devolve um **`BlockCompressed`** com o payload comprimido, a **árvore** do
  bloco, o `original_size` e o `compressed_size`.
- **`block_compressed_free`** libera árvore + payload e zera a struct.

## Por que existe

- **modularizacao.md (Módulo 6):** a entrada é um buffer original em memória; a
  saída é o buffer comprimido, a árvore de Huffman, o tamanho original e o
  comprimido. Este módulo é a unidade de trabalho que o **pipeline de threads**
  (Módulos 13/14) vai paralelizar — cada bloco é comprimido de forma
  independente.
- A árvore acompanha o resultado porque os próximos módulos precisam dela: a
  **serialização** no cabeçalho do bloco (Módulo 7) e a **descompressão**
  (Módulo 8), para que o `cunzip` reconstrua e decodifique **sem depender deste
  processo** (RULES REGRA 9).

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `include/block.h` | Declara `BlockCompressed` e a API `block_compress` / `block_compressed_free`. |
| `src/block.c` | Orquestra frequências → árvore → códigos → escrita bit a bit; doa o buffer do `BitWriter` ao resultado. |
| `tests/test_block_compress.c` | Testes: bloco conhecido, redução real, folha única, bloco vazio, 256 bytes, bordas — com **decodificador manual** validando o roundtrip. |

## Estruturas principais

```c
typedef struct {
    HuffNode *tree;            // arvore de Huffman do bloco (NULL so em bloco vazio)
    uint8_t  *data;            // payload comprimido (bytes, MSB-first)
    size_t    compressed_size; // bytes validos em 'data' (inclui padding final)
    size_t    original_size;   // bytes do bloco original (criterio de parada)
} BlockCompressed;
```

O `BlockCompressed` é **dono** da árvore e do payload — `block_compressed_free`
libera ambos. O `original_size` é o que permite parar a decodificação no ponto
certo, ignorando os bits de padding do último byte (contrato do Módulo 5).

## Funções principais

| Função | O que faz |
|--------|-----------|
| `block_compress(input, len, out)` | Comprime `len` bytes e preenche `*out`. `true` em sucesso; `false` em `out` NULL ou falta de memória (deixando `*out` zerado). |
| `block_compressed_free(bc)` | Libera árvore + payload e zera a struct. Seguro com NULL. |

### Algoritmo (resumo)

Frequências → `huffman_build_tree` → `huffman_build_codes` → para cada byte
`b` do bloco, `bit_writer_write_bits(w, table[b].bits, table[b].length)` →
`bit_writer_flush`. Complexidade **O(n)** no tamanho do bloco para a codificação
(mais O(k log k) da construção da árvore, com k = símbolos distintos ≤ 256).

## Decisão de projeto — doação do buffer (sem cópia)

O `BitWriter` já cresce sozinho e termina com o payload completo em
`w.buffer`/`w.byte_count`. No caminho de **sucesso**, em vez de copiar para um
buffer novo e chamar `bit_writer_free`, **doamos** o ponteiro `w.buffer`
diretamente para `out->data`. Por isso `block.c` **não** chama
`bit_writer_free` no sucesso — quem passa a ser dono daquele buffer é o
`BlockCompressed`, liberado por `block_compressed_free`. Evita uma cópia do
payload inteiro (relevante no teste de fogo de 1 GB). Nos caminhos de **erro**,
`bit_writer_free` é chamado normalmente.

## Decisão de projeto — bloco vazio e folha única

- **Bloco vazio** (`len == 0`): resultado válido e vazio (`tree == NULL`,
  `data == NULL`, tamanhos 0). Quem decide a política para arquivo vazio é o
  `czip` (Módulo 10); aqui apenas não quebramos.
- **Folha única** (um único byte distinto): a árvore é uma folha e cada byte
  vira **1 bit** (código "0", definido no Módulo 4). 50 bytes → 50 bits → 7
  bytes após o flush. A decodificação trata esse caso lendo 1 bit por byte e
  emitindo o símbolo da raiz.

## Como compilar e testar

```sh
# Linux (ambiente oficial de validacao)
make test       # roda toda a suite, incluindo test_block_compress
make asan       # (opcional) valida ausencia de vazamentos

# Windows (MinGW)
mingw32-make test
```

Saída esperada (critério de aprovação):

```
=== Testes da compressao de bloco (Modulo 6) ===
Teste 1: bloco conhecido pequeno (saida valida + roundtrip)
Teste 2: distribuicao enviesada reduz o tamanho
Teste 3: folha unica (um unico simbolo repetido)
Teste 4: bloco vazio
Teste 5: todos os 256 valores de byte
Teste 6: bordas (NULL)

30 verificacoes, 0 falha(s).
TODOS OS TESTES PASSARAM.
```

O teste retorna código `!= 0` se qualquer verificação falhar, fazendo o
`make test` falhar.

## Como explicar na defesa

- **O que é um "bloco" e por que comprimir por bloco?** O arquivo é dividido em
  blocos independentes; cada um tem sua própria árvore. Isso permite paralelizar
  (Módulo 13) e isolar corrupção (um bloco corrompido não derruba os outros).
- **Por que a árvore acompanha o bloco comprimido?** Porque o `cunzip` precisa
  dela para decodificar e não pode depender deste processo — ela será serializada
  no cabeçalho do bloco (Módulo 7).
- **Como você prova que a saída é válida sem o descompressor?** O teste inclui um
  decodificador manual que desce a árvore bit a bit até reconstruir
  `original_size` bytes e compara com o original (`memcmp`).
- **Por que não copiar o payload para um buffer próprio?** O `BitWriter` já o
  produziu; doar o ponteiro evita uma cópia do bloco inteiro. A posse passa para
  o `BlockCompressed`.
- **Pergunta provável: "e o último byte incompleto?"** → Tem padding de zeros do
  `flush`; a parada real na descompressão é por `original_size`, não pelos bits.

## Decisões de projeto / referências

- **Doação do buffer do `BitWriter`** ao resultado (sem cópia); `block.c` não
  chama `bit_writer_free` no sucesso.
- **Árvore é dona do resultado**; `block_compressed_free` libera árvore + payload.
- **Bloco vazio** retorna resultado válido vazio; **folha única** gera 1 bit por
  byte.
- Ver também: `modularizacao.md` (Módulo 6), `RULES.md` (REGRAS 5, 9 e 10),
  `include/huffman.h` (Módulos 3/4), `include/bitio.h` (Módulo 5) e a entrada do
  `DIARIO.md` de 2026-06-22 — Módulo 6.
