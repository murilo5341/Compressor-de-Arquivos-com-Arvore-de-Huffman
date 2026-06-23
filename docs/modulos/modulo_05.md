# Módulo 5 — Escrita e leitura bit a bit

Empacota e desempacota bits reais em bytes. É a ponte entre a **tabela de
códigos** (Módulo 4), que produz códigos de tamanho em *bits*, e o **payload
comprimido** gravado no arquivo, que é uma sequência de *bytes*. Sem isso, cada
bit viraria um caractere `'0'`/`'1'` e não haveria compressão alguma.

## O que faz

- **`BitWriter`**: acumula bits num buffer de bytes que **cresce sozinho**
  (`bit_writer_write_bit`, `bit_writer_write_bits`), e fecha o último byte
  parcial com **padding de zeros** (`bit_writer_flush`).
- **`BitReader`**: lê bits de um buffer existente na mesma ordem
  (`bit_reader_read_bit`, `bit_reader_read_bits`).
- Ordem **MSB-first**: o primeiro bit escrito ocupa o bit 7 (mais significativo)
  do primeiro byte. Isso casa com a tabela de códigos do Módulo 4, que guarda os
  bits encostados à direita MSB-first — escrever um código é
  `bit_writer_write_bits(w, code.bits, code.length)`.

## Por que existe

- **modularizacao.md (Módulo 5):** o edital exige gravar os códigos de Huffman
  como **bits reais**, não como caracteres. O compressor escreve com tamanho em
  bits; o descompressor lê os mesmos bits na mesma ordem.
- **RULES REGRA 10:** os testes mínimos pedem explicitamente uma sequência de
  bits que **não termina em múltiplo de 8**, escrita e lida de volta idêntica.
- **Tratamento do byte parcial (RULES REGRA 5):** o último byte quase sempre tem
  bits de preenchimento; o módulo documenta e implementa esse contrato.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `include/bitio.h` | Declara `BitWriter`, `BitReader` e a API de leitura/escrita. |
| `src/bitio.c` | Empacotamento MSB-first, buffer dinâmico e padding no flush. |
| `tests/test_bitio.c` | Testes: roundtrip não alinhado, ordem MSB-first, flush/padding, write/read_bits, bordas, crescimento do buffer. |

## Estruturas principais

```c
typedef struct {
    uint8_t *buffer;      // bytes ja escritos (cresce com realloc dobrando)
    size_t   capacity;    // capacidade alocada de 'buffer'
    size_t   byte_count;  // bytes COMPLETOS ja gravados
    uint8_t  current;     // byte parcial em construcao (bits nas posicoes altas)
    int      bit_count;   // bits ja ocupados em 'current' (0..7)
    bool     error;       // true se uma alocacao falhou
} BitWriter;

typedef struct {
    const uint8_t *buffer;  // bytes de origem (somente leitura)
    size_t         size;    // total de bytes
    size_t         byte_pos;// byte atual
    int            bit_pos; // proximo bit (0 = bit 7/MSB .. 7 = bit 0/LSB)
} BitReader;
```

Ambas as `struct`s são **públicas**: depois de `bit_writer_flush()` o chamador
precisa de `buffer`/`byte_count` para gravar o payload; o `BitReader` aponta para
memória já existente (por isso não tem buffer próprio nem função de liberação).

## Funções principais

| Função | O que faz |
|--------|-----------|
| `bit_writer_init(w)` | Aloca o buffer inicial. Retorna `false` em NULL/falta de memória; em caso de sucesso liberar com `bit_writer_free`. |
| `bit_writer_write_bit(w, bit)` | Escreve um bit (MSB-first); fecha o byte automaticamente a cada 8 bits. |
| `bit_writer_write_bits(w, bits, count)` | Emite `count` bits (0..64) da posição `count-1` à `0` — forma de gravar um `HuffCode`. |
| `bit_writer_flush(w)` | Fecha o byte parcial completando com **zeros à direita** (padding). No-op se não houver parcial. |
| `bit_writer_free(w)` | Libera o buffer. Seguro com NULL. |
| `bit_reader_init(r, buf, size)` | Aponta o leitor para `buf`. |
| `bit_reader_read_bit(r)` | Lê o próximo bit (0/1) ou `-1` no fim do fluxo. |
| `bit_reader_read_bits(r, count, out)` | Lê `count` bits (0..64) em `*out`; `false` se faltar bit. |

