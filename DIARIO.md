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

### 2026-06-22 — Módulo 4: tabela de códigos de Huffman

**Prompt do aluno:**
> Implementar o Módulo 4 com base no estado atual do repositório, usando
> modularizacao.md como especificação principal e seguindo o padrão dos módulos
> anteriores: implementação, testes, documentação técnica, registro no DIARIO.md,
> commit, push e exportação da sessão.

**Resposta da IA (resumo):**
> Implementou a geração da tabela de códigos de Huffman em include/huffman.h e
> src/huffman.c. Adicionou a struct HuffCode { uint64_t bits; uint8_t length; } e
> a função huffman_build_codes(root, table[256]), que percorre a árvore (esquerda
> = bit 0, direita = bit 1) preenchendo o código de cada folha. Os bits são
> guardados alinhados à direita, MSB-first (primeiro bit do caminho na posição
> length-1), para casar com a escrita bit a bit do Módulo 5; length == 0 marca
> símbolo ausente. Tratou o caso especial obrigatório: folha única (bloco com um
> único byte distinto) recebe o código "0" (bits=0, length=1), fechando o caso
> que o Módulo 3 deixara em aberto. A função valida que nenhum código passa de 64
> bits e retorna false nesse caso (e em root/table NULL). Criou
> tests/test_huffman_codes.c cobrindo a modularizacao.md/REGRA 10: byte mais
> frequente com código mais curto, propriedade de prefixo (nenhum código é
> prefixo de outro, testado em todos os pares), folha única, decodificação
> coerente (cada código desce a árvore de volta ao seu símbolo), bordas NULL e
> arquivo com os 256 valores de byte (códigos de 8 bits, todos decodificáveis) —
> 27 verificações, 0 falhas. Atualizou o Makefile (alvo test_huffman_codes
> linkando huffman.c + heap.c, .PHONY e alvo test). Compila limpo com
> gcc -std=c11 -Wall -Wextra -Werror (0 warnings); suíte completa (heap, CRC32,
> árvore, códigos) passa. Criou a doc técnica docs/modulos/modulo_04.md.

**Erros da IA / Correções necessárias:**
> Nenhum erro identificado nesta interação.

