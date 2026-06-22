# RULES — Regras Obrigatórias para Toda Interação com IA

> **Projeto**: Compressor de Arquivos com Árvore de Huffman (Tema 11)
> **Disciplinas**: Estrutura de Dados (C) × Sistemas Operacionais
> **Instituição**: Ifes Cachoeiro de Itapemirim
> **Apresentação**: 02/07/2026

---

## 🔴 REGRA 1 — Documentação de Uso de IA (OBRIGATÓRIO A CADA INTERAÇÃO)

**A cada prompt do usuário e resposta gerada, OBRIGATORIAMENTE:**

1. **Atualizar AUTOMATICAMENTE o arquivo `DIARIO.md`** com uma nova entrada contendo:
   ```markdown
   ### [DATA] — [Título breve da interação]

   **Prompt do aluno:**
   > [Transcrição exata ou resumo fiel do que foi pedido]

   **Resposta da IA (resumo):**
   > [Resumo do que foi respondido/gerado]

   **Erros da IA / Correções necessárias:**
   > [O que a IA errou e o que a equipe corrigiu — se nada, escrever "Nenhum erro identificado"]

   **Decisões de projeto tomadas:**
   > [Decisões técnicas relevantes tomadas nesta interação]
   ```

2. **Sempre que uma interação gerar ou modificar código**, a resposta da IA deve incluir uma entrada pronta para colar no DIARIO.md. O aluno deve registrar essa entrada antes de fazer commit.

3. Ao final de cada interação que gere ou modifique código, **lembrar o aluno de fazer commit incremental** com mensagem significativa.

---

## 🔴 REGRA 2 — Linguagem e Compilação

- **Linguagem obrigatória**: C (padrão C11)
- **Compilação obrigatória**: `gcc -std=c11 -Wall -Wextra -Werror`
- **Tolerância zero**: 0 warnings. Qualquer warning = penalidade de −5%.
- Todo código gerado DEVE compilar limpo com as flags acima.

---

## 🔴 REGRA 3 — Commits Incrementais

- **Commits devem ser incrementais** (pequenas mudanças, frequentes).
- **Mensagens de commit significativas**: descrever O QUE foi feito.
- ❌ PROIBIDO: commits massivos tipo "initial commit" com milhares de linhas.
- ❌ PROIBIDO: mensagens genéricas como "update", "changes", "fix".
- ✅ CORRETO: "Implementa heap_insert() e heap_extract_min()", "Adiciona teste unitário do CRC32".
- Ao gerar código, sempre sugerir uma mensagem de commit adequada.

---

## 🔴 REGRA 4 — Arcabouço Técnico

Todo código produzido deve respeitar:

1. **Sem vazamentos de memória** no caminho feliz (penalidade: −10%)
   - Sempre liberar memória alocada com `malloc`/`calloc`
   - Validar com Valgrind/AddressSanitizer

2. **Sem data races** (penalidade: −15%)
   - Código concorrente deve ser livre de races
   - Validar com ThreadSanitizer (TSan)

3. **Sem corrupção de dados** (penalidade: −20%)
   - CRC32 por bloco para detecção de corrupção
   - Bloco corrompido não impede descompressão dos demais

4. **Makefile obrigatório** com alvos:
   - `all` — compila o projeto
   - `test` — roda testes unitários
   - `stress` — roda testes de stress/carga
   - `clean` — limpa artefatos de compilação

---

## 🔴 REGRA 5 — Escopo do Tema 11

O projeto é estritamente o **Tema 11 — Compressor de Arquivos com Árvore de Huffman**:

### Núcleo de Estrutura de Dados:
- Árvore de Huffman construída via **heap binário** (fila de prioridade)
- Geração da **tabela de códigos** por travessia da árvore
- **Serialização compacta** da árvore no cabeçalho de cada bloco
- **Decodificação** descendo a árvore bit a bit

### Núcleo de Sistemas Operacionais:
- **Pipeline de threads**: leitor → N codificadores → escritor reordenador
- **Filas limitadas** com condvars (condition variables)
- **Compressão por blocos independentes** para paralelismo
- Verificação de integridade por **CRC32 por bloco**

