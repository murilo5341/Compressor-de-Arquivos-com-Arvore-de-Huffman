# Módulo 9 — Formato do arquivo `.cz`

Define e (de)serializa os **cabeçalhos** do arquivo compactado: o cabeçalho
global do arquivo e o cabeçalho de cada bloco. É a "planta" do `.cz` que o
`czip` (Módulo 10) grava e o `cunzip` (Módulo 11) lê — sem ele os bytes
comprimidos seriam um amontoado sem metadados.

## O que faz

- **Cabeçalho global**: `magic "CZHF"` + `version` + `block_size` +
  `block_count`. Funções `cz_write_global_header` / `cz_read_global_header`
  (esta valida magic e versão).
- **Cabeçalho de bloco**: `block_index`, `original_size`, `compressed_size`,
  `tree_size`, `original_crc32`. Funções `cz_write_block_header` /
  `cz_read_block_header`.
- **Salto de bloco**: `cz_skip_block_payload` avança `tree_size +
  compressed_size` bytes, deixando o arquivo posicionado no próximo cabeçalho —
  é o que permite **pular um bloco corrompido** sem perder a sincronização.
- Todos os inteiros são gravados em **little-endian byte a byte**, portável
  entre arquiteturas (não depende do endianess nativo).

## Por que existe

- **RULES REGRA 5 / REGRA 9 / modularizacao.md (Módulo 9):** o `.cz` deve
  carregar metadados suficientes para que o `cunzip` reconstrua, valide, **pule
  blocos corrompidos** e restaure os demais blocos sem depender do processo que
  compactou. A árvore serializada (Módulo 7) e o `original_crc32` (Módulo 2)
  vivem no cabeçalho de cada bloco.
- O `original_size` é o critério de parada da decodificação (trata o byte
  parcial final — Módulos 5/8); `tree_size` e `compressed_size` localizam o
  início do próximo bloco mesmo com o atual corrompido.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `include/format.h` | Constantes do formato, `CzGlobalHeader`, `CzBlockHeader` e a API de (de)serialização. |
| `src/format.c` | Escrita/leitura little-endian byte a byte; validação de magic/versão; salto de bloco via `fseek`. |
| `tests/test_format.c` | Roundtrip dos cabeçalhos, layout little-endian cru, magic/versão inválidos, salto de bloco e bordas (NULL/truncado). |

## Estruturas principais

```c
typedef struct {
    char     magic[CZ_MAGIC_SIZE];  /* 'C','Z','H','F' (sem '\0') */
    uint32_t version;
    uint32_t block_size;            /* tamanho de bloco da compressao */
    uint64_t block_count;           /* total de blocos */
} CzGlobalHeader;

typedef struct {
    uint64_t block_index;
    uint32_t original_size;     /* criterio de parada da decodificacao */
    uint32_t compressed_size;   /* bytes do payload comprimido */
    uint32_t tree_size;         /* bytes da arvore serializada (Modulo 7) */
    uint32_t original_crc32;    /* CRC32 do conteudo original (Modulo 2) */
} CzBlockHeader;
```

Layout em disco: cabeçalho global (20 bytes) e, por bloco, cabeçalho (24 bytes)
+ `tree_bytes[tree_size]` + `compressed_bytes[compressed_size]`.

## Funções principais

| Função | O que faz |
|--------|-----------|
| `cz_global_header_init(h, block_size, block_count)` | Preenche `h` com magic/versão correntes (conveniência para o czip). |
| `cz_write_global_header(f, h)` / `cz_read_global_header(f, h)` | Grava/lê o cabeçalho global; a leitura **valida** magic e versão. |
| `cz_write_block_header(f, h)` / `cz_read_block_header(f, h)` | Grava/lê o cabeçalho de um bloco (a leitura retorna `false` no fim da sequência). |
| `cz_skip_block_payload(f, h)` | Salta `tree_size + compressed_size` bytes → início do próximo cabeçalho (pula bloco corrompido). |

## Como compilar e testar

```sh
# Linux
make test       # roda toda a suite, incluindo test_format

# Windows (MinGW)
mingw32-make test
```

Saída esperada:

```
=== Testes do formato .cz (Modulo 9) ===
Teste 1: roundtrip do cabecalho global
Teste 2: layout little-endian do cabecalho global
Teste 3: roundtrip do cabecalho de bloco
Teste 4: magic invalido -> leitura falha
Teste 5: versao incompativel -> leitura falha
Teste 6: salto de bloco (cz_skip_block_payload)
Teste 7: bordas (NULL e truncado)

51 verificacoes, 0 falha(s).
TODOS OS TESTES PASSARAM.
```

## Como explicar na defesa

- **Por que magic number e versão?** O magic (`CZHF`) identifica o arquivo como
  um `.cz` e rejeita arquivos estranhos; a versão protege contra ler um formato
  futuro incompatível. Ambos são **escolha da equipe**, não exigência do edital.
- **Por que endianess little-endian byte a byte?** Para o arquivo ser portável:
  gravar a representação nativa de um `uint32` quebraria entre máquinas de
  endianess diferente. Desmontamos cada inteiro em bytes na ordem fixada.
- **Como pular um bloco corrompido?** Pergunta provável → o cabeçalho do bloco
  guarda `tree_size` e `compressed_size`; mesmo sem confiar no conteúdo,
  `cz_skip_block_payload` salta exatamente esses bytes e cai no próximo
  cabeçalho. Por isso os tamanhos ficam **antes** dos dados do bloco.
- **Por que `original_size` no cabeçalho?** É o critério de parada da
  decodificação (os bits de padding do último byte são ignorados — Módulos 5/8).

## Decisões de projeto / referências

- **Endianess little-endian documentada**, gravada byte a byte (portável) em vez
  da representação nativa.
- **`tree_size` e `compressed_size` no cabeçalho** (antes dos dados) habilitam o
  salto de bloco corrompido — premissa de design, não só teste.
- **Leitura valida magic + versão**; `cz_read_block_header` retornando `false`
  sinaliza fim da sequência de blocos (sem campo de "fim" explícito).
- `format.c` ainda **não** entra em `COMMON_SRCS` — só será linkado ao
  `czip`/`cunzip` a partir dos Módulos 10/11. Por ora, apenas no teste.
- Ver também: `modularizacao.md` (Módulo 9), `RULES.md` (REGRAS 5 e 9),
  `include/crc32.h` (Módulo 2), `include/tree_serialization.h` (Módulo 7),
  `include/block.h` (Módulos 6/8) e a entrada do `DIARIO.md` de 2026-06-22 —
  Módulo 9.
