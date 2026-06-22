# Módulo 2 — CRC32 por bloco

Checksum **CRC32** usado para detectar corrupção dos dados bloco a bloco. Na
compressão grava-se o CRC32 do conteúdo **original** de cada bloco no cabeçalho;
na descompressão recalcula-se sobre o bloco restaurado e compara-se. Fecha a
Fundação (E1) antes de o formato `.cz` (Módulo 9) depender dele.

## O que faz

- Implementa o **CRC-32/ISO-HDLC** (a mesma variante de zlib/gzip/PNG) com
  algoritmo **table-driven** (tabela de 256 entradas, um byte por iteração).
- Oferece cálculo **de uma vez** (`crc32_buffer`) e **incremental**
  (`crc32_init` → `crc32_update` → `crc32_finalize`) para acumular um bloco em
  vários pedaços.
- API **segura contra `NULL`** (buffer `NULL` com `len 0` é um no-op).
- Vem com teste unitário cobrindo os casos obrigatórios da **RULES REGRA 10**.

## Por que existe

- **modularizacao.md (Módulo 2) / RULES REGRA 5:** o CRC32 é a defesa contra
  corrupção exigida pelo projeto. Foi movido para a Fundação porque o campo
  `original_crc32` já está previsto no cabeçalho do bloco (Módulo 9) — tê-lo
  pronto agora evita retrabalho do formato.
- **RULES REGRA 10:** exige testes de CRC32 para buffer vazio, valor conhecido
  fixo e sensibilidade à alteração de um byte — todos cobertos aqui.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `include/crc32.h` | API pública e documentação da política de CRC32 adotada. |
| `src/crc32.c` | Implementação table-driven (montagem da tabela + init/update/finalize). |
| `tests/test_crc32.c` | Testes unitários (vazio, valores conhecidos, sensibilidade a 1 byte, equivalência incremental). |

## Estruturas principais

Este módulo não expõe `struct`s: o estado do cálculo é um simples `uint32_t` que
circula entre `crc32_init`/`crc32_update`/`crc32_finalize`. Internamente há
apenas a tabela de consulta, montada uma única vez sob demanda:

```c
// src/crc32.c — detalhes internos
#define CRC32_POLY    0xEDB88320u  // polinomio refletido (LSB-first)
#define CRC32_INITIAL 0xFFFFFFFFu  // estado inicial e XOR final

static uint32_t crc32_table[256];      // tabela de consulta (1 entrada por byte)
static bool     crc32_table_pronta;    // construida na 1a chamada (lazy init)
```

Cada entrada `crc32_table[i]` é o CRC do byte `i` processado bit a bit com o
polinômio refletido. Com a tabela pronta, `crc32_update` processa **um byte por
iteração**: `crc = (crc >> 8) ^ tabela[(crc ^ byte) & 0xFF]`.

## Funções principais

| Função | O que faz |
|--------|-----------|
| `crc32_buffer(buf, len)` | Atalho: calcula o CRC32 finalizado de um buffer inteiro de uma vez. |
| `crc32_init()` | Retorna o estado inicial (`0xFFFFFFFF`) para começar um cálculo incremental. |
| `crc32_update(crc, buf, len)` | Acumula `len` bytes no estado `crc`; no-op se `buf == NULL` ou `len == 0`. |
| `crc32_finalize(crc)` | Aplica o XOR final (`0xFFFFFFFF`) e devolve o CRC32 definitivo. |
| `crc32_montar_tabela()` (interna) | Constrói `crc32_table` a partir do polinômio refletido (chamada uma vez). |

## Como compilar e testar

```sh
# Linux (ambiente oficial de validacao)
make test       # compila e roda test_heap e test_crc32
make asan       # (opcional) roda os testes/binarios com AddressSanitizer

# Windows (MinGW, em C:\MinGW\bin)
mingw32-make test
```

Saída esperada (critério de aprovação):

```
=== Testes do CRC32 por bloco (Modulo 2) ===
Teste 1: CRC32 de buffer vazio
Teste 2: CRC32 de strings conhecidas (valores fixos)
Teste 3: CRC32 muda ao alterar um byte
Teste 4: calculo de uma vez equivale ao incremental

7 verificacoes, 0 falha(s).
TODOS OS TESTES PASSARAM.
```

A corretude é verificada contra vetores **públicos** do CRC-32:
`CRC32("123456789") == 0xCBF43926` e a frase do raposo `== 0x414FA339`. O teste
retorna código `!= 0` se qualquer verificação falhar, fazendo o `make test` falhar.

## Como explicar na defesa

- **O que é o CRC32 e para que serve aqui?** É um checksum de 32 bits do conteúdo
  do bloco. Guardamos o CRC do bloco original; na descompressão recalculamos e
  comparamos — se diferir, o bloco corrompeu (detecção de corrupção exigida).
- **Por que table-driven?** Em vez de processar 8 bits por byte, a tabela
  pré-calcula o resultado de cada byte, deixando o laço em **um lookup por byte** —
  bem mais rápido, ideal para arquivos grandes (teste de fogo, Módulo 17).
- **Por que essa variante (0xEDB88320, init/xorout 0xFFFFFFFF)?** É o CRC-32 do
  zlib/gzip/PNG, padrão de fato e com vetores de teste públicos, o que torna a
  implementação verificável.
- **Por que init e xorout = 0xFFFFFFFF?** O init evita que sequências de zeros à
  esquerda passem despercebidas; o XOR final é convenção da variante para casar
  com os valores de referência.
- **Por que a API incremental?** Um bloco pode ser processado em pedaços
  (streaming/leitura por partes); `init`/`update`/`finalize` acumulam o mesmo
  resultado que o cálculo de uma vez (testado no Teste 4).
- **Pergunta provável: "CRC32 garante segurança/integridade contra ataque?"** →
  Não; detecta corrupção acidental (ruído, bug, disco), não adulteração
  maliciosa — para isso seria preciso um hash criptográfico.

## Decisões de projeto / referências

- **Política (RULES REGRA 5):** CRC32 calculado sobre o **conteúdo original
  (descomprimido)** do bloco; gravado na compressão, recalculado e comparado na
  descompressão. Registrada no `DIARIO.md` e no topo de `include/crc32.h`.
- **Variante CRC-32/ISO-HDLC** (poly refletido `0xEDB88320`, init/xorout
  `0xFFFFFFFF`), table-driven com **lazy init** da tabela.
- `crc32.c` ainda **não** entra em `COMMON_SRCS` do Makefile — será linkado ao
  `czip`/`cunzip` quando o formato `.cz` (Módulo 9) passar a gravar/verificar o
  checksum.
- Ver também: `modularizacao.md` (Módulo 2), `RULES.md` (REGRAS 5 e 10), entrada
  do `DIARIO.md` de 2026-06-22 — Módulo 2.