### Algoritmo (resumo)

- **Escrita:** `current |= bit << (7 - bit_count)`, depois `bit_count++`. Quando
  `bit_count == 8`, anexa `current` ao buffer (garantindo capacidade, dobrando
  quando necessário) e reinicia. O `flush` anexa o `current` mesmo incompleto —
  as posições baixas vazias já valem zero (padding).
- **Leitura:** `bit = (buffer[byte_pos] >> (7 - bit_pos)) & 1`, depois avança
  `bit_pos`; ao chegar a 8, passa ao próximo byte. `read_bits` acumula
  `valor = (valor << 1) | bit`, reconstruindo o valor MSB-first.

Complexidade: **O(1) amortizado** por bit (o realloc dobrando dá custo amortizado
constante); leitura é O(1) por bit.

## Decisão de projeto — onde a decodificação para (padding)

Os bits de padding inseridos pelo `flush` **não carregam informação**. O módulo
de bit I/O **não** sabe onde o conteúdo útil termina — quem decide isso é a
descompressão (Módulos 8/9), que decodifica até reconstruir **`original_size`**
bytes do bloco e então ignora os bits de padding finais. Esse contrato está
documentado no topo de `bitio.h`. A alternativa (gravar a contagem exata de bits)
foi dispensada porque `original_size` já é obrigatório no cabeçalho do bloco
(Módulo 9) e basta como critério de parada.

## Como compilar e testar

```sh
# Linux (ambiente oficial de validacao)
make test       # roda test_heap, test_crc32, test_huffman_tree, test_huffman_codes, test_bitio
make asan       # (opcional) valida ausencia de vazamentos

# Windows (MinGW)
mingw32-make test
```

Saída esperada (critério de aprovação):

```
=== Testes da escrita/leitura bit a bit (Modulo 5) ===
Teste 1: roundtrip de sequencia nao alinhada (13 bits)
Teste 2: empacotamento MSB-first (byte conhecido)
Teste 3: bytes cheios e flush sem byte parcial
Teste 4: write_bits / read_bits (codigo de 11 bits)
Teste 5: bordas (fim de fluxo, count 0, NULL)
Teste 6: crescimento do buffer (10000 bits)

33 verificacoes, 0 falha(s).
TODOS OS TESTES PASSARAM.
```

O teste retorna código `!= 0` se qualquer verificação falhar, fazendo o
`make test` falhar.

## Como explicar na defesa

- **Por que bit a bit e não caractere a caractere?** Um código de Huffman tem
  tamanho em bits (ex.: 3 bits). Se gravasse `'0'`/`'1'` como bytes, cada bit
  ocuparia 8 bits — o arquivo *cresceria*. Empacotando 8 bits por byte é que há
  compressão.
- **O que é MSB-first e por que essa ordem?** O primeiro bit escrito vai para o
  bit mais significativo do byte. Escolhi essa ordem para casar com a tabela do
  Módulo 4, que guarda o código MSB-first encostado à direita: escrever é só
  emitir da posição `length-1` até `0`.
- **Como trato o último byte incompleto?** O `flush` completa o byte com zeros à
  direita (padding). Esses bits não significam nada.
- **Então como a leitura sabe onde parar?** Não é o leitor de bits que decide: a
  descompressão para quando reconstrói `original_size` bytes do bloco; os bits de
  padding finais são ignorados.
- **Pergunta provável: "e se faltar memória no meio?"** → O escritor marca
  `error` e as operações viram no-op retornando `false`; o chamador detecta e
  aborta sem corromper nada.

## Decisões de projeto / referências

- **MSB-first** para casar com a representação de `HuffCode` (Módulo 4).
- **Buffer dinâmico dobrando a capacidade** (custo amortizado O(1) por byte),
  porque o tamanho comprimido só é conhecido ao final.
- **Padding com zeros no flush**; a parada real é por `original_size` (Módulo 9),
  não pela contagem de bits.
- **`BitReader` sem buffer próprio** (aponta para memória externa) — sem alocação,
  sem função de liberação.
- Ver também: `modularizacao.md` (Módulo 5), `RULES.md` (REGRAS 5 e 10),
  `include/huffman.h` (Módulo 4) e a entrada do `DIARIO.md` de 2026-06-22 —
  Módulo 5.
