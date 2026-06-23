# Diário de Engenharia — Compressor de Arquivos com Árvore de Huffman

> **Projeto**: Tema 11 — Compressor de Arquivos com Árvore de Huffman
> **Disciplinas**: Estrutura de Dados (C) × Sistemas Operacionais
> **Instituição**: Ifes Cachoeiro de Itapemirim

---

## Semana 1

### 2026-06-21 — Configuração inicial do projeto e criação de workflow/rules

**Prompt do aluno:**
> Com base no arquivo Trabalho_Interdisciplinar_ED_SO_Ifes.pdf, eu preciso que você faça as Rules que devem ser seguidas a cada prompt enviado e resposta gerada. Siga estritamente o que se pede no PDF no que diz respeito a padronização de trabalho, principalmente a documentação sobre o uso de IA.

**Resposta da IA (resumo):**
> A IA leu e extraiu o conteúdo completo do PDF do trabalho interdisciplinar. Identificou o Tema 11 (Compressor de Arquivos com Árvore de Huffman) como o tema do grupo. Criou os seguintes arquivos:
> 1. `RULES.md` — 11 regras obrigatórias derivadas do edital, incluindo: documentação de uso de IA no DIARIO.md a cada interação, commits incrementais, compilação sem warnings, código sem data races/vazamentos, formato de resposta padronizado
> 2. `DIARIO.md` — Inicializado com formato correto conforme exigido pelo edital (seção 3.2)

**Erros da IA / Correções necessárias:**
> Nenhum erro identificado nesta interação (configuração inicial, sem código gerado).

**Decisões de projeto tomadas:**
> - Estrutura do DIARIO.md segue formato semanal com campos: prompt, resposta, erros, decisões
> - Rules são 11 regras derivadas diretamente das seções 2, 3, 4, 5 e 6 do edital

---

### 2026-06-21 — Criacao dos documentos de contexto e implementacao

**Prompt do aluno:**
> Pegue todo o contexto desse diretório, e me mande oq voce interpretou do meu tema. O meu tema é o 11, Compressor de Arquivos Huffman. Ao final, crie um documento dentro da pasta docs, entitulado context.md. Nesse arquivo deve possuir tudo aquilo que voce capturou do que se trata  o tema, qual a proposta da atividade,dificuldade de implementacao, tempo estimado de conclusao utilizando IA e um outro arquivo sobre implementação (Como implementar). Crie e documente de acordo com que foi dito no RULES.md, documente no DIARIO.md a minha entrada de prompt e o restante que é pedido

**Resposta da IA (resumo):**
> A IA analisou o contexto ja levantado do repositorio, seguiu as regras definidas em RULES.md e criou dois documentos dentro de docs/: `context.md`, com a explicacao completa do Tema 11, proposta da atividade, dificuldade, tempo estimado com IA, riscos e estado atual do repositorio; e `implementacao.md`, com um guia incremental de como implementar o compressor Huffman, incluindo heap, Huffman, bit I/O, serializacao, formato .cz, CRC32, czip/cunzip, pipeline com threads, testes e relatorio experimental. Tambem atualizou este DIARIO.md com a entrada da interacao.

**Erros da IA / Correcoes necessarias:**
> Nenhum erro identificado nesta interacao. Foram criados apenas documentos de apoio, sem implementacao de codigo C.

**Decisoes de projeto tomadas:**
> - Separar a documentacao em `docs/context.md` para entendimento do tema e `docs/implementacao.md` para orientar a construcao incremental.
> - Registrar que o repositorio ainda nao possui `src/` nem `tests/`, embora o `Makefile` ja referencie esses diretorios.
> - Recomendar implementacao primeiro single-thread e somente depois evoluir para pipeline concorrente.
> - Documentar um formato inicial possivel para o arquivo `.cz`, com cabecalho global e metadados por bloco.

---

### 2026-06-21 — Reforco das regras de implementacao do Tema 11

**Prompt do aluno:**
> Implemente essas novas mudanças e documente no diario.md

