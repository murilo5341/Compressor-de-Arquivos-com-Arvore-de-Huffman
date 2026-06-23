# Módulo 14 — Escritor reordenador

Fecha o **núcleo de Sistemas Operacionais (E3)**. Garante que os blocos
comprimidos pelas N codificadoras do pipeline ([Módulo 13](modulo_13.md)) —
que terminam **fora de ordem** — sejam gravados no `.cz` na **ordem correta**, e
**liga o `czip` ao pipeline** para que isso rode de verdade.

## O que faz

- **Reordena a saída:** a thread escritora mantém `next` (próximo índice
  esperado) e um vetor `pending[]` indexado pelo índice do bloco; guarda blocos
  adiantados e, ao chegar `next`, grava-o e todos os consecutivos já presentes.
- **Memória limitada:** libera cada bloco logo após gravá-lo — nunca acumula o
  arquivo inteiro em RAM (importante no teste de fogo de 1 GB).
- **Saída determinística:** o `.cz` gerado com **N threads é byte a byte igual**
  ao gerado com **1 thread** — o paralelismo muda só a velocidade, não os bytes.
- **Wiring:** `main_czip.c` passa a chamar `pipeline_compress_file` em Linux
  (`HAVE_PTHREAD`), mantendo o caminho sequencial ([Módulo 10](modulo_10.md)) no
  Windows/MinGW sem libpthread. Antes deste módulo o pipeline era código morto.

## Por que existe

- **modularizacao.md (Módulo 14):** as codificadoras terminam em tempos
  diferentes; sem reordenador, os blocos sairiam embaralhados e o `cunzip`
  ([Módulo 11](modulo_11.md)) não reconstruiria o original.
- **RULES REGRA 5 / context.md:** o edital exige "escritor reordenador" que
  mantenha a ordem mesmo com compressão paralela.
- **RULES REGRA 4 (−10% por leak):** a posse de memória dos blocos termina aqui;
  o módulo exige validação com **AddressSanitizer/Valgrind** sem vazamentos.
- **RULES REGRA 9:** sem mutex global de I/O — o arquivo é tocado por uma thread
  de cada vez (ordem garantida por `pthread_create`/`join`).

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `src/pipeline.c` | `writer_thread` — reordenador (`next` + `pending[]`). |
| `src/main_czip.c` | Liga o `czip` ao pipeline em Linux (`#ifdef HAVE_PTHREAD`). |
| `Makefile` | `-DHAVE_PTHREAD`, linka `pipeline.c`/`queue.c` ao czip, alvo `test_pipeline`. |
| `tests/test_pipeline.sh` | Roundtrip + reorder determinístico (8 vs 1 thread). |

## Estruturas principais

O reordenador não cria tipo novo: usa o `CompressedBlock` (M13) e dois estados
locais da escritora.

```c
/* Estado local da writer_thread (em src/pipeline.c). */
CompressedBlock **pending;     /* vetor indexado pelo índice do bloco       */
size_t            pending_cap; /* capacidade do vetor (cresce dobrando)     */
uint64_t          next;        /* próximo índice a gravar, em ordem         */
uint64_t          count;       /* blocos efetivamente gravados              */
```

`pending[index]` guarda blocos que chegaram **antes** da sua vez. O vetor cresce
dobrando conforme aparecem índices maiores, então não há limite fixo de quantos
blocos podem estar adiantados.

## Funções principais

| Função | O que faz |
|--------|-----------|
| `writer_thread(arg)` | Tira blocos da `done_q`, guarda em `pending[]`, grava em ordem a partir de `next` e libera cada um após gravar. |
| `write_block(out, cb)` | Grava cabeçalho + árvore + payload de 1 bloco (layout do M9). |
| `pipeline_compress_file(...)` | Orquestra leitora/codificadoras/escritora; chamada pelo `main_czip.c` em Linux. |

Trecho central do reordenador (grava todos os consecutivos disponíveis):

```c
pending[cb->index] = cb;
while (next < pending_cap && pending[next] != NULL) {
    CompressedBlock *w = pending[next];
    if (!write_block(p->out, w)) failed = true;
    free_compressed(w);          /* libera logo após gravar */
    pending[next] = NULL;
    next++; count++;
}
```

