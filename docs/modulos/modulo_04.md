# Módulo 4 — Tabela de códigos de Huffman

Gera, a partir da **árvore de Huffman** (Módulo 3), a **tabela de códigos** que
associa cada byte ao seu código binário. É a ponte entre a árvore e a escrita
bit a bit (Módulo 5): com a tabela em mãos, comprimir um bloco é trocar cada
byte pela sua sequência de bits.

## O que faz

- Percorre a árvore da raiz às folhas atribuindo a cada símbolo um código:
  **esquerda = bit 0, direita = bit 1** (`huffman_build_codes`).
- Guarda cada código como `{bits, length}` numa tabela de 256 posições;
  símbolos ausentes ficam com `length == 0`.
- Trata o **caso especial obrigatório**: bloco com um único byte distinto
  (folha única) recebe o código `0` (1 bit).
- **Valida** que nenhum código passa de 64 bits (limite da representação),
  retornando `false` em vez de estourar silenciosamente.

## Por que existe

- **modularizacao.md (Módulo 4):** o edital exige a *geração da tabela de
  códigos por travessia da árvore*. Sem ela não há como codificar o payload.
- **RULES REGRA 10:** os testes mínimos pedem explicitamente verificar que
  bytes frequentes recebem códigos mais curtos e que vale a **propriedade de
  prefixo** (nenhum código é prefixo de outro).
- **RULES REGRA 5:** o caso de um único símbolo precisa de código de
  comprimento 1 (`0`) — sem isso a (de)codificação não saberia quantos bits ler.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `include/huffman.h` | Declaração de `HuffCode` e `huffman_build_codes` (mesmo .h do Módulo 3). |
| `src/huffman.c` | Travessia recursiva da árvore que preenche a tabela. |
| `tests/test_huffman_codes.c` | Testes: código mais curto p/ frequente, prefixo, folha única, decodificação, bordas, 256 bytes. |

## Estruturas principais

```c
typedef struct {
    uint64_t bits;    // ate 64 bits do codigo, encostados a DIREITA (MSB-first)
    uint8_t  length;  // comprimento em bits; 0 = simbolo ausente
} HuffCode;
```

Os bits ficam **alinhados à direita, MSB-first**: o primeiro bit do caminho
(raiz→folha) está na posição `length - 1` e o último na posição `0`. Assim, ao
escrever (Módulo 5) basta emitir os bits da posição `length - 1` até `0`.
`length == 0` é a forma de marcar um símbolo que não aparece no bloco.

## Funções principais

| Função | O que faz |
|--------|-----------|
| `huffman_build_codes(root, table[256])` | Reinicia a tabela e atribui o código de cada folha percorrendo a árvore. Retorna `false` em `root`/`table` NULL ou se algum código passar de 64 bits. |
| `build_codes_rec` (interna) | Recursão que acumula `bits`/`length` descendo a árvore (0 à esquerda, 1 à direita). |

### Algoritmo (resumo)

1. Zera a tabela (`length = 0` em todas as 256 posições).
2. Se a raiz é uma **folha única**, atribui `{bits = 0, length = 1}` ao byte.
3. Caso contrário, desce recursivamente: a cada passo `bits = (bits << 1) | b`
   e `length++`, com `b = 0` à esquerda e `b = 1` à direita.
4. Ao chegar numa folha, grava `{bits, length}` na posição do símbolo.

Complexidade: **O(n)** no número de nós da árvore (≤ 511 para 256 símbolos);
cada folha é visitada uma vez.

## Decisão de projeto — representação de códigos (lacuna crítica #3)

`HuffCode.bits` é um `uint64_t`, o que **limita o código a 64 bits**. Foi a
opção **(a)** da `modularizacao.md`: manter `uint64_t` **e validar** o limite.

Justificativa: numa árvore de Huffman, um código só atinge comprimento `L` se o
bloco tiver pelo menos `Fibonacci(L + 2)` bytes (a pior distribuição é do tipo
Fibonacci). Para `L = 64` isso exige um único bloco com ~10¹³ bytes (dezenas de
TB), inviável com os tamanhos de bloco usados aqui (dezenas de KB a poucos MB).
Mesmo assim, `huffman_build_codes` **checa** o limite e retorna `false` se algum
código passaria de 64 bits, em vez de truncar silenciosamente — comportamento
defensável e seguro. A alternativa (b), vetor dinâmico de bytes, seria mais
robusta para o caso geral, mas adiciona alocação e complexidade que o limite
prático torna desnecessárias.

## Como compilar e testar

```sh
# Linux (ambiente oficial de validacao)
make test       # roda test_heap, test_crc32, test_huffman_tree e test_huffman_codes
make asan       # (opcional) valida ausencia de vazamentos

# Windows (MinGW)
mingw32-make test
```

Saída esperada (critério de aprovação):

```
=== Testes da tabela de codigos de Huffman (Modulo 4) ===
Teste 1: byte mais frequente recebe codigo mais curto
Teste 2: propriedade de prefixo
Teste 3: folha unica -> codigo de 1 bit (0)
Teste 4: cada codigo decodifica de volta ao seu simbolo
Teste 5: bordas (root/table NULL)
Teste 6: todos os 256 valores de byte recebem codigo

27 verificacoes, 0 falha(s).
TODOS OS TESTES PASSARAM.
```

O teste retorna código `!= 0` se qualquer verificação falhar, fazendo o
`make test` falhar.

## Como explicar na defesa

- **Como o código de cada byte é obtido?** Percorrendo a árvore da raiz até a
  folha do byte: cada passo à esquerda acrescenta um bit `0`, à direita um `1`.
  O caminho inteiro é o código.
- **Por que bytes frequentes têm códigos mais curtos?** Porque na árvore de
  Huffman eles ficam mais perto da raiz (Módulo 3), então o caminho até eles é
  mais curto. É a propriedade que garante compressão.
- **Por que vale a propriedade de prefixo?** Os símbolos só ficam em folhas;
  como o caminho até uma folha nunca passa por outra folha, nenhum código é
  prefixo de outro. Isso é o que torna a decodificação não ambígua.
- **Como os bits são guardados?** Num `uint64_t` encostado à direita, MSB-first:
  o primeiro bit do caminho fica na posição `length - 1`. Ao escrever, emito da
  posição `length - 1` até `0`.
- **E o bloco de um único byte?** A árvore é uma folha única; atribuo o código
  `0` (1 bit). Sem isso o código teria comprimento 0 e a leitura não saberia
  quantos bits consumir.
- **Pergunta provável: "e se um código passar de 64 bits?"** → Na prática não
  passa (precisaria de um bloco de dezenas de TB), mas a função valida e retorna
  `false` em vez de estourar — ver a decisão de representação acima.

## Decisões de projeto / referências

- **Representação `uint64_t` + validação de 64 bits** (opção (a) da
  `modularizacao.md`), registrada no `DIARIO.md`.
- **Folha única → código `0`** (1 bit), fechando o caso aberto no Módulo 3.
- **`length == 0` marca símbolo ausente** — convenção simples que evita uma
  estrutura paralela de "presença".
- **Bits MSB-first alinhados à direita** para casar com a escrita bit a bit do
  Módulo 5.
- Ver também: `modularizacao.md` (Módulo 4), `RULES.md` (REGRAS 5 e 10),
  `include/huffman.h` (Módulo 3) e a entrada do `DIARIO.md` de 2026-06-22 —
  Módulo 4.