**Resposta da IA (resumo):**
> A IA atualizou o `RULES.md` para tornar explicitos pontos necessarios para implementar corretamente o Tema 11: compilacao com `-std=c11`, serializacao da arvore no cabecalho de cada bloco, formato minimo do arquivo `.cz`, metadados obrigatorios por bloco, tratamento de padding de bits, politica documentada de CRC32, parametros de linha de comando para threads e tamanho de bloco, testes minimos obrigatorios e requisitos de relatorio experimental com scripts versionados. Tambem removeu o video do escopo automatizado das regras, pois ele sera produzido manualmente pela equipe.

**Erros da IA / Correcoes necessarias:**
> Nenhum erro identificado nesta interacao. As mudancas foram apenas em documentacao e regras de trabalho, sem codigo C.

**Decisoes de projeto tomadas:**
> - O arquivo `.cz` deve ter cabecalho global com magic number, versao, tamanho de bloco, quantidade de blocos e endianess documentada.
> - Cada bloco deve armazenar indice, tamanho original, tamanho comprimido, tamanho da arvore serializada, CRC32, arvore serializada e payload comprimido.
> - O tamanho original do bloco deve ser o criterio minimo para parar a decodificacao e tratar bits de preenchimento no ultimo byte.
> - O CRC32 minimo obrigatorio sera o do conteudo original restaurado de cada bloco, salvo decisao posterior documentada.
> - O `czip` deve permitir variar numero de threads e tamanho de bloco para experimentos e defesa oral.
> - A bateria de testes deve cobrir casos limite de Huffman, roundtrip byte a byte, corrupcao de blocos e execucao com diferentes quantidades de threads.

---
### 2026-06-22 — Criacao do arquivo modularizacao.md como base de build

> Quero modularizar cada parte a qual deve ser feito, nao quero commits gigantes, quero que seja implementado aos poucos para que fique o mais didatico e com o menor acoplamento possivel. Quero modularizar no seguinte sentido: 
MODULO 1: Construcao da arvore de Huffman e inicio da implementacao da codificacao 
MOUDULO 2: Estrutura de Descompactacao de arquivos 
... 
E ir dessa forma de acordo com cada etapa. O que eu ti fornecei é apenas um exemplo. Agora que ja temos o context.md nos fornecendo o contexto da aplicacao + as rules que devem ser seguidas. 

**Resposta da IA (resumo):**
> Criacao do modularizacao.md baseado no contexto e nas regras.
---

### 2026-06-22 — Avaliação do modularizacao.md como base de build

**Prompt do aluno:**
> Analisar se o modularizacao.md cobre 100% das lacunas do trabalho.txt,
> usando RULES.md, context.md e implementacao.md como base.

**Resposta da IA (resumo):**
> Cobertura ~85%. Identificadas 5 lacunas críticas: (1) CRC32 posicionado tarde,
> conflitando com E1 e com o fechamento do formato; (2) falta configuração de
> threads/tamanho de bloco via CLI (REGRA 5/7); (3) códigos limitados a 64 bits
> sem ressalva; (4) matriz de testes obrigatórios (REGRA 10) não representada;
> (5) sanitizers (TSan/ASan/Valgrind) ausentes do plano. Mais lacunas menores
> (endianess, folha única, vídeo  E1–E4).

**Erros da IA / Correções necessárias:**
> Nenhum erro identificado nesta interação (análise documental).

**Decisões de projeto tomadas:**
> CRC32 será movido para a fundação; CLI de threads/bloco entra no czip;
> sanitizers entram no Makefile; r documentada.

### 2026-06-22 — Auditoria de escopo de context.md e RULES.md vs. edital

**Prompt do aluno:**
> Analisar context.md e RULES.md, comparar com trabalho.txt e reformular caso estejam fora do escopo do Tema 11.

