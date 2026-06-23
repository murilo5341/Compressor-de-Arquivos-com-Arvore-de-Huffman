# Módulo 16 — Teste de corrupção de payload

Resumo: script shell que corrompe 1 byte do **payload** de um bloco do meio do
`.cz` e prova que o `cunzip` **detecta** o bloco por CRC32 (sem crash) e **restaura
os demais byte a byte** — a recuperação parcial exigida pelo teste de fogo.

## O que faz

- Comprime um arquivo de texto grande com `--block-size` pequeno (muitos blocos).
- Percorre os cabeçalhos do `.cz` para localizar o payload do bloco do meio.
- Inverte 1 byte do payload (`val XOR 0xFF`) sem alterar o tamanho do arquivo.
- Roda o `cunzip` e exige: exit `≠ 0` e `< 128` (detectou, não crashou).
- Reconstrói o **esperado** = original sem os bytes do bloco corrompido e compara
  com a saída real via `cmp` (prova forte de que só aquele bloco foi perdido).

## Por que existe

Cumpre a parte de **detecção + recuperação parcial** do teste de fogo
(RULES REGRA 5/10) e fecha o requisito "bloco corrompido → detectado → demais OK".
É o complemento do Módulo 15: lá a corrupção é da **árvore/metadados**; aqui é do
**payload** comprimido — caminho de falha distinto, em que o bloco até decodifica,
mas o CRC32 do conteúdo restaurado **diverge** do gravado.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `tests/test_corruption.sh` | Corrompe 1 byte de payload e valida detecção + recuperação parcial. |
| `Makefile` | Alvo `test_corruption` (depende de `all`); incluído no `make test` em Windows e Linux. |

## Como localiza o bloco-alvo

Lê os inteiros little-endian dos cabeçalhos (helper `ru32` via `od`), seguindo o
layout do Módulo 9:

```text
cabecalho global: ... block_size@8 (u32), block_count@12 (u64)
cada bloco (24 B): block_index@+0(u64) original_size@+8 compressed_size@+12
                   tree_size@+16 original_crc32@+20
                   depois tree_bytes[tree_size], depois payload[compressed_size]
```

Percorre bloco a bloco somando `24 + tree_size + compressed_size` para achar o
início de cada payload; corrompe o **primeiro** byte do payload do bloco
`block_count/2` (bits de código reais, não padding) → o CRC32 sempre acusa.

## Detecção vs. recuperação parcial

- **Detecção**: o `cunzip` recalcula o CRC32 do bloco restaurado e compara com o do
  cabeçalho; divergência → bloco descartado, aviso no stderr, exit `1`.
- **Recuperação parcial**: a posição do próximo bloco vem do cabeçalho
  (`tree_size + compressed_size`, Módulo 11), então o salto não depende do conteúdo
  corrompido; os demais blocos saem intactos. O `cmp` contra o esperado
  (original − bloco-alvo) confirma que nenhum outro bloco foi afetado.

## Como compilar e testar

```sh
make all
make test            # inclui test_corruption
make test_corruption # roda só este teste
```

Saída esperada:

```text
test_corruption: OK - bloco 69/139 corrompido no payload detectado por CRC32;
                 demais 138 blocos restaurados byte a byte (exit=1).
```

## Como explicar na defesa

- Por que corromper o **primeiro** byte do payload, não o último? O último byte
  tem padding de zeros (Módulo 5); alterá-lo poderia não mudar o conteúdo decodificado.
  O primeiro byte carrega bits de código reais → garante divergência de CRC32.
- Como o `cunzip` acha o próximo bloco se o atual está corrompido? Pelos tamanhos do
  **cabeçalho** (`tree_size + compressed_size`), nunca pelo conteúdo — é o que o
  Módulo 9 preparou e o Módulo 11 usa.
- Pergunta provável: "E se a corrupção caísse num bit de padding?" → não acusaria
  CRC (conteúdo idêntico); por isso o teste corrompe o início do payload.
- Diferença para o Módulo 15? Árvore/metadados quebram a **decodificação**; payload
  quebra o **conteúdo** (CRC32). Ambos graciosos, mas por mecanismos diferentes.

## Decisões de projeto / referências

- Prova de recuperação parcial por **reconstrução do esperado + `cmp`** (não só
  pelo tamanho da saída): detecta qualquer bloco extra afetado, não apenas a contagem.
- Teste em **shell**, rodando também no Windows (caminho sequencial), como o Módulo 15.
- Ver também: `modularizacao.md` (Módulo 16), `RULES.md` (REGRA 5/10),
  `modulo_15.md` (corrupção de árvore/metadados), `modulo_11.md` (salto de bloco).