### Utilitário final:
- `czip` — comprime arquivos grandes em blocos
- `cunzip` — descomprime mantendo integridade
- `czip` deve permitir configurar **número de threads** e **tamanho do bloco**, para viabilizar medições experimentais e defesa oral

### Formato obrigatório do arquivo `.cz`:

> **Decisão de projeto (não exigência do edital):** o edital não define extensão de arquivo nem
> magic number. A extensão `.cz` e o magic number `CZHF` são **escolhas da equipe**. O que o edital
> de fato exige é que o cabeçalho de cada bloco contenha a árvore de Huffman serializada, para que
> `cunzip` reconstrua a árvore e descomprima sem depender do processo original. Os nomes podem ser
> alterados desde que documentados — devemos saber justificá-los na defesa oral.

O arquivo compactado deve conter metadados suficientes para que `cunzip` consiga reconstruir, validar, pular blocos corrompidos e restaurar os demais blocos sem depender da memória do processo que compactou.

#### Cabeçalho global mínimo:
- Assinatura/magic number do formato, por exemplo `CZHF`
- Versão do formato
- Tamanho de bloco usado na compactação
- Quantidade total de blocos
- Ordem de bytes/endianess documentada

#### Cabeçalho mínimo de cada bloco:
- Índice sequencial do bloco
- Tamanho original do bloco, antes da compressão
- Tamanho do payload comprimido em bytes
- Tamanho da árvore serializada, em bytes ou bits
- CRC32 por bloco, conforme política documentada
- Árvore de Huffman serializada do próprio bloco
- Payload comprimido em bits

#### Regras de decodificação e integridade:
- O formato deve permitir localizar corretamente o início do próximo bloco, mesmo quando o bloco atual estiver corrompido.
- O formato deve permitir parar a decodificação do payload no ponto correto, tratando o último byte parcial e seus bits de preenchimento.
- O `original_size` do bloco deve ser armazenado e usado como critério mínimo para parar a reconstrução dos bytes originais, salvo se o projeto documentar outro mecanismo equivalente, como quantidade real de bits comprimidos.
- A política de CRC32 deve ser documentada antes da implementação. O mínimo obrigatório é validar o CRC32 do conteúdo original restaurado de cada bloco.

### Teste de fogo:
- Arquivo de **1 GB** comprimido com sucesso
- **Demonstrar speedup quase linear** até o número de núcleos disponíveis (meta literal do edital). Esta é a obrigação primária do teste de fogo, não pode ser substituída pela análise de gargalo.
- **Complementarmente**, caso o speedup observado não seja linear, o relatório deve explicar experimentalmente o gargalo: leitura, escrita, reordenação, CRC32 ou custo da compressão.
- Identificar quando o **estágio de escrita serializa** o pipeline
- **Bloco corrompido** manualmente → detectado → demais blocos OK

---

## 🔴 REGRA 6 — Defesa Oral

O código deve ser escrito de forma que o aluno consiga:
- **Explicar cada função** e decisão de projeto
- **Modificar o código ao vivo** (ex: alterar tamanho do bloco, número de threads)
- **Demonstrar entendimento** do mecanismo central

Por isso:
- Código deve ser **bem comentado** em português
- Funções devem ter **nomes descritivos** em snake_case
- Cada módulo deve ter uma **documentação breve** no topo do arquivo .h
- Evitar abstrações excessivamente complexas que o aluno não consiga explicar
- Preferir clareza a elegância

---

## 🔴 REGRA 7 — Relatório Experimental

Toda análise experimental deve:
- Usar **dados reais** coletados pela equipe (não inventados)
- Gerar **gráficos** com scripts reproduzíveis (gnuplot, matplotlib, etc.) versionados no repositório
- Confrontar **teoria × prática** (complexidade assintótica vs. medições)
- Produzir relatório técnico em PDF, preferencialmente entre 8 e 15 páginas, conforme edital
- Cobrir:
  - Speedup vs. threads (1, 2, 4, 8, 16)
  - Taxa de compressão por tipo de arquivo
  - Identificação do gargalo no pipeline
  - Overhead do CRC32

