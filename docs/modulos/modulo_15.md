# Módulo 15 — Suíte de roundtrip e edge-cases

Resumo: dois scripts shell que provam, de ponta a ponta, que `czip`→`cunzip`
reconstrói **byte a byte** todos os arquivos de borda obrigatórios (RULES REGRA 10)
e que a corrupção da **árvore serializada / metadados** é reportada sem crash.

## O que faz

- `scripts/gen_edge_cases.sh <dir>` gera os 7 casos obrigatórios: vazio, 1 byte,
  símbolo repetido, todos os 256 valores de byte, texto, binário pequeno e aleatório.
- `tests/test_roundtrip.sh` comprime e descomprime cada caso com **3 block-sizes**
  (7, 64, 65536) e compara com o original via `cmp` (roundtrip byte a byte).
- Valida o caso de **corrupção da árvore/metadados** do bloco: altera 1 byte na
  região da árvore e, em outra rodada, no campo `tree_size`, exigindo que o `cunzip`
  reporte erro e saia **sem crash** (exit de erro normal, nunca sinal ≥ 128).

## Por que existe

Cumpre a **RULES REGRA 10** (testes mínimos obrigatórios) e parte da REGRA 4
(sem corrupção de dados). É a primeira verificação automatizada de ponta a ponta do
par `czip`/`cunzip`: os módulos anteriores testavam unidades isoladas; aqui se prova
o sistema inteiro nos casos de borda que mais quebram compressores (arquivo vazio,
folha única, alfabeto completo, dados incompressíveis). A corrupção de
**árvore/metadados** é deliberadamente separada da corrupção de **payload** (Módulo 16).

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `scripts/gen_edge_cases.sh` | Gera os 7 arquivos de borda num diretório de saída. |
| `tests/test_roundtrip.sh` | Roundtrip byte a byte dos 7 casos × 3 block-sizes + corrupção árvore/metadados. |
| `Makefile` | Alvo `test_roundtrip` (depende de `all`); incluído no `make test` em Windows e Linux. |

## Casos cobertos (RULES REGRA 10)

| Caso | Arquivo gerado | O que exercita |
|------|----------------|----------------|
| Arquivo vazio | `empty` | 0 blocos; só cabeçalho global. |
| Um único byte | `single_byte` | bloco mínimo. |
| Símbolo repetido | `repeated_symbol` | **folha única** (código "0", 1 bit/byte). |
| Todos os 256 valores | `all_256` | alfabeto completo, códigos de 8 bits. |
| Texto | `text` | distribuição enviesada → boa compressão. |
| Binário pequeno | `binary_small` | NULs, bytes de controle, alto bit. |
| Aleatório | `random` | pior caso (incompressível). |
| Corrupção árvore/metadados | (gerado inline) | erro reportado **sem crash** (≠ payload, Módulo 16). |

## Layout do `.cz` usado na corrupção

```text
[0..19]  cabecalho global (magic[4] + version + block_size + block_count)
[20..43] cabecalho do 1o bloco (24 bytes): ... offset 36 = tree_size
[44..]   arvore serializada do 1o bloco, depois o payload
```

A corrupção em `off 44` cai na **árvore**; em `off 36` cai em **metadados**
(`tree_size`). Usa-se arquivo de símbolo único (árvore = folha única, 2 bytes) para
garantir que a alteração não caia no payload.

## Como compilar e testar

```sh
# Linux e Windows (MSYS2)
make all
make test           # inclui test_roundtrip
make test_roundtrip # roda só esta suite

# inspecionar os casos gerados
sh scripts/gen_edge_cases.sh /tmp/casos && ls -l /tmp/casos
```

Saída esperada:

```text
test_roundtrip: OK - roundtrip byte a byte dos 7 casos x 3 block-sizes.
test_roundtrip: OK - corrupcao de arvore/metadados reportada sem crash.
test_roundtrip: TODOS OS CASOS PASSARAM.
```

## Como explicar na defesa

- Por que **3 block-sizes**? `7` (pequeno, não-potência-de-2) força muitos blocos e
  último bloco parcial; `65536` é o padrão de produção (1 bloco nos casos pequenos).
- Por que separar **árvore/metadados** (M15) de **payload** (M16)? São caminhos de
  falha distintos: árvore inválida → `tree_deserialize` retorna NULL; payload
  corrompido → CRC32 do bloco restaurado diverge. Ambos devem ser graciosos.
- Como detectar "sem crash" em shell? Exit code `≥ 128` indica término por **sinal**
  (segfault = 139); o teste exige exit de erro normal (`1`), nunca sinal.
- Pergunta provável: "E se o arquivo for vazio?" → 0 blocos, `cunzip` recria arquivo
  vazio; `cmp` confirma 0 bytes.

## Decisões de projeto / referências

- Suíte em **shell** (não `.c`): testa os binários reais ponta a ponta, como o
  usuário os usa; `cmp` é o critério byte a byte. Registrada no `DIARIO.md`.
- `test_roundtrip` roda **também no Windows** (usa só o caminho sequencial), ao
  contrário de `test_pipeline`/`test_queue` (exigem pthreads, só Linux).
- Geração de casos isolada em `scripts/gen_edge_cases.sh`, reutilizável por outros
  testes/benchmarks (Módulos 16–17).
- Ver também: `modularizacao.md` (Módulo 15), `RULES.md` (REGRA 10), `modulo_16.md`
  (corrupção de payload + recuperação parcial).
