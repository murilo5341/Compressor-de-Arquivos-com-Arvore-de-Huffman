# Módulo 11 — `cunzip` sequencial

Segundo **executável** do projeto: o descompressor `cunzip`. Fecha o lado da
**leitura** do formato `.cz`, completando o roundtrip byte a byte
(`czip` → `cunzip` → `cmp`) — ainda **sem threads** (o pipeline concorrente é
dos Módulos 12–14).

## O que faz

- **CLI** (`parse_args`): aceita os dois posicionais `<entrada.cz> <saida>`.
- **Leitura e validação do cabeçalho global** (`cz_read_global_header`, Módulo 9):
  rejeita arquivos que não comecem com o magic `CZHF` ou de versão incompatível.
- **Descompressão por blocos** (`decompress_file`): itera os `block_count` blocos
  declarados no cabeçalho global.
- **Um bloco** (`decompress_one_block`): lê a árvore serializada e a reconstrói
  (`tree_deserialize`, Módulo 7) → lê o payload e descomprime até `original_size`
  bytes (`block_decompress`, Módulo 8) → recalcula o CRC32 do conteúdo restaurado
  (Módulo 2) e compara com o gravado → grava o bloco se válido.
- **Recuperação parcial**: bloco corrompido (CRC divergente, árvore/payload
  inválido) é **detectado, descartado e pulado**; os demais continuam.

## Por que existe

- **modularizacao.md (Módulo 11):** o utilitário `cunzip` é entregável
  obrigatório e prova que o `.cz` carrega metadados suficientes para reconstruir
  o original **sem depender do processo que compactou** (RULES REGRA 9).
- **RULES REGRA 5 (integridade / teste de fogo):** valida o CRC32 por bloco e
  garante que um bloco corrompido não impede a descompressão dos demais.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `src/main_cunzip.c` | CLI, leitura/validação do cabeçalho global, laço por blocos, reconstrução da árvore, descompressão, verificação de CRC32 e escrita da saída. |

## Estruturas principais

```c
typedef struct {
    const char *input_path;   /* arquivo .cz de entrada */
    const char *output_path;  /* arquivo restaurado de saida */
} CunzipOptions;
```

Reaproveita `CzGlobalHeader` e `CzBlockHeader` (Módulo 9) para ler os cabeçalhos.

## Funções principais

| Função | O que faz |
|--------|-----------|
| `parse_args(argc, argv, opt)` | Lê os dois posicionais `<entrada.cz> <saida>`. |
| `decompress_file(opt)` | Abre arquivos, valida o cabeçalho global e itera os blocos. |
| `decompress_one_block(in, bh, out, write_error)` | Lê árvore + payload, descomprime, valida CRC32 e grava o bloco; retorna `false` se corrompido. |
| `read_exact(f, buf, n)` | Lê exatamente `n` bytes (detecta leitura curta / truncamento). |

## Como compilar e testar

```sh
# Linux
make all
./czip   entrada.txt saida.cz
./cunzip saida.cz   restaurado.txt
cmp entrada.txt restaurado.txt        # roundtrip byte a byte

# Windows (MinGW)
mingw32-make all
./cunzip.exe saida.cz restaurado.txt
```

Critério esperado: o roundtrip `czip` → `cunzip` → `cmp` é idêntico para arquivo
vazio, 1 byte, símbolo repetido, todos os 256 valores de byte, texto, dados
aleatórios e arquivos multi-bloco. Corrompendo 1 byte de um payload, o bloco
correspondente é detectado por CRC32 e descartado (aviso no stderr, exit ≠ 0),
e os demais blocos são restaurados — a saída fica menor exatamente pelo tamanho
do bloco perdido.

## Como explicar na defesa

- **Como o `cunzip` sabe parar de decodificar cada bloco?** Pelo `original_size`
  do cabeçalho (Módulos 5/8): conta bytes emitidos e ignora os bits de padding do
  último byte. Não depende de "contar bits".
- **Como um bloco corrompido não derruba os demais?** A posição do próximo bloco
  é recalculada do cabeçalho: `offset_do_payload + tree_size + compressed_size`.
  Mesmo sem confiar no conteúdo do bloco, sabemos onde ele termina (`fseek`),
  então pulamos e seguimos. A integridade vem do CRC32 do conteúdo restaurado.
- **Por que validar CRC do conteúdo ORIGINAL e não do comprimido?** Decisão do
  Módulo 2: valida o resultado final, pegando inclusive erros de decodificação,
  não só corrupção de bytes no disco.
- **Pergunta provável:** "e se o `.cz` não for um `.cz`?" → `cz_read_global_header`
  valida magic `CZHF` e versão; arquivo estranho falha com mensagem clara.

## Decisões de projeto / referências

- **Resincronização por offset do cabeçalho** (não por leitura do conteúdo):
  garante o salto de bloco corrompido sem perder a sincronização (RULES REGRA 5).
- **Exit code:** `0` quando todos os blocos foram restaurados; `1` em erro fatal
  (abrir/criar arquivo, magic/versão, truncamento, falha de escrita) **ou** quando
  algum bloco foi descartado por corrupção (saída parcial gravada, com aviso).
- **`cunzip` permanece single-thread**: o pipeline concorrente é dos Módulos
  12–14; a suíte formal de roundtrip e os testes de corrupção são dos Módulos
  15–16.
- Ver também: `modularizacao.md` (Módulo 11), `RULES.md` (REGRAS 5 e 9),
  `include/format.h` (Módulo 9), `include/block.h` (Módulo 8),
  `include/tree_serialization.h` (Módulo 7), `include/crc32.h` (Módulo 2) e a
  entrada do `DIARIO.md` de 2026-06-23 — Módulo 11.