**Resposta da IA (resumo):**
> Verificou que ambos os arquivos estão dentro do escopo do Tema 11 e fiéis ao edital
> (Huffman via heap, pipeline de threads, CRC32 por bloco, czip/cunzip, teste de fogo de 1 GB,
> pesos das entregas e critérios de avaliação corretos). Apontou lacunas menores: ausência do
> vídeo de até 5 min exigido no pacote final, e recomendou marcar .cz/CZHF como decisão de projeto.

**Erros da IA / Correções necessárias:**
> Nenhum erro identificado. Pendente aprovação do aluno para aplicar os ajustes pontuais.

**Decisões de projeto tomadas:**
> Manter os arquivos como estão no escopo; incluir o vídeo demonstrativo na lista de entregáveis;
> documentar .cz e CZHF como escolhas da equipe, não exigências do edital.

### 2026-06-22 — Revisão do modularizacao.md conforme correções da claudeSession1

**Prompt do aluno:**
> Atualizar modularizacao.md para seguir exatamente as correções do claudeSession1.md
> (lacunas críticas e menores), exceto o registro do vídeo, que será feito manualmente depois.

**Resposta da IA (resumo):**
> Reescreveu o plano: CRC32 movido para a fundação (Mód. 2), formato .cz já com CRC32/endianess
> e salto de bloco corrompido (Mód. 9), CLI de threads/bloco no czip (Mód. 10/13), sanitizers
> asan/tsan no Makefile (Mód. 0/13/14), nova suíte de roundtrip com edge-cases (Mód. 15),
> decisão da representação de códha única e padding explicitados,
> e mapa de entregas E1–E4. Módulos renumerados 0–18.

**Erros da IA / Correções necessárias:**
> Nenhum erro identificado (reestruturação documental).

### 2026-06-22 — Módulo 0: estrutura inicial do projeto e Makefile

**Prompt do aluno:**
> Com base em RULES.md, context.md, implementacao.md e modularizacao.md, começar a
> implementar os módulos. Primeiro o Módulo 0, responsável pela estrutura do projeto.

**Resposta da IA (resumo):**
> Criou a estrutura do Módulo 0: pastas include/, src/, tests/, scripts/ (com .gitkeep nas
> vazias); esqueletos src/main_czip.c e src/main_cunzip.c que validam argumentos e imprimem
> uso; Makefile com os alvos obrigatórios all/test/stress/clean mais asan/tsan/valgrind;
> README.md expandido com estrutura, instruções de build e documentação dos sanitizers/Valgrind.
> Compila limpo com gcc -std=c11 -Wall -Wextra -Werror (0 warnings); make all/clean validados.

**Erros da IA / Correções necessárias:**
> 1) O .gitignore estava ignorando o próprio Makefile (entregável obrigatório); foi reescrito
>    para ignorar apenas artefatos de build (binários, *.o, *.cz, results/, .claude/).
> 2) O MinGW local (GCC 6.3.0, modelo de threads win32) não possui libpthread, então `-pthread`
>    quebrava o build no Windows; o Makefile passou a detectar o SO e só usa -pthread no Linux.

**Decisões de projeto tomadas:**
> - Cabeçalhos (.h) ficam em include/ e fontes (.c) em src/, conforme modularizacao.md.
> - Makefile multiplataforma: detecta Windows_NT para ajustar -pthread, rm/del e extensão .exe.
> - asan/tsan/valgrind existem desde o início, mas só rodam em Linux (MinGW não os suporta);
>   a validação de vazamentos (−10%) e data races (−15%) será feita em ambiente Linux.
> - Binários czip/cunzip iniciam como esqueletos; a lógica entra nos módulos seguintes.

### 2026-06-22 — Módulo 1: heap binário mínimo (fila de prioridade do Huffman)

**Prompt do aluno:**
> Seguir para o Módulo 1, no mesmo formato do Módulo 0: implementar o heap binário
> mínimo (fila de prioridade) usado pela árvore de Huffman, com testes.