Os experimentos de speedup devem ser executáveis por linha de comando, variando o número de threads e, quando necessário, o tamanho do bloco.

---

## 🔴 REGRA 8 — Estrutura de Resposta

Toda resposta técnica deve seguir esse formato quando envolver implementação, modificação de código, testes, relatório ou decisões de projeto:

1. **Contexto**: Breve resumo do que foi pedido
2. **Decisões de projeto**: Justificativa técnica de escolhas feitas
3. **Código** (se aplicável): Código em C11, compilável, comentado
4. **Testes** (se aplicável): Como testar o que foi implementado
5. **Próximos passos**: O que deve ser feito a seguir no workflow
6. **Commit sugerido**: Mensagem de commit para as alterações
7. **Entrada do DIARIO.md**: Bloco formatado pronto para colar no diário

---

## 🔴 REGRA 9 — Proibições

- ❌ NÃO gerar o projeto inteiro de uma vez
- ❌ NÃO usar bibliotecas externas além da libc e pthreads
- ❌ NÃO deixar informações essenciais apenas em memória.
  No Tema 11, é permitido montar em RAM o heap binário, a árvore de Huffman e a tabela de códigos de cada bloco durante a compressão. Porém, o arquivo `.cz` deve armazenar no cabeçalho de cada bloco a árvore serializada, para que o `cunzip` consiga reconstruir a árvore e descomprimir o arquivo sem depender do processo original.
- ❌ NÃO usar mutex global após a Entrega 2
- ❌ NÃO gerar código sem atualizar o DIARIO.md
- ❌ NÃO fazer commits massivos

---

## 🔴 REGRA 10 — Testes Mínimos Obrigatórios

Além dos testes específicos de cada módulo, o projeto deve conter testes automatizados para:

- Heap binário mínimo: inserção, extração em ordem, empates e heap vazio
- CRC32: buffer vazio, entrada conhecida e alteração de byte
- Escrita/leitura de bits: sequências que não terminam em múltiplos de 8 bits
- Huffman: construção da árvore, geração da tabela de códigos, serialização e desserialização
- Arquivo vazio
- Arquivo com um único byte
- Arquivo com um único símbolo repetido
- Arquivo contendo todos os 256 valores possíveis de byte
- Arquivo de texto
- Arquivo binário pequeno
- Arquivo aleatório
- Roundtrip byte a byte: `czip` seguido de `cunzip` e comparação com o original
- Corrupção manual de bloco/payload: erro detectado e demais blocos preservados
- Corrupção da árvore serializada ou metadados do bloco: erro reportado sem crash
- Execução com 1, 2, 4, 8 e 16 threads, quando disponível no ambiente

---

## 🔴 REGRA 11 — Cronograma

| Entrega | Escopo | Peso |
|---------|--------|------|
| **E1 — Fundação** | Structs, heap, blocos, CRC32, testes unitários | 15% |
| **E2 — Núcleo de ED** | Árvore de Huffman, czip/cunzip single-thread | 40% |
| **E3 — Núcleo de SO** | Pipeline de threads, filas com condvars, reordenação | 10% |
| **E4 — Robustez** | Teste de fogo (1GB), relatório, gráficos | 35% |

**Apresentação final: 02/07/2026**

---

## 🔴 REGRA 12 — Critérios de Avaliação

| Critério | Peso |
|----------|------|
| Corretude funcional (testes + teste de fogo) | 15% |
| Qualidade da árvore de Huffman (correção, serialização, eficiência) | 15% |
| Mecanismos de SO (pipeline, concorrência, CRC32) | 10% |
| Relatório experimental (gráficos, confronto teoria × prática) | 5% |
| **Defesa oral individual** | **50%** |
| Engenharia (Makefile, testes, organização, commits) | 5% |