**Decisões de projeto tomadas:**
> - REPRESENTAÇÃO DOS CÓDIGOS (lacuna crítica #3): escolhida a opção (a) da
>   modularizacao.md — manter uint64_t E validar o limite de 64 bits. Um código
>   de Huffman só atinge comprimento L se o bloco tiver pelo menos Fibonacci(L+2)
>   bytes; para L=64 isso exigiria um bloco de ~10^13 bytes, inviável com os
>   tamanhos de bloco do projeto. Ainda assim, huffman_build_codes valida e
>   retorna false se algum código passaria de 64 bits, em vez de estourar
>   silenciosamente. A opção (b) (vetor dinâmico de bytes) foi descartada por
>   adicionar alocação/complexidade desnecessária ao caso prático.
> - FOLHA ÚNICA → código "0" (1 bit), fechando o caso especial aberto no Módulo 3
>   (sem isso o código teria comprimento 0 e a leitura não saberia quantos bits
>   consumir).
> - BITS MSB-FIRST alinhados à direita (primeiro bit do caminho em length-1),
>   para que a escrita bit a bit do Módulo 5 emita da posição length-1 até 0.
> - length == 0 marca SÍMBOLO AUSENTE — convenção simples, sem estrutura paralela
>   de presença.
> - huffman.c/heap.c continuam fora de COMMON_SRCS; serão linkados ao czip/cunzip
>   só quando a compressão de bloco (Módulo 6) os usar. Por ora, apenas no teste.

### 2026-06-22 — Módulo 5: escrita e leitura bit a bit

**Prompt do aluno:**
> Implementar o Módulo 5 com base no estado atual do repositório, usando
> modularizacao.md como especificação principal e seguindo o padrão dos módulos
> anteriores: implementação, testes, documentação técnica, registro no DIARIO.md,
> commit e push.

**Resposta da IA (resumo):**
> Implementou a escrita/leitura bit a bit em include/bitio.h, src/bitio.c e
> tests/test_bitio.c. API: BitWriter (bit_writer_init/write_bit/write_bits/flush/
> free) acumulando bits num buffer de bytes que cresce sozinho (realloc dobrando),
> e BitReader (bit_reader_init/read_bit/read_bits) lendo de um buffer existente. O
> empacotamento é MSB-first: o primeiro bit escrito ocupa o bit 7 do primeiro byte,
> casando com a tabela de códigos do Módulo 4 (bits encostados à direita MSB-first)
> — gravar um código é bit_writer_write_bits(w, code.bits, code.length). O flush
> fecha o último byte parcial com padding de ZEROS à direita; documentou o contrato
> de que a parada real da decodificação vem de original_size do bloco (Módulos 8/9),
> não do leitor de bits. O teste cobre a modularizacao.md/REGRA 10: roundtrip de uma
> sequência de 13 bits NÃO alinhada (escreve, flush, lê de volta idêntica + padding),
> empacotamento MSB-first com byte conhecido (1,0,1,1,0 -> 0xB0), bytes cheios sem
> padding, write_bits/read_bits com código de 11 bits, bordas (fim de fluxo -> -1,
> count 0, count > 64, segurança contra NULL) e crescimento do buffer (10000 bits ->
> 1250 bytes, vários reallocs) — 33 verificações, 0 falhas. Atualizou o alvo `test`
> do Makefile (test_bitio linkando bitio.c) e o .PHONY. Compila limpo com
> gcc -std=c11 -Wall -Wextra -Werror (0 warnings); suíte completa (heap, CRC32,
> árvore, códigos, bitio) passa. Criou a doc técnica docs/modulos/modulo_05.md.

**Erros da IA / Correções necessárias:**
> Nenhum erro identificado nesta interação.

**Decisões de projeto tomadas:**
> - ORDEM MSB-FIRST (primeiro bit no bit 7 do byte) para casar exatamente com a
>   representação de HuffCode do Módulo 4 (bits encostados à direita, MSB-first):
>   escrever um código vira emitir da posição length-1 até 0.
> - BUFFER DINÂMICO dobrando a capacidade (custo amortizado O(1) por byte), porque
>   o tamanho comprimido só é conhecido ao final da escrita.
> - PADDING COM ZEROS no flush; o bit I/O NÃO decide onde o conteúdo útil termina.
>   A parada real é por original_size do bloco (Módulos 8/9), evitando gravar a
>   contagem exata de bits (original_size já é obrigatório no cabeçalho — Módulo 9).
> - STRUCTS PÚBLICAS: o chamador precisa de buffer/byte_count após o flush; o
>   BitReader aponta para memória externa (sem buffer próprio, sem função de free).
> - API NULL-safe e com flag de erro (BitWriter.error): falha de alocação vira no-op
>   retornando false, sem corromper o buffer parcial.
> - bitio.c ainda NÃO entra em COMMON_SRCS — só será linkado ao czip/cunzip quando a
>   compressão de bloco (Módulo 6) passar a usá-lo. Por ora, apenas no teste.

### 2026-06-22 — Módulo 6: compressão de um bloco em memória

**Prompt do aluno:**
> Implementar o Módulo 6 com base no estado atual do repositório, usando
> modularizacao.md como especificação principal e seguindo o padrão dos módulos
> anteriores: implementação, testes, documentação técnica, registro no DIARIO.md,
> commit e push.

**Resposta da IA (resumo):**
> Implementou a compressão de um bloco em memória em include/block.h, src/block.c
> e tests/test_block_compress.c. A função block_compress(input, len, out) amarra os
> módulos anteriores no pipeline clássico de Huffman: conta frequências (Mód. 3),
> monta a árvore (Mód. 3), gera a tabela de códigos (Mód. 4) e emite o código de
> cada byte bit a bit com o BitWriter (Mód. 4/5), fechando o último byte parcial
> com flush. O resultado é um BlockCompressed { tree, data, compressed_size,
> original_size } — a struct é dona da árvore e do payload, liberados juntos por
> block_compressed_free(). Tratou os casos de borda: bloco vazio (len 0) retorna
> resultado válido vazio (tree NULL); bloco de um único byte distinto gera folha
> única com 1 bit por byte; falta de memória libera tudo e zera *out. Como o
> descompressor formal só chega no Módulo 8, o teste inclui um decodificador manual
> mínimo que desce a árvore bit a bit até reconstruir original_size bytes, provando
> que o payload é válido. O teste cobre a modularizacao.md/REGRA 10: bloco conhecido
> ("ABRACADABRA") com roundtrip exato, distribuição enviesada que realmente REDUZ o
> tamanho, folha única (50 bytes -> 7 bytes), bloco vazio, todos os 256 valores de
> byte com roundtrip e bordas NULL — 30 verificações, 0 falhas. Atualizou o Makefile
> (alvo test_block_compress linkando block.c + huffman.c + heap.c + bitio.c, .PHONY
> e alvo test). Compila limpo com gcc -std=c11 -Wall -Wextra -Werror (0 warnings);
> suíte completa (heap, CRC32, árvore, códigos, bitio, bloco) passa. Criou a doc
> técnica docs/modulos/modulo_06.md.

**Erros da IA / Correções necessárias:**
> Nenhum erro identificado nesta interação.

**Decisões de projeto tomadas:**
> - DOAÇÃO DO BUFFER (sem cópia): no caminho de sucesso, o ponteiro w.buffer do
>   BitWriter é doado direto para out->data, em vez de copiar para um buffer novo;
>   por isso block.c NÃO chama bit_writer_free no sucesso — a posse passa para o
>   BlockCompressed (liberado por block_compressed_free). Evita copiar o payload
>   inteiro (relevante no teste de fogo de 1 GB). Nos caminhos de erro, o free é
>   chamado normalmente.
> - BlockCompressed é DONO da árvore e do payload; block_compressed_free libera os
>   dois e zera a struct (NULL-safe).
> - BLOCO VAZIO retorna resultado válido vazio (tree NULL, sem payload); a política
>   de arquivo vazio fica para o czip (Módulo 10).
> - FOLHA ÚNICA gera 1 bit por byte (código "0" do Módulo 4); a parada da
>   decodificação é por original_size, ignorando o padding final (contrato Mód. 5).
> - Teste com DECODIFICADOR MANUAL (desce a árvore bit a bit) para validar o
>   roundtrip antes de o Módulo 8 existir.
> - block.c ainda NÃO entra em COMMON_SRCS — só será linkado ao czip/cunzip a
>   partir do Módulo 10. Por ora, apenas no teste.

### 2026-06-22 — Módulo 7: serialização da árvore de Huffman

**Prompt do aluno:**
> Implementar o Módulo 7 com base no estado atual do repositório, usando
> modularizacao.md como especificação principal e seguindo o padrão dos módulos
> anteriores: implementação, testes, documentação técnica, registro no DIARIO.md,
> commit e push.

**Resposta da IA (resumo):**
> Implementou a serialização da árvore de Huffman em include/tree_serialization.h,
> src/tree_serialization.c e tests/test_tree_serialization.c. API:
> tree_serialize(root, &bytes, &size) percorre a árvore em PRÉ-ORDEM escrevendo
> bits com o BitWriter (Módulo 5) — folha = bit 1 + 8 bits do byte; nó interno =
> bit 0 + serialização(esquerda) + serialização(direita) — e doa o buffer ao
> chamador; tree_deserialize(bytes, size) faz a travessia recursiva simétrica com
> o BitReader e devolve a raiz reconstruída. O formato é auto-delimitado: a leitura
> consome exatamente os bits de cada nó, então a recursão para sozinha e os bits de
> padding finais são ignorados. Folha única (um único byte distinto) vira "1 + 8
> bits" = 2 bytes. Serialização truncada/corrompida → tree_deserialize retorna NULL
> sem crash (suporte ao salto de bloco corrompido do Módulo 9/11). O teste cobre a
> modularizacao.md/REGRA 10: roundtrip de árvore comum provado por igualdade
> ESTRUTURAL recursiva E por igualdade da TABELA DE CÓDIGOS gerada pelas duas
> árvores, folha única, todos os 256 valores de byte (tamanho serializado esperado
> de 320 bytes), truncamento → NULL e bordas NULL — 30 verificações, 0 falhas.
> Atualizou o Makefile (alvo test_tree_serialization linkando tree_serialization.c
> + huffman.c + heap.c + bitio.c, .PHONY e alvo test). Compila limpo com
> gcc -std=c11 -Wall -Wextra -Werror (0 warnings); suíte completa (heap, CRC32,
> árvore, códigos, bitio, bloco, serialização) passa. Criou a doc técnica
> docs/modulos/modulo_07.md.

**Erros da IA / Correções necessárias:**
> O teste comparava as tabelas de códigos das duas árvores com memcmp sobre o
> array de HuffCode { uint64_t bits; uint8_t length; }, mas a struct tem bytes de
> PADDING entre 'length' e o fim, com conteúdo indefinido — o memcmp falhava mesmo
> com os valores logicamente iguais. Corrigido com uma comparação campo a campo
> (bits + length). A igualdade estrutural recursiva já passava; o ajuste foi só na
> verificação da tabela.

**Decisões de projeto tomadas:**
> - FORMATO PRÉ-ORDEM (1 = folha + 8 bits do byte; 0 = nó interno + esq + dir),
>   bit a bit, MSB-first, reusando o bit I/O do Módulo 5 — mínimo e auto-delimitado,
>   sem contador de nós nem marcador de fim.
> - TAMANHO EM BYTES no resultado (não em bits): casa com o tree_size do cabeçalho
>   (Módulo 9); os bits de padding finais são inofensivos porque a recursão da
>   leitura para ao completar a árvore.
> - 'frequency' NÃO é restaurado na desserialização (fica 0): a decodificação só
>   precisa de is_leaf, symbol e dos filhos.
> - DOAÇÃO DO BUFFER do BitWriter ao chamador (sem cópia), mesmo padrão do Módulo 6.
> - ROBUSTEZ: fim de fluxo no meio da árvore → tree_deserialize retorna NULL sem
>   crash, preparando o salto de bloco corrompido (Módulos 9/11).
> - tree_serialization.c ainda NÃO entra em COMMON_SRCS — só será linkado ao
>   czip/cunzip a partir do Módulo 9/10. Por ora, apenas no teste.

### 2026-06-22 — Módulo 8: descompressão de um bloco em memória

**Prompt do aluno:**
> Implementar o Módulo 8 com base no estado atual do repositório, usando
> modularizacao.md como especificação principal e seguindo o padrão dos módulos
> anteriores: implementação, testes, documentação técnica, registro no DIARIO.md,
> commit e push.

**Resposta da IA (resumo):**
> Implementou a descompressão de um bloco em include/block.h e src/block.c (caminho
> inverso do Módulo 6). API: block_decompress(tree, data, data_size, original_size,
> &out) desce a árvore de Huffman bit a bit (esquerda = 0, direita = 1) com o
> BitReader (Módulo 5) até alcançar uma folha, emitindo o byte, e repete até
> produzir original_size bytes — o critério de parada é original_size, ignorando os
> bits de padding do último byte (contrato do Módulo 5). Devolve em *out um buffer
> recém-alocado de original_size bytes (chamador libera com free). Tratou os casos
> de borda: bloco vazio (original_size == 0) → sucesso com *out NULL; folha única →
> 1 bit por byte emitindo o símbolo da raiz; out NULL → false; tree NULL com
> original_size > 0 → false; payload truncado / nó interno sem filho → false,
> libera o buffer parcial e zera *out. A assinatura recebe tree + data + tamanhos
> (em vez de um BlockCompressed) porque é assim que o cunzip (Módulo 11) terá os
> dados: árvore vinda da desserialização (Módulo 7) e tamanhos vindos do cabeçalho
> (Módulo 9). Criou tests/test_block_decompress.c fechando o ROUNDTRIP real
> (block_compress → block_decompress → memcmp): bloco conhecido (ABRACADABRA),
> folha única, bloco vazio, todos os 256 valores de byte, bloco grande enviesado
> (1000 bytes), payload truncado → false sem crash e bordas (out/tree NULL) — 28
> verificações, 0 falhas. Atualizou o Makefile (alvo test_block_decompress linkando
> block.c + huffman.c + heap.c + bitio.c, .PHONY e alvo test). Compila limpo com
> gcc -std=c11 -Wall -Wextra -Werror (0 warnings); suíte completa (heap, CRC32,
> árvore, códigos, bitio, compressão de bloco, serialização, descompressão de
> bloco) passa. Criou a doc técnica docs/modulos/modulo_08.md.

**Erros da IA / Correções necessárias:**
> Nenhum erro identificado nesta interação.

**Decisões de projeto tomadas:**
> - PARADA POR original_size (não pelos bits): o payload tem padding de zeros no
>   último byte (Módulo 5); contar bytes evita gravar a contagem exata de bits.
> - FOLHA ÚNICA: árvore de um só nó → cada byte gasta 1 bit e emite o símbolo da
>   raiz (espelha o código "0" do Módulo 4); sem esse caso a descida nunca sairia
>   da raiz.
> - ASSINATURA DESACOPLADA (tree + data + tamanhos, não BlockCompressed): casa com
>   o fluxo do cunzip, onde a árvore (Módulo 7) e os tamanhos (Módulo 9) chegam
>   separados do cabeçalho do bloco.
> - ROBUSTEZ: bits insuficientes ou nó interno sem filho → false, libera o buffer
>   parcial e zera *out, preparando o salto de bloco corrompido (Módulos 9/11).
> - block.c passa a conter compressão (Módulo 6) e descompressão (Módulo 8); ainda
>   NÃO entra em COMMON_SRCS — só será linkado ao czip/cunzip a partir do Módulo
>   10/11. Por ora, apenas nos testes.

### 2026-06-22 — Módulo 9: formato do arquivo .cz

**Prompt do aluno:**
> Implementar o Módulo 9 com base no estado atual do repositório, usando
> modularizacao.md como especificação principal e seguindo o padrão dos módulos
> anteriores: implementação, testes, documentação técnica, registro no DIARIO.md,
> commit e push.

**Resposta da IA (resumo):**
> Implementou o formato do arquivo .cz em include/format.h, src/format.c e
> tests/test_format.c. API de (de)serialização dos cabeçalhos: o cabeçalho
> GLOBAL (CzGlobalHeader: magic "CZHF", version, block_size, block_count — 20
> bytes) com cz_global_header_init/cz_write_global_header/cz_read_global_header
> (a leitura valida magic e versão), e o cabeçalho de BLOCO (CzBlockHeader:
> block_index, original_size, compressed_size, tree_size, original_crc32 — 24
> bytes) com cz_write_block_header/cz_read_block_header. Todos os inteiros são
> gravados em LITTLE-ENDIAN byte a byte (helpers write_u32_le/write_u64_le e
> os pares de leitura), independente do endianess nativo, para portabilidade.
> Implementou também cz_skip_block_payload, que salta tree_size +
> compressed_size bytes (fseek SEEK_CUR) deixando o arquivo no início do próximo
> cabeçalho — é a peça que permite ao cunzip (Módulo 11) PULAR um bloco
> corrompido sem perder a sincronização. O módulo NÃO comprime nem descomprime:
> só lê/escreve cabeçalhos; os bytes da árvore serializada (Módulo 7) e do
> payload comprimido (Módulo 6) são gravados/lidos separadamente pelo
> czip/cunzip usando tree_size e compressed_size. O teste usa tmpfile() (C
> padrão, portável) e cobre a modularizacao.md/REGRA 10: roundtrip do cabeçalho
> global, verificação do LAYOUT little-endian em bytes crus (version=1 → 01 00
> 00 00; valores 0x04030201 e 0x0807060504030201 conferidos byte a byte),
> roundtrip do cabeçalho de bloco, magic inválido → leitura falha, versão
> incompatível → leitura falha, salto de bloco (escreve b0+tree+payload+b1, lê
> b0, salta, lê b1 correto) e bordas (NULL e arquivo truncado) — 51
> verificações, 0 falhas. Atualizou o Makefile (alvo test_format linkando
> format.c, .PHONY e alvo test). Compila limpo com gcc -std=c11 -Wall -Wextra
> -Werror (0 warnings); suíte completa (heap, CRC32, árvore, códigos, bitio,
> compressão, serialização, descompressão, formato) passa. Criou a doc técnica
> docs/modulos/modulo_09.md.

**Erros da IA / Correções necessárias:**
> Nenhum erro identificado nesta interação.

**Decisões de projeto tomadas:**
> - ENDIANESS LITTLE-ENDIAN DOCUMENTADA, gravada BYTE A BYTE: cada uint32/uint64
>   é desmontado/remontado em bytes na ordem fixada, em vez de gravar a
>   representação nativa de memória. Garante portabilidade entre arquiteturas
>   (lacuna menor de endianess apontada na revisão do modularizacao.md).
> - TAMANHOS ANTES DOS DADOS no cabeçalho do bloco (tree_size, compressed_size):
>   premissa de design que habilita cz_skip_block_payload a localizar o início
>   do próximo bloco mesmo com o atual corrompido (RULES REGRA 5/9). O salto não
>   confia no conteúdo do bloco, apenas nos tamanhos.
> - original_crc32 (CRC32 do conteúdo ORIGINAL, Módulo 2) e original_size já
>   reservados no cabeçalho do bloco — sem retrabalho do formato (CRC32 movido
>   para a fundação na revisão do plano).
> - MAGIC "CZHF" + VERSION validados na leitura: rejeita arquivos estranhos e
>   formatos futuros incompatíveis. Magic/extensão .cz são escolha da equipe
>   (não exigência do edital), documentados e defensáveis oralmente.
> - cz_read_block_header retornando false sinaliza FIM da sequência de blocos
>   (sem campo de "fim" explícito); a contagem real está em block_count do
>   cabeçalho global.
> - Teste com tmpfile() (C padrão) em vez de open_memstream (POSIX, ausente no
>   MinGW) — portável Windows/Linux.
> - format.c ainda NÃO entra em COMMON_SRCS — só será linkado ao czip/cunzip a
>   partir dos Módulos 10/11. Por ora, apenas no teste.

### 2026-06-22 — Módulo 10: czip sequencial com CLI

**Prompt do aluno:**
> Implementar o Módulo 10 com base no estado atual do repositório, usando
> modularizacao.md como especificação principal e seguindo o padrão dos módulos
> anteriores: implementação, testes, documentação técnica, registro no DIARIO.md,
> commit e push.

**Resposta da IA (resumo):**
> Implementou o compressor sequencial em src/main_czip.c (antes só esqueleto do
> Módulo 0), fechando o lado da ESCRITA do formato .cz. A CLI (parse_args) aceita
> --threads N e --block-size BYTES em qualquer ordem antes dos posicionais
> <entrada> <saida.cz>; --threads é validado e IGNORADO nesta versão sequencial
> (com aviso), existindo apenas para não retrabalhar o parsing no Módulo 13;
> --block-size define o tamanho do bloco (padrão 64 KiB). O fluxo (compress_file):
> grava o cabeçalho global com block_count provisório 0, lê a entrada em pedaços
> de block_size bytes e, para cada bloco (compress_one_block), calcula o CRC32 do
> conteúdo original (Mód. 2), comprime com block_compress (Mód. 6), serializa a
> árvore com tree_serialize (Mód. 7) e grava cabeçalho do bloco + árvore + payload
> no layout do Módulo 9; ao fim, fseek volta ao offset 0 e reescreve o cabeçalho
> global com a quantidade REAL de blocos. Atualizou o Makefile preenchendo
> COMMON_SRCS (block.c, huffman.c, heap.c, bitio.c, tree_serialization.c, crc32.c,
> format.c), agora linkados ao czip/cunzip. Compila limpo com
> gcc -std=c11 -Wall -Wextra -Werror (0 warnings). Smoke test manual confirmou:
> magic CZHF; arquivo vazio -> 20 bytes (só cabeçalho global, block_count=0);
> arquivo de 1 byte -> 47 bytes (20 + 24 + 2 árvore folha única + 1 payload),
> block_count=1; texto de 243893 bytes -> 148528 bytes em 60 blocos de 4096; o
> campo block_count reescrito conferido byte a byte (0, 1 e 60 em little-endian).
> Suíte unitária completa (Módulos 1-9) segue passando, 0 falhas. Criou a doc
> técnica docs/modulos/modulo_10.md.

**Erros da IA / Correções necessárias:**
> Nenhum erro identificado nesta interação.

**Decisões de projeto tomadas:**
> - BLOCK_COUNT PROVISÓRIO + fseek DE REESCRITA: o cabeçalho global é gravado com
>   block_count=0 e corrigido no fim, em vez de pré-calcular o tamanho do arquivo
>   (nem todo arquivo é stat-ável de forma portável). A saída é regular/seekable,
>   então o fseek ao offset 0 + cz_write_global_header é seguro.
> - CLI com --threads ACEITO E IGNORADO na versão sequencial (parsing pronto para
>   o Módulo 13) e --block-size com padrão 64 KiB; valores validados como inteiros
>   positivos (--block-size também limitado a UINT32_MAX, pois é uint32 no formato).
> - DIVISÃO EM BLOCOS por fread de block_size bytes; o último bloco pode ser menor.
>   Cada bloco carrega árvore + CRC32 próprios (independência -> paralelismo e
>   recuperação parcial nos módulos seguintes).
> - COMMON_SRCS deixa de ser vazio: a partir do Módulo 10 czip/cunzip linkam toda
>   a cadeia de ED. cunzip continua esqueleto até o Módulo 11 (linkar funções não
>   usadas é inofensivo, sem warning).
> - O roundtrip byte a byte (czip -> cunzip -> cmp) fica para o Módulo 11; aqui a
>   validação foi por inspeção do formato gerado (tamanhos exatos e block_count).

### 2026-06-23 — Módulo 11: cunzip sequencial

**Prompt do aluno:**
> Implementar o Módulo 11 com base no estado atual do repositório, usando
> modularizacao.md como especificação principal e seguindo o padrão dos módulos
> anteriores: implementação, testes, documentação técnica, registro no DIARIO.md,
> commit e push.

**Resposta da IA (resumo):**
> Implementou o descompressor sequencial em src/main_cunzip.c (antes só esqueleto
> do Módulo 0), fechando o lado da LEITURA do formato .cz e completando o roundtrip
> byte a byte. A CLI (parse_args) aceita os dois posicionais <entrada.cz> <saida>.
> O fluxo (decompress_file): abre o .cz, lê e VALIDA o cabeçalho global (magic CZHF
> + versão, Mód. 9) e itera os block_count blocos. Para cada bloco
> (decompress_one_block): lê o cabeçalho (Mód. 9), lê a árvore serializada e a
> reconstrói com tree_deserialize (Mód. 7), lê o payload e descomprime com
> block_decompress até original_size bytes (Mód. 8), recalcula o CRC32 do conteúdo
> restaurado (Mód. 2) e compara com o gravado; se válido, grava o bloco na saída.
> RECUPERAÇÃO PARCIAL: a posição do próximo bloco é recalculada do cabeçalho
> (offset do payload + tree_size + compressed_size) e aplicada com fseek a cada
> iteração, então um bloco corrompido (CRC divergente, árvore/payload inválido) é
> detectado, descartado e PULADO sem perder a sincronização — os demais blocos
> continuam. Compila limpo com gcc -std=c11 -Wall -Wextra -Werror (0 warnings).
> Roundtrip czip->cunzip->cmp validado em 7 casos (vazio, 1 byte, símbolo
> repetido, todos os 256 valores de byte, texto, aleatório 200 KB e multi-bloco
> com --block-size 256), todos IDÊNTICOS. Teste de corrupção: arquivo de texto em
> 75 blocos (block-size 2048), 1 byte do payload do meio invertido -> bloco 37
> detectado por CRC32 e descartado, 74 blocos restantes restaurados (saída
> 149952 = 152000 - 2048, exatamente um bloco a menos), exit code 1. Suíte
> unitária completa (Módulos 1-9) segue passando, 0 falhas. Criou a doc técnica
> docs/modulos/modulo_11.md.

**Erros da IA / Correções necessárias:**
> Nenhum erro identificado nesta interação.

**Decisões de projeto tomadas:**
> - RESINCRONIZAÇÃO POR OFFSET DO CABEÇALHO (não pelo conteúdo): a cada bloco
>   grava-se payload_start = ftell e, ao fim do processamento, fseek para
>   payload_start + tree_size + compressed_size. Assim o salto de bloco corrompido
>   (RULES REGRA 5) funciona mesmo sem confiar no conteúdo do bloco — só nos
>   tamanhos do cabeçalho. É o uso prático do que o Módulo 9 preparou.
> - PARADA POR original_size: a descompressão usa original_size como critério de
>   parada (Módulos 5/8), ignorando os bits de padding do último byte.
> - EXIT CODE: 0 quando todos os blocos foram restaurados; 1 em erro fatal
>   (abrir/criar arquivo, magic/versão inválidos, cabeçalho truncado, falha de
>   escrita) OU quando algum bloco foi descartado por corrupção (saída parcial
>   gravada, com aviso no stderr). Distingue "fatal" de "escrita na saída" via um
>   flag write_error, para abortar o arquivo todo só quando a saída falha.
> - CLI MÍNIMA (só os dois posicionais): cunzip não precisa de --threads/--block-size
>   (esses parâmetros estão no .cz / no czip). O pipeline concorrente é dos
>   Módulos 12-14.
> - VALIDAÇÃO DO ROUNDTRIP feita por execução real (czip->cunzip->cmp) nos
>   edge-cases obrigatórios; a suíte formal em script (test_roundtrip) é do
>   Módulo 15 e o teste de corrupção formal é do Módulo 16. cunzip é um main()
>   sem função pura unit-testável isolada, então não foi criado tests/test_cunzip.c
>   (evita refatorar o main em lib — fora do escopo deste módulo).

### 2026-06-23 — Módulo 12: fila bloqueante com mutex e condvars

**Prompt do aluno:**
> Implementar o Módulo 12 com base no estado atual do repositório, usando
> modularizacao.md como especificação principal e seguindo o padrão dos módulos
> anteriores: implementação, testes, documentação técnica, registro no DIARIO.md,
> commit e push.

**Resposta da IA (resumo):**
> Implementou a fila bloqueante limitada (base da concorrência, E3) em
> include/queue.h, src/queue.c e tests/test_queue.c. API: queue_create/
> queue_push/queue_pop/queue_close/queue_destroy (+ queue_size). É um buffer
> CIRCULAR de capacidade fixa guardando ponteiros opacos (void*), protegido por
> um pthread_mutex_t e duas condvars: not_empty (consumidores esperam quando
> vazia) e not_full (produtores esperam quando cheia). queue_push bloqueia
> enquanto cheia e aberta; queue_pop bloqueia enquanto vazia e aberta e retorna
> false quando vazia E fechada (sinal de fim, sem item sentinela); queue_close
> faz broadcast nas duas condvars e a fila ainda DRENA os itens pendentes. As
> esperas usam while (não if) por causa de wakeups espúrios. Guarda head+count
> (não head/tail) para distinguir fila cheia de vazia sem desperdiçar slot. A
> fila NÃO é dona dos itens (queue_destroy não os libera). O teste cobre a
> modularizacao.md/REGRA 10: FIFO single-thread, close que drena e depois
> sinaliza fim, push após close rejeitado, 1 produtor × 4 consumidores com
> 100000 itens (soma consumida == soma 1..N, nada perde/duplica), backpressure
> (fila de capacidade 2 + consumidor lento, soma confere) e bordas (NULL,
> capacity 0). Atualizou o Makefile: variável CONC_TESTS (= test_queue só em
> Linux, vazia no Windows) adicionada ao alvo test, novo alvo test_queue
> (linka queue.c + -pthread) e .PHONY. make test no Windows segue passando e
> PULA test_queue corretamente; toda a suíte unitária (Módulos 1-9) passa, 0
> falhas.

**Erros da IA / Correções necessárias:**
> Nenhum erro identificado. Limitação de ambiente: o MinGW local (GCC 6.3.0,
> modelo de threads win32) NÃO possui libpthread, então src/queue.c e
> tests/test_queue.c não compilam/rodam no Windows. A validação possível no
> Windows foi: (1) make test continua verde e pula test_queue via CONC_TESTS;
> (2) verificação de sintaxe de queue.c e test_queue.c com gcc -std=c11 -Wall
> -Wextra -Werror -fsyntax-only contra um pthread.h de stub (0 warnings),
> confirmando assinaturas e tipos. A validação de execução concorrente e de
> data races (make tsan) será feita em ambiente Linux, como já planejado para
> E3 (Módulos 12-14) desde o Módulo 0.

**Decisões de projeto tomadas:**
> - BUFFER CIRCULAR com head+count (não head/tail): a posição de entrada é
>   (head+count)%capacity, eliminando a ambiguidade "cheia x vazia" que surge
>   quando head==tail.
> - ESPERA EM while (não if): pthread_cond_wait pode acordar espuriamente e
>   vários threads podem competir pelo mutex; a condição (cheia/vazia) é
>   reavaliada ao readquirir o lock.
> - SINAL DE FIM SEM SENTINELA: queue_pop retorna false quando vazia E fechada;
>   o consumidor encerra com while (queue_pop(...)). queue_close usa broadcast
>   (acorda todos), enquanto push/pop usam signal (liberam um parceiro).
> - PUSH APÓS CLOSE REJEITADO (contrato explícito) e DRENAGEM: itens já
>   enfileirados saem antes de pop sinalizar fim.
> - FILA LIMITADA = BACKPRESSURE: capacidade fixa evita consumo ilimitado de
>   memória quando um estágio é mais rápido (relevante no teste de fogo de 1 GB).
> - A FILA NÃO É DONA DOS ITENS: queue_destroy não libera o que ficou
>   enfileirado; a posse dos blocos é do pipeline (Módulos 13/14).
> - PORTABILIDADE: queue só entra no make test em Linux (CONC_TESTS); no Windows
>   é pulada. queue.c ainda NÃO entra em COMMON_SRCS — só será linkado ao czip
>   quando o pipeline (Módulo 13) usá-lo.

### 2026-06-23 — Módulo 13: pipeline concorrente de compressão

**Prompt do aluno:**
> Crie o modulo_13.md, commite e faça push. (O código do pipeline —
> include/pipeline.h e src/pipeline.c — já existia no repositório, não rastreado,
> sem documentação técnica nem entrada no DIARIO.)

**Resposta da IA (resumo):**
> Documentou e versionou o pipeline concorrente (E3). O pipeline liga leitora ->
> raw_q -> N codificadoras -> done_q -> escritora pelas filas bloqueantes do
> Módulo 12. A leitora lê blocos de block_size, numera e enfileira; fecha a
> raw_q no EOF. Cada codificadora calcula CRC32 (M2), comprime com Huffman (M6),
> serializa a árvore (M7) e enfileira o bloco comprimido; a última a sair fecha a
> done_q (contador encoders_active sob mutex). A escritora reordena. O único
> estado compartilhado mutável (contador + flags de erro) é protegido por
> ctx->lock; o FILE* de saída é tocado por uma thread de cada vez, com ordem
> garantida por pthread_create/join (sem mutex global de I/O — RULES REGRA 9).
> Criou docs/modulos/modulo_13.md (seguindo _template.md) e fez commit bbb6e03 +
> push de pipeline.h, pipeline.c e o doc.

**Erros da IA / Correções necessárias:**
> O commit do Módulo 13 (bbb6e03) NÃO atualizou o DIARIO.md (viola RULES REGRA 1).
> Falha registrada e corrigida aqui, junto com a entrada do Módulo 14. Além
> disso, o código do pipeline tinha sido escrito numa sessão anterior já com a
> reordenação embutida, antecipando parte do Módulo 14 — overlap tratado no M14.

**Decisões de projeto tomadas:**
> - O .cz gerado pelo pipeline é byte a byte idêntico ao do czip sequencial
>   (Módulo 10): paralelismo muda só a velocidade, não a saída.
> - Posse de memória explícita seguindo o fluxo: leitora aloca data ->
>   codificadora libera data e aloca tree_bytes/payload -> escritora grava e
>   libera. A fila só carrega ponteiros.
> - Doação do payload (bc.data -> cb->payload, anulando o original) evita copiar
>   os bytes comprimidos entre estágios.

---

### 2026-06-23 — Módulo 14: escritor reordenador + wiring do czip ao pipeline

**Prompt do aluno:**
> Implemente o Módulo 14 com base no estado atual do repositório, usando
> modularizacao.md como especificação principal e seguindo o padrão dos módulos
> anteriores: implementação, testes, documentação técnica, registro no DIARIO.md,
> commit e push. Pare e pergunte se houver dúvida de projeto/conflito.

**Resposta da IA (resumo):**
> Antes de implementar, a IA detectou que a reordenação (núcleo do Módulo 14) já
> existia na writer_thread (src/pipeline.c), escrita junto ao Módulo 13. Parou e
> perguntou o escopo; o aluno aprovou a recomendação: ligar o czip ao pipeline,
> testar e validar a reordenação, documentar. Implementou: (1) main_czip.c agora
> chama pipeline_compress_file sob #ifdef HAVE_PTHREAD (Linux), mantendo o caminho
> sequencial do Módulo 10 no Windows/MinGW — antes o pipeline era código morto
> (nunca linkado nem chamado); os helpers sequenciais (compress_one_block,
> compress_file) ficam sob #ifndef HAVE_PTHREAD para não dispararem
> -Wunused-function no Linux. (2) Makefile: variáveis PTHREAD_DEF
> (= -DHAVE_PTHREAD) e PIPELINE_SRCS (= src/pipeline.c src/queue.c) por
> plataforma; czip linka o pipeline + a fila só em Linux; novo alvo test_pipeline
> em CONC_TESTS. (3) tests/test_pipeline.sh: teste de integração que comprime ~1
> MiB com block-size 4096 (256 blocos) usando 8 e 1 thread e exige .cz IDÊNTICOS
> (reorder determinístico) + roundtrip czip->cunzip byte a byte. (4)
> docs/modulos/modulo_14.md. Validação no Windows: make all e make test seguem
> verdes (0 warnings), roundtrip sequencial OK, o aviso de --threads ignorado
> aparece, e o branch HAVE_PTHREAD do main_czip.c compila limpo via -fsyntax-only.

**Erros da IA / Correções necessárias:**
> Nenhum erro de implementação identificado. Limitação de ambiente: o MinGW local
> (GCC 6.3.0, modelo win32) não tem libpthread, então o pipeline e o
> test_pipeline NÃO compilam/rodam no Windows. A validação obrigatória do módulo
> — ausência de vazamentos (make asan / Valgrind, RULES REGRA 4 −10%) e de data
> races (make tsan, −15%) — deve ser executada em ambiente Linux, como já
> planejado para a Entrega 3 desde o Módulo 0. Os comandos exatos estão em
> docs/modulos/modulo_14.md e no Makefile.

**Decisões de projeto tomadas:**
> - REORDER NA ESCRITORA com next + pending[] (vetor que cresce dobrando), não
>   ordenação dentro da fila: concentra a ordem num só lugar, mantém as filas
>   simples e o caminho quente sem travas.
> - LIBERAÇÃO IMEDIATA de cada bloco após gravado (e do pending[] inteiro no
>   caminho de erro): memória proporcional aos blocos em voo, não ao arquivo —
>   condição para passar no ASan e para o teste de fogo de 1 GB.
> - WIRING POR HAVE_PTHREAD: czip escolhe pipeline (Linux) vs sequencial
>   (Windows) em tempo de compilação; a CLI (--threads/--block-size) é idêntica
>   nos dois, então o relatório de speedup (M17) não depende do build.
> - TESTE DE REORDER POR DETERMINISMO: comprimir o mesmo arquivo com N e 1 thread
>   deve gerar .cz idênticos; se a reordenação falhasse, a versão paralela sairia
>   com blocos trocados. Complementado por roundtrip byte a byte.
> - SEM MUTEX GLOBAL DE I/O (RULES REGRA 9): o arquivo de saída é tocado por uma
>   thread (a escritora) durante o pipeline; ordem com a main garantida por
>   pthread_create/join.

### 2026-06-23 — Módulo 15: suíte de roundtrip e edge-cases

**Prompt do aluno:**
> Implementar o Módulo 15 com base no estado atual do repositório, usando
> modularizacao.md como especificação principal e seguindo o padrão dos módulos
> anteriores: implementação, testes, documentação técnica, registro no DIARIO.md,
> commit e push. Parar e perguntar se houver dúvida de projeto/conflito.

**Resposta da IA (resumo):**
> O prompt pedia "modulo 05", mas o Módulo 5 (bitio) já estava implementado e
> commitado e o repositório estava no Módulo 14; a IA PAROU e confirmou com o
> aluno que o alvo real era o próximo módulo não-feito, o Módulo 15 (suíte de
> roundtrip + edge-cases, E4). Implementou dois scripts shell: (1)
> scripts/gen_edge_cases.sh, que gera os 7 casos obrigatórios da RULES REGRA 10
> (vazio, 1 byte, símbolo repetido, todos os 256 valores de byte, texto, binário
> pequeno e aleatório) usando só printf/head//dev/urandom (portável MSYS2/Linux);
> e (2) tests/test_roundtrip.sh, que comprime e descomprime cada caso com 3
> block-sizes (7, 64, 65536) e compara com o original via cmp (roundtrip byte a
> byte), e ainda valida o caso de CORRUPÇÃO DA ÁRVORE serializada (offset 44) e de
> METADADOS (tree_size, offset 36) exigindo que o cunzip reporte erro e saia SEM
> crash (exit < 128; sinal ≥ 128 = falha). Atualizou o Makefile: novo alvo
> test_roundtrip (depende de all) incluído no make test em Windows E Linux (usa só
> o caminho sequencial, diferente de test_pipeline/test_queue que exigem pthreads),
> e o adicionou ao .PHONY. make test passa inteiro no Windows: toda a suíte
> unitária (Módulos 1-9, 290 verificações) + test_roundtrip (7 casos × 3
> block-sizes + 2 corrupções) verde, 0 warnings com
> gcc -std=c11 -Wall -Wextra -Werror. Criou a doc técnica docs/modulos/modulo_15.md.

**Erros da IA / Correções necessárias:**
> O prompt trazia "modulo 05" (e o placeholder MODULO_ALVO), em conflito com o
> estado do repositório (Módulo 5 já feito, repo no Módulo 14). A IA não assumiu:
> parou e perguntou, e o aluno confirmou o Módulo 15. Nenhum erro de implementação
> identificado depois disso.

**Decisões de projeto tomadas:**
> - SUÍTE EM SHELL (não .c): testa os binários czip/cunzip reais de ponta a ponta,
>   como o usuário os usa; cmp é o critério de igualdade byte a byte. O edital
>   (REGRA 10) pede explicitamente o roundtrip czip→cunzip→comparação.
> - test_roundtrip RODA TAMBÉM NO WINDOWS (caminho sequencial dos Módulos 10/11),
>   ao contrário de test_pipeline/test_queue (pthreads, só Linux) — amplia a
>   cobertura no ambiente de desenvolvimento local.
> - 3 BLOCK-SIZES por caso (7, 64, 65536): o 7, pequeno e não-potência-de-2, força
>   muitos blocos e último bloco parcial; 65536 é o padrão de produção (1 bloco nos
>   casos pequenos). Exercita tanto arquivo de bloco único quanto multi-bloco.
> - CORRUPÇÃO ÁRVORE/METADADOS separada da de PAYLOAD (Módulo 16): caminhos de
>   falha distintos (tree_deserialize→NULL vs CRC32 divergente). "Sem crash"
>   detectado por exit < 128 (≥128 = término por sinal, ex. segfault=139).
> - GERAÇÃO DE CASOS ISOLADA em scripts/gen_edge_cases.sh, reutilizável pelos
>   Módulos 16-17 (corrupção de payload e benchmarks).