**Resposta da IA (resumo):**
> Implementou o min-heap binário em include/heap.h, src/heap.c e tests/test_heap.c.
> API: heap_create/heap_push/heap_pop_min/heap_destroy (+ heap_size/heap_is_empty).
> Ordenação por frequência (min-heap), com desempate determinístico pela ordem de
> inserção. Heap cresce sozinho (realloc dobrando a capacidade) e a API é segura
> contra NULL. O teste cobre os casos da REGRA 10: inserção fora de ordem → saída
> crescente, pop em heap vazio/NULL sem travar e empates em ordem de inserção
> (53 verificações, 0 falhas). Atualizou o alvo `test` do Makefile para compilar e
> rodar test_heap (com prefixo de execução portável Windows/Linux). Compila limpo
> com gcc -std=c11 -Wall -Wextra -Werror (0 warnings).

**Erros da IA / Correções necessárias:**
> Nenhum erro identificado. (O alvo `clean` ainda imprime erro de `del` quando o
> mingw32-make é chamado de dentro do bash, mas é o comportamento já conhecido do
> Módulo 0, ignorado pelo `-` na receita e inofensivo no cmd/PowerShell.)

**Decisões de projeto tomadas:**
> - Desempate por ORDEM DE INSERÇÃO (seq monotônico atribuído no push), e não por
>   "menor byte": é totalmente determinístico/reproduzível e funciona também para
>   nós internos da árvore (que não têm um único byte). Coerente com modularizacao.md.
> - HeapItem guarda `frequency` (chave) e um `payload` opaco (void*) que, a partir
>   do Módulo 3, apontará para o nó da árvore de Huffman — mantém o heap desacoplado.
> - MinHeap é tipo opaco (struct na .c) e cresce dinamicamente; API NULL-safe.
> - heap.c ainda não entra em COMMON_SRCS (só será linkado ao czip/cunzip quando o
>   Huffman, no Módulo 3, passar a usá-lo).

### 2026-06-22 — Módulo 2: CRC32 por bloco (detecção de corrupção)

**Prompt do aluno:**
> Vamos para a próxima implementação do módulo 02, seguindo o mesmo prompt que foi
> dado para a realização do módulo 00 e 01. Siga os passos realizados no claudeSession3.

**Resposta da IA (resumo):**
> Implementou o CRC32 por bloco em include/crc32.h, src/crc32.c e tests/test_crc32.c.
> Antes de codificar, documentou a POLÍTICA de CRC32 (REGRA 5) no topo do .h e neste
> diário. API: crc32_buffer (cálculo de uma vez) e a tríade incremental
> crc32_init/crc32_update/crc32_finalize (para acumular um bloco em pedaços). Algoritmo
> table-driven (tabela de 256 entradas, construída sob demanda na 1ª chamada) com a
> variante CRC-32/ISO-HDLC (polinômio refletido 0xEDB88320, init/xorout 0xFFFFFFFF),
> igual ao zlib/gzip. A API é segura contra NULL. O teste cobre os casos da REGRA 10:
> buffer vazio (== 0), valores conhecidos fixos (CRC32("123456789")==0xCBF43926 e a
> frase do raposo ==0x414FA339), mudança do CRC ao alterar 1 byte, e equivalência entre
> o cálculo de uma vez e o incremental (7 verificações, 0 falhas). Atualizou o alvo
> `test` do Makefile para também compilar e rodar test_crc32 (e adicionou test_crc32 ao
> .PHONY). Compila limpo com gcc -std=c11 -Wall -Wextra -Werror (0 warnings). Criou a
> doc técnica docs/modulos/modulo_02.md.

**Erros da IA / Correções necessárias:**
> Nenhum erro identificado nesta interação.