## Como compilar e testar

```sh
# Linux (pipeline ligado ao czip)
make                 # czip ja usa o pipeline concorrente
make test            # inclui test_pipeline (reorder deterministico + roundtrip)
make test_pipeline   # so o teste do Modulo 14

# Validacao obrigatoria do modulo (RULES REGRA 4):
make asan && ./czip --threads 8 --block-size 4096 entrada.bin saida.cz   # sem leak
make tsan && ./czip --threads 8 --block-size 4096 entrada.bin saida.cz   # sem race
valgrind --leak-check=full --error-exitcode=1 ./czip --threads 8 entrada.bin saida.cz

# Windows (MinGW sem libpthread): czip usa o caminho SEQUENCIAL (M10);
# test_pipeline NAO entra (CONC_TESTS vazio).
mingw32-make all
```

Critério esperado: `test_pipeline` confirma que o `.cz` com 8 threads é
**idêntico** ao com 1 thread (`cmp` zerado → reorder correto) e que o roundtrip
`czip → cunzip` reconstrói o original byte a byte. Sob ASan/Valgrind, zero
vazamentos; sob TSan, zero data races.

## Como explicar na defesa

- **Por que a escritora reordena em vez de ordenar a fila?** As codificadoras
  são independentes e terminam fora de ordem; ordenar dentro da fila exigiria
  espera/sincronização extra. Concentrar a ordem num só lugar (a escritora)
  mantém as filas simples e o caminho quente sem travas.
- **Como funciona o `pending[]`?** É indexado pelo índice global do bloco. Bloco
  adiantado fica guardado; só grava quando `next` chega nele, daí drena todos os
  consecutivos. Ex.: esperado=3, chega 5 (guarda), chega 4 (guarda), chega 3 →
  grava 3, 4, 5 em sequência.
- **Por que a saída é idêntica ao sequencial?** A ordem dos bytes depende só do
  índice do bloco, não de qual thread terminou primeiro nem do escalonador.
- **Onde a memória é liberada?** Logo após `write_block`, cada `CompressedBlock`
  é liberado (`free_compressed`). No caminho de erro, os blocos órfãos em
  `pending[]` também são liberados antes de a thread sair — daí passar no ASan.
- **Pergunta provável:** "e se uma codificadora falhar e um índice no meio nunca
  chegar?" → a escritora marca `failed`, para de gravar, mas continua **drenando
  e liberando** o que chega (não trava o pipeline); ao final libera o `pending[]`
  inteiro e a `main` aborta sem `.cz` válido.

## Decisões de projeto / referências

- **Reorder na escritora** com `next` + `pending[]` (vetor que cresce dobrando),
  não estrutura ordenada na fila — mais simples de explicar e sem trava no
  caminho quente.
- **Liberação imediata** de cada bloco após gravado: memória proporcional ao
  número de blocos *em voo*, não ao tamanho do arquivo.
- **Wiring por `HAVE_PTHREAD`:** o `czip` escolhe pipeline (Linux) vs sequencial
  (Windows) em tempo de compilação; a CLI (`--threads`/`--block-size`) é a mesma
  nos dois, então o relatório de speedup (M17) não depende do ambiente de build.
- **Reorder já fora escrito junto ao M13:** a `writer_thread` nasceu com a
  reordenação; o Módulo 14 a **liga ao `czip`**, **testa** (reorder determinístico
  + roundtrip) e **valida** memória (ASan/Valgrind), além de documentá-la.
- **Portabilidade / validação:** sanitizers e Valgrind rodam em Linux; o MinGW
  local (GCC 6.3.0, modelo win32) não tem libpthread. No Windows confirma-se que
  o caminho sequencial e a suíte unitária seguem passando e que ambos os branches
  do `main_czip.c` compilam limpos (`-Werror`, 0 warnings).
- Ver também: `modularizacao.md` (Módulo 14), `RULES.md` (REGRAS 4, 5 e 9),
  `modulo_13.md` (pipeline), `modulo_12.md` (fila) e a entrada do `DIARIO.md` de
  2026-06-23 — Módulo 14.
