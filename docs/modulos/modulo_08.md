# Módulo 8 — Descompressão de um bloco em memória

Reconstrói os bytes originais de um bloco a partir do **payload comprimido** e da
**árvore de Huffman**. É o caminho inverso do Módulo 6 e o núcleo que o `cunzip`
(Módulo 11) vai usar.

## O que faz

- **`block_decompress(tree, data, data_size, original_size, &out)`** desce a
  árvore **bit a bit** (esquerda = 0, direita = 1) usando o `BitReader` (Módulo
  5) até alcançar uma folha, emitindo o seu byte, e repete até produzir
  `original_size` bytes.
- O critério de parada é **`original_size`**: os bits de padding do último byte
  são ignorados porque a contagem de bytes já terminou (contrato do Módulo 5).
- Devolve em `*out` um buffer recém-alocado de `original_size` bytes (o chamador
  libera com `free`).

## Por que existe

- **modularizacao.md (Módulo 8):** comprimir bloco → descomprimir bloco →
  comparar com o original. Fecha o **roundtrip** em memória antes do formato `.cz`
  (Módulo 9) e do `cunzip` (Módulo 11).
- Trabalha com peças **independentes do processo** que compactou: a árvore vem da
  desserialização (Módulo 7) e os tamanhos vêm do cabeçalho do bloco (Módulo 9) —
  por isso a assinatura recebe `tree` + `data` + tamanhos, e não um
  `BlockCompressed`.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `include/block.h` | Declara `block_decompress` ao lado de `block_compress`. |
| `src/block.c` | Implementa a descida da árvore bit a bit com o `BitReader`. |
| `tests/test_block_decompress.c` | Roundtrip real (compress→decompress) + folha única, vazio, 256 bytes, bloco grande, truncamento e bordas. |

## Estruturas principais

Não cria struct nova. Lê de um `HuffNode` (Módulo 3) com um `BitReader` (Módulo
5); a saída é um buffer de bytes `malloc`-ado.

## Funções principais

| Função | O que faz |
|--------|-----------|
| `block_decompress(tree, data, data_size, original_size, &out)` | Reconstrói `original_size` bytes em `*out`. `true` em sucesso; `false` em `out` NULL, `tree` NULL com dados, bits insuficientes, árvore inconsistente ou falta de memória (deixando `*out` = NULL). |

### Custo

Cada byte de saída desce no máximo a altura da árvore; **O(n·h)** no pior caso,
**O(total de bits do payload)** na prática — uma passada linear pelos bits.

## Decisões de projeto

- **Parada por `original_size`**, não pelos bits: o payload tem padding no último
  byte (Módulo 5); contar bytes evita gravar a contagem exata de bits.
- **Folha única**: árvore com um só nó → cada byte gasta **1 bit** e emite o
  símbolo da raiz (espelha o código "0" do Módulo 4). Sem esse caso, a descida
  nunca sairia da raiz.
- **Assinatura desacoplada** (`tree` + `data` + tamanhos): casa com o fluxo do
  `cunzip`, onde a árvore e os tamanhos chegam separados do cabeçalho.
- **Robustez**: bits insuficientes ou nó interno sem filho → `false`, libera o
  buffer parcial e zera `*out` (suporte ao bloco corrompido dos Módulos 9/11).

## Como compilar e testar

```sh
# Linux
make test       # roda toda a suite, incluindo test_block_decompress

# Windows (MinGW)
mingw32-make test
```

Saída esperada:

```
=== Testes da descompressao de bloco (Modulo 8) ===
Teste 1: bloco conhecido (ABRACADABRA) roundtrip
Teste 2: folha unica (um unico simbolo repetido)
Teste 3: bloco vazio
Teste 4: todos os 256 valores de byte
Teste 5: bloco maior enviesado (1000 bytes)
Teste 6: payload truncado -> false sem crash
Teste 7: bordas (out NULL, tree NULL com dados)

28 verificacoes, 0 falha(s).
TODOS OS TESTES PASSARAM.
```

## Como explicar na defesa

- **Como decodifica?** Lê bit a bit; 0 vai à esquerda, 1 à direita; ao chegar
  numa folha emite o byte e volta à raiz. Repete até `original_size` bytes.
- **Por que para por `original_size` e não no fim dos bits?** O último byte tem
  padding de zeros que não é dado real; a contagem de bytes é o critério correto.
- **Como prova que está certo?** O teste comprime (Módulo 6) e descomprime
  (Módulo 8) e compara byte a byte (`memcmp`) — roundtrip fechado.
- **Pergunta provável: "e se o payload vier truncado/corrompido?"** → A leitura
  detecta fim de bits ou nó inconsistente e retorna `false` sem crash; o CRC32
  (Módulo 2) e o salto de bloco (Módulos 9/11) tratam a recuperação parcial.

## Decisões de projeto / referências

- Descida da árvore bit a bit reusando o `BitReader` do Módulo 5; parada por
  `original_size`.
- Ver também: `modularizacao.md` (Módulo 8), `RULES.md` (REGRAS 5, 9 e 10),
  `include/huffman.h` (Módulo 3), `include/bitio.h` (Módulo 5),
  `include/tree_serialization.h` (Módulo 7) e a entrada do `DIARIO.md` de
  2026-06-22 — Módulo 8.