**Decisões de projeto tomadas:**
> - POLÍTICA DE CRC32 (REGRA 5): calcular sobre o CONTEÚDO ORIGINAL (descomprimido) de
>   cada bloco; a compressão grava o valor no cabeçalho e a descompressão recalcula sobre
>   o bloco restaurado e compara. Coerente com modularizacao.md (Módulo 2).
> - Variante CRC-32/ISO-HDLC (zlib/gzip): poly refletido 0xEDB88320, init/xorout
>   0xFFFFFFFF — padrão de fato, com vetores de teste públicos verificáveis.
> - Implementação table-driven com lazy init da tabela: um lookup por byte, rápido o
>   bastante para o teste de fogo de 1 GB (Módulo 17).
> - API incremental (init/update/finalize) além do atalho crc32_buffer, para permitir
>   acumular o CRC32 de um bloco lido em pedaços.
> - crc32.c ainda não entra em COMMON_SRCS (só será linkado ao czip/cunzip quando o
>   formato .cz, no Módulo 9, passar a gravar/verificar o checksum).

### 2026-06-22 — Módulo 3: construção da árvore de Huffman

**Prompt do aluno:**
> Implementar o Módulo 3 com base no estado atual do repositório, usando
> modularizacao.md como especificação principal e seguindo o padrão dos módulos
> anteriores: implementação, testes, documentação técnica, registro no DIARIO.md,
> commit, push e exportação da sessão.

**Resposta da IA (resumo):**
> Implementou a construção da árvore de Huffman em include/huffman.h, src/huffman.c
> e tests/test_huffman_tree.c. API: huffman_count_frequencies (conta os bytes 0–255
> de um bloco), huffman_build_tree (monta a árvore reaproveitando o min-heap do
> Módulo 1 como fila de prioridade) e huffman_free_tree (libera recursivamente). O
> algoritmo cria uma folha por byte com freq > 0, empurra todas no min-heap e
> combina os dois menores num nó interno (freq = soma) até sobrar a raiz. Tratou o
> caso especial obrigatório: bloco com um único byte distinto gera uma FOLHA ÚNICA
> (raiz é a própria folha, profundidade 0); a atribuição do código "0" fica para o
> Módulo 4. Bloco vazio retorna NULL. O HuffNode é público (não opaco) porque os
> Módulos 4/7/8 percorrem a árvore. O caminho de erro de memória libera tudo que já
> foi alocado (sem vazamentos). O teste cobre os casos da modularizacao.md/REGRA 10:
> contagem correta, árvore válida (folhas = símbolos distintos, nó interno = soma
> dos filhos, total preservado), folha única, bloco vazio → NULL, arquivo com os
> 256 valores de byte e a relação profundidade × frequência (símbolo mais frequente
> não fica mais fundo) — 31 verificações, 0 falhas. Atualizou o alvo `test` do
> Makefile (test_huffman_tree linkando huffman.c + heap.c) e o .PHONY. Compila limpo
> com gcc -std=c11 -Wall -Wextra -Werror (0 warnings); suíte completa (heap, CRC32,
> Huffman) passa. Criou a doc técnica docs/modulos/modulo_03.md.

**Erros da IA / Correções necessárias:**
> Nenhum erro identificado nesta interação.

**Decisões de projeto tomadas:**
> - FOLHA ÚNICA para bloco com um só byte distinto (modularizacao.md, Módulo 3);
>   a atribuição do código "0" (1 bit) fica no Módulo 4 (tabela de códigos).
> - HuffNode PÚBLICO (não opaco): tabela de códigos (Módulo 4), serialização
>   (Módulo 7) e descompressão (Módulo 8) precisam percorrer a árvore.
> - Reaproveitamento do min-heap do Módulo 1 como fila de prioridade — o passo
>   central de Huffman ("os dois menores") é exatamente o que o heap faz em O(log n).
> - Determinismo: folhas empurradas em ordem de byte (0→255) + desempate do heap
>   pela ordem de inserção → mesma distribuição gera sempre a mesma árvore (testes
>   reproduzíveis).
> - Caminho de erro de memória libera todas as subárvores já alocadas (heap_drain_
>   and_free + liberação dos nós já retirados do heap), sem vazamentos.
> - huffman.c/heap.c ainda NÃO entram em COMMON_SRCS — só serão linkados ao
>   czip/cunzip quando a compressão de bloco (Módulo 6+) passar a usá-los; por ora
>   linkados apenas no teste.