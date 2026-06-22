╭─── Claude Code v2.1.186 ─────────────────────────────────────────────────────╮
│                                                    │ Tips for getting        │
│                Welcome back Murilo!                │ started                 │
│                                                    │ Run /init to create a … │
│                       ▐▛███▜▌                      │ ─────────────────────── │
│                      ▝▜█████▛▘                     │ What's new              │
│                        ▘▘ ▝▝                       │ Added `claude mcp logi… │
│   Opus 4.8 · Claude Pro · bi@tintasmarfim.com's    │ Added status filtering… │
│   Organization                                     │ Added a "Skills" secti… │
│ ~\Desktop\compressor de arquivos\Compressor-de-Ar… │ /release-notes for more │
╰──────────────────────────────────────────────────────────────────────────────╯

❯ Vamos para a proxima implementacao do modulo02, seguindo o mesmo prompt que   
                                                                                
    foi dado para a realizacao do modulo 00 e 01. Siga os passos realizados no  
                                                                                
    claudeSession3.                                                             

● Vou explorar o projeto para entender a estrutura, os módulos anteriores e o
  que foi feito no claudeSession3.

● 2 background agents launched (↓ to manage)
   ├ Explorar estrutura e módulos do projeto
   └ Ler documentação de módulos e sessions

● Lancei dois agentes de exploração em paralelo: um mapeando a estrutura do
  projeto, build e código dos módulos 00/01, e outro lendo a documentação dos
  módulos e os arquivos claudeSession (especialmente o claudeSession3 que você
  mencionou). Aguardando os resultados para montar o plano do módulo 02.

✻ Baked for 18s

● Agent "Explorar estrutura e módulos do projeto" came to rest · 1m 3s

● O primeiro agente terminou com um panorama completo. Aguardando o segundo
  agente (documentação dos módulos e claudeSessions) para entender o fluxo de
  trabalho exato do claudeSession3.

✻ Worked for 7s

● Agent "Ler documentação de módulos e sessions" came to rest · 1m 2s

● Tenho um panorama completo dos dois agentes. Vou ler eu mesmo os arquivos
  críticos para garantir precisão no plano: a spec do Módulo 2, o template de
  doc, o padrão de teste e o formato do DIARIO.

  Read 3 files (ctrl+o to expand)

● Agora vou ler os padrões de estilo (heap.h, test_heap.c), a doc do módulo 01 e
  o final do DIARIO para espelhar exatamente o formato.

  Read 3 files (ctrl+o to expand)

● Tenho tudo o que preciso. A spec do Módulo 2 está totalmente definida em
  modularizacao.md (CRC32 por bloco), o fluxo de trabalho está padronizado nas
  claudeSessions 3 e 4, e os padrões de código/teste/doc estão claros. Não há
  decisões ambíguas que exijam confirmação — a tarefa é replicar o fluxo
  estabelecido. Vou escrever o plano.

● Updated plan
  ⎿  /plan to preview

● O plano está completo. Ele replica exatamente o fluxo dos módulos 0 e 1 para
  implementar o Módulo 2 — CRC32 por bloco.

  Resumo do que será feito:
  - Política CRC32 (documentada antes — REGRA 5): variante padrão
  CRC-32/ISO-HDLC (zlib/gzip), calculada sobre o conteúdo original de cada
  bloco.
  - Código: include/crc32.h, src/crc32.c (table-driven, API NULL-safe com
  cálculo em bloco único e incremental), tests/test_crc32.c (framework caseiro
  igual ao do heap).
  - Testes (REGRA 10): buffer vazio, string conhecida com valor fixo
  (0xCBF43926), sensibilidade a 1 byte, e equivalência incremental.
  - Makefile: adicionar alvo test_crc32 espelhando test_heap.
  - Docs: entrada no DIARIO.md + criar docs/modulos/modulo_02.md.
  - Commit (após seu OK): feat: adiciona CRC32 por bloco com politica 
  documentada.

● User approved Claude's plan
  ⎿  Plan saved to: ~\.claude\plans\vamos-para-a-proxima-ancient-reef.md · /plan
     to edit
     Plano — Implementação do Módulo 02 (CRC32 por bloco)

     Contexto

     O projeto "Compressor de Arquivos com Árvore de Huffman" (Tema 11, Ifes) é
     construído de forma incremental em módulos 0–18, agrupados nas entregas
     E1–E4
     (ver modularizacao.md). Já estão prontos:

     - Módulo 0 — estrutura do projeto, Makefile, esqueletos czip/cunzip
     (claudeSession3).
     - Módulo 1 — min-heap binário com desempate determinístico
     (claudeSession4).

     O próximo passo é o Módulo 2 — CRC32 por bloco, que fecha a Fundação (E1).
     Ele precisa existir antes de fechar o formato .cz (Módulo 9), onde o campo
     original_crc32 já está previsto no cabeçalho do bloco — implementá-lo agora
     evita retrabalho. O CRC32 detecta corrupção: na compressão grava-se o
     checksum
     do bloco original; na descompressão recalcula-se sobre o bloco restaurado e
     compara-se.

     Esta tarefa replica o fluxo de trabalho já usado nos módulos 0 e 1
     (documentado em claudeSession3/claudeSession4), seguindo as 12 regras do
     RULES.md.

     Decisão de política (documentar ANTES de implementar — RULES REGRA 5)

     - Variante: CRC-32/ISO-HDLC (a mesma do zlib/gzip/PNG) — polinômio
     refletido 0xEDB88320, init = 0xFFFFFFFF, xorout = 0xFFFFFFFF.
     É o padrão de fato e dá valores de teste verificáveis publicamente.
     - Escopo: calcular o CRC32 do conteúdo original (descomprimido) de cada
     bloco. Compressão grava no cabeçalho; descompressão recalcula sobre o bloco
     restaurado e compara. (Conforme modularizacao.md linhas 103–113.)
     - Implementação: algoritmo table-driven com tabela de 256 entradas
     (uint32_t). API que permite cálculo incremental (acumular bloco a bloco),
     espelhando o estilo NULL-safe do heap.

     Arquivos a criar

     ┌────────────────────┬─────────────────────────────────────────────────────
     ┐
     │      Arquivo       │                      Conteúdo
     │
     ├────────────────────┼─────────────────────────────────────────────────────
     ┤
     │                    │ Cabeçalho com doc no topo (RULES REGRA 6), guard
     │
     │ include/crc32.h    │ #ifndef CRC32_H, política documentada em
     │
     │                    │ comentário, API pública.
     │
     ├────────────────────┼─────────────────────────────────────────────────────
     ┤
     │                    │ #include "crc32.h" + libc; tabela de 256 entradas;
     │
     │ src/crc32.c        │ implementação table-driven. Estilo do src/heap.c
     │
     │                    │ (comentários em PT, 4 espaços, K&R, snake_case).
     │
     ├────────────────────┼─────────────────────────────────────────────────────
     ┤
     │                    │ Framework caseiro idêntico ao tests/test_heap.c
     │
     │ tests/test_crc32.c │ (contadores static, macro CHECK(cond,msg) com
     │
     │                    │ __LINE__, main chama os testes e retorna
     │
     │                    │ EXIT_SUCCESS/FAILURE).
     │
     └────────────────────┴─────────────────────────────────────────────────────
     ┘

     API pública proposta (crc32.h)

     /* CRC32 de um buffer inteiro de uma vez (caso comum). buf pode ser NULL se
     len==0. */
     uint32_t crc32_buffer(const void *buf, size_t len);

     /* Cálculo incremental, para acumular bloco a bloco:
      *   uint32_t c = crc32_init();
      *   c = crc32_update(c, parte1, n1);
      *   c = crc32_update(c, parte2, n2);
      *   uint32_t final = crc32_finalize(c);
      */
     uint32_t crc32_init(void);
     uint32_t crc32_update(uint32_t crc, const void *buf, size_t len);
     uint32_t crc32_finalize(uint32_t crc);

     crc32_buffer é implementado em termos de init/update/finalize.

     Testes obrigatórios (RULES REGRA 10)

     1. Buffer vazio: crc32_buffer(NULL, 0) == 0x00000000 (e ("", 0) idem).
     2. String conhecida (valor fixo): crc32_buffer("123456789", 9) == 
     0xCBF43926
     (vetor de teste padrão do CRC-32/ISO-HDLC). Adicionar também
     crc32_buffer("The quick brown fox jumps over the lazy dog", 43) == 
     0x414FA339.
     3. Sensibilidade a 1 byte: alterar um único byte muda o CRC32 (comparar
     dois
     buffers que diferem em 1 byte → checksums diferentes).
     4. Equivalência incremental: calcular em uma chamada vs. em pedaços
     (init/update×N/finalize) produz o mesmo valor — garante o uso "por bloco".

     Critério: make test (Linux) / mingw32-make test (Windows) imprime
     N verificacoes, 0 falha(s). e sai com código 0.

     Alterações no Makefile

     Espelhar o padrão do test_heap (C:\...\Makefile linhas 44, 62–66):

     1. Adicionar test_crc32 ao .PHONY (linha 44).
     2. Fazer o alvo test depender também de test_crc32: test: test_heap 
     test_crc32.
     3. Adicionar o alvo:
     test_crc32: tests/test_crc32.c src/crc32.c
         $(CC) $(CFLAGS) tests/test_crc32.c src/crc32.c -o test_crc32$(EXE)
     $(LDFLAGS)
         $(RUN)test_crc32$(EXE)
     4. Atualizar o comentário-índice de testes (linhas 59–61) citando o
     Módulo 2.

     Observação: COMMON_SRCS permanece vazio por ora — crc32.c só será linkado a
     czip/cunzip quando o formato .cz usar o checksum (Módulo 9). Não alterar
     COMMON_SRCS neste módulo.

     Documentação e registro (fluxo padrão dos módulos 0 e 1)

     1. DIARIO.md (raiz) — adicionar entrada na "Semana 1" no formato exato das
     anteriores: **Prompt do aluno:**, **Resposta da IA (resumo):**,
     **Erros da IA / Correções necessárias:**, **Decisões de projeto tomadas:**
     — registrando a política de CRC32 escolhida (REGRA 5) e a implementação.
     2. docs/modulos/modulo_02.md — criar a partir de docs/modulos/_template.md
     (convenção "doc-por-modulo" da auto-memória), preenchendo todas as seções:
     O que faz / Por que existe / Arquivos / Estruturas / Funções / Como
     compilar e
     testar / Como explicar na defesa / Decisões. Usar modulo_01.md como modelo
     de
     profundidade.

     Verificação (end-to-end)

     1. Compilação limpa (RULES REGRA 2):
     gcc -std=c11 -Wall -Wextra -Werror -O2 -Iinclude -c src/crc32.c → 0
     warnings.
     2. Testes: mingw32-make test (Windows) — deve compilar e rodar test_heap
     e test_crc32, ambos com 0 falhas. Os valores de teste (0xCBF43926,
     etc.) validam a corretude contra a referência pública.
     3. Conferir que make clean continua removendo test_crc32 (já coberto pelo
     glob test_* na linha 102).

     Resposta final e commit (após implementar)

     - Resposta no formato da RULES REGRA 8 (Contexto / Decisões / Arquivos /
     Testes / Próximos passos / Commit sugerido / Entrada do DIARIO).
     - Commit em Conventional Commits, apenas após OK do usuário. Sugestão:
     feat: adiciona CRC32 por bloco com politica documentada
     (a modularizacao.md sugere a mensagem; adaptada ao padrão feat: usado no
     histórico).

     Arquivos críticos de referência

     - modularizacao.md (linhas 92–125) — spec do Módulo 2.
     - Makefile (linhas 44, 59–66) — padrão do alvo de teste a espelhar.
     - tests/test_heap.c — framework de teste a replicar.
     - include/heap.h, src/heap.c — estilo de cabeçalho/código.
     - docs/modulos/_template.md, docs/modulos/modulo_01.md — modelo de doc.
     - DIARIO.md — formato das entradas.

● Plano aprovado. Vou começar a implementação. Primeiro, criar a lista de
  tarefas e ler os arquivos completos que vou espelhar (resto do test_heap.c e
  src/heap.c para estilo, e a doc do módulo 01).

  Read 2 files (ctrl+o to expand)

● Tenho todos os padrões. Agora vou criar os três arquivos de código do Módulo
  2.

● Write(include\crc32.h)
  ⎿  Wrote 47 lines to include\crc32.h
       1 /*
       3  *
       4  * Serve para detectar corrupcao dos dados: cada bloco do arquiv
       5  * do seu conteudo ORIGINAL gravado no cabecalho (na compressao)
         . Na
         para-se com o
       7  * valor gravado; se diferirem, o bloco esta corrompido.
       9  * Politica adotada (modularizacao.md, Modulo 2 / RULES REGRA 5)
         :
         do) do bloco;
     … +44 lines (ctrl+o to expand)

● Write(src\crc32.c)
  ⎿  Wrote 71 lines to src\crc32.c
       1 /*
       3  *
       4  * Usa o algoritmo "table-driven": uma tabela de 256 entradas (u
         ma por valor de
          polinomio bit a
       6  * bit. A tabela e construida uma unica vez, na primeira chamada
       7  * polinomio refletido 0xEDB88320 (CRC-32/ISO-HDLC, igual ao zli
         b/gzip).
       8  *
      10  *   - estado inicial 0xFFFFFFFF (crc32_init);
     … +69 lines (ctrl+o to expand)

● Write(tests\test_crc32.c)
  ⎿  Wrote 117 lines to tests\test_crc32.c
       1 /*
          RULES REGRA 10).
       3  *
       4  * Cobre os casos obrigatorios:
       6  *   2) CRC32 de strings conhecidas com valor esperado fixo (vet
         ores publicos);
       8  *   4) equivalencia entre o calculo de uma vez e o incremental 
         (uso por bloco).
       9  *
         ake test` falhar).
     … +120 lines (ctrl+o to expand)

● Agora atualizo o Makefile: .PHONY, alvo test e novo alvo test_crc32.

● Update(Makefile)  ⎿  Added 1 line, removed 1 line
      41  CZIP_SRCS   = src/main_czip.c   $(COMMON_SRCS)
      42  CUNZIP_SRCS = src/main_cunzip.c $(COMMON_SRCS)
      43  
      44 -.PHONY: all test test_heap stress clean asan tsan valgrind help
      44 +.PHONY: all test test_heap test_crc32 stress clean asan tsan va
         +lgrind help                                                    
      45  
      46  # -------------------------------------------------------------
          ---------------
      47  # Compilacao principal

● Update(Makefile)
  ⎿  Added 6 lines, removed 1 line
      58  # Testes unitarios
      59  # Cada modulo adiciona aqui a compilacao e execucao do seu test
          e.
      60  #   Modulo 1 - heap binario (test_heap)
      61 +#   Modulo 2 - CRC32 por bloco (test_crc32)                    
      62  # -------------------------------------------------------------
          ---------------
      62 -test: test_heap                                                
      63 +test: test_heap test_crc32                                     
      64  
      65  test_heap: tests/test_heap.c src/heap.c
      66    $(CC) $(CFLAGS) tests/test_heap.c src/heap.c -o test_heap$(EX
          E) $(LDFLAGS)
      67    $(RUN)test_heap$(EXE)
      68  
      69 +test_crc32: tests/test_crc32.c src/crc32.c                     
      70 +  $(CC) $(CFLAGS) tests/test_crc32.c src/crc32.c -o test_crc32$
         +(EXE) $(LDFLAGS)                                               
      71 +  $(RUN)test_crc32$(EXE)                                       
      72 +                                                               
      73  # -------------------------------------------------------------
          ---------------
      74  # Teste de stress / carga (sera implementado no Modulo 17 - tes
          te de fogo)
      75  # -------------------------------------------------------------
          ---------------

● Agora valido: compilação limpa do crc32.c e execução dos testes.

  Ran 2 shell commands (ctrl+o to expand)

● Ambos passam, e os valores 0xCBF43926/0x414FA339 confirmam a corretude contra
  a referência pública. Agora vou criar a documentação. Deixe-me ler o
  modulo_01.md completo para espelhar a profundidade.

  Read 1 file (ctrl+o to expand)

● Agora crio a documentação do Módulo 2 espelhando essa profundidade.

● Write(docs\modulos\modulo_02.md)
  ⎿  Wrote 124 lines to docs\modulos\modulo_02.md
       1 # Módulo 2 — CRC32 por bloco
       3 Checksum **CRC32** usado para detectar corrupção dos dados bloco
          a bloco. Na
       4 compressão grava-se o CRC32 do conteúdo **original** de cada blo
       5 na descompressão recalcula-se sobre o bloco restaurado e compara
         -se. Fecha a
       7 
       8 ## O que faz
       9 
         p/PNG) com
     … +165 lines (ctrl+o to expand)

● Agora a entrada no DIARIO.md. Vou ler o final do arquivo para inserir após a
  última entrada (Módulo 1).

  Read 1 file (ctrl+o to expand)

● Vou adicionar a entrada do Módulo 2 ao final do DIARIO.

● Update(DIARIO.md)
  ⎿  Added 38 lines, removed 1 line
      189  >   do Módulo 3, apontará para o nó da árvore de Huffman — man
           tém o heap desacoplado.
      190  > - MinHeap é tipo opaco (struct na .c) e cresce dinamicamente
           ; API NULL-safe.
      191  > - heap.c ainda não entra em COMMON_SRCS (só será linkado ao 
           czip/cunzip quando o
      192 ->   Huffman, no Módulo 3, passar a usá-lo).                   
      192   No newline at end of file
      193 +>   Huffman, no Módulo 3, passar a usá-lo).                   
      194 +                                                              
      195 +### 2026-06-22 — Módulo 2: CRC32 por bloco (detecção de corrup
          +ção)                                                          
      196 +                                                              
      197 +**Prompt do aluno:**                                          
      198 +> Vamos para a próxima implementação do módulo 02, seguindo o 
          +mesmo prompt que foi                                          
      199 +> dado para a realização do módulo 00 e 01. Siga os passos rea
          +lizados no claudeSession3.                                    
      200 +                                                              
      201 +**Resposta da IA (resumo):**                                  
      202 +> Implementou o CRC32 por bloco em include/crc32.h, src/crc32.
          +c e tests/test_crc32.c.                                       
      203 +> Antes de codificar, documentou a POLÍTICA de CRC32 (REGRA 5)
          + no topo do .h e neste                                        
      204 +> diário. API: crc32_buffer (cálculo de uma vez) e a tríade in
          +cremental                                                     
      205 +> crc32_init/crc32_update/crc32_finalize (para acumular um blo
          +co em pedaços). Algoritmo                                     
      206 +> table-driven (tabela de 256 entradas, construída sob demanda
          + na 1ª chamada) com a                                         
      207 +> variante CRC-32/ISO-HDLC (polinômio refletido 0xEDB88320, in
          +it/xorout 0xFFFFFFFF),                                        
      208 +> igual ao zlib/gzip. A API é segura contra NULL. O teste cobr
          +e os casos da REGRA 10:                                       
      209 +> buffer vazio (== 0), valores conhecidos fixos (CRC32("123456
          +789")==0xCBF43926 e a                                         
      210 +> frase do raposo ==0x414FA339), mudança do CRC ao alterar 1 b
          +yte, e equivalência entre                                     
      211 +> o cálculo de uma vez e o incremental (7 verificações, 0 falh
          +as). Atualizou o alvo                                         
      212 +> `test` do Makefile para também compilar e rodar test_crc32 (
          +e adicionou test_crc32 ao                                     
      213 +> .PHONY). Compila limpo com gcc -std=c11 -Wall -Wextra -Werro
          +r (0 warnings). Criou a                                       
      214 +> doc técnica docs/modulos/modulo_02.md.                      
      215 +                                                              
      216 +**Erros da IA / Correções necessárias:**                      
      217 +> Nenhum erro identificado nesta interação.                   
      218 +                                                              
      219 +**Decisões de projeto tomadas:**                              
      220 +> - POLÍTICA DE CRC32 (REGRA 5): calcular sobre o CONTEÚDO ORI
          +GINAL (descomprimido) de                                      
      221 +>   cada bloco; a compressão grava o valor no cabeçalho e a de
          +scompressão recalcula sobre                                   
      222 +>   o bloco restaurado e compara. Coerente com modularizacao.m
          +d (Módulo 2).                                                 
      223 +> - Variante CRC-32/ISO-HDLC (zlib/gzip): poly refletido 0xEDB
          +88320, init/xorout                                            
      224 +>   0xFFFFFFFF — padrão de fato, com vetores de teste públicos
          + verificáveis.                                                
      225 +> - Implementação table-driven com lazy init da tabela: um loo
          +kup por byte, rápido o                                        
      226 +>   bastante para o teste de fogo de 1 GB (Módulo 17).        
      227 +> - API incremental (init/update/finalize) além do atalho crc3
          +2_buffer, para permitir                                       
      228 +>   acumular o CRC32 de um bloco lido em pedaços.             
      229 +> - crc32.c ainda não entra em COMMON_SRCS (só será linkado ao
          + czip/cunzip quando o                                         
      230 +>   formato .cz, no Módulo 9, passar a gravar/verificar o chec
          +ksum).                                                        
      231   No newline at end of file

● Por fim, atualizo o índice de memória (convenção doc-por-módulo já existe; não
  precisa novo arquivo) e limpo os binários de teste gerados na raiz.

  Ran 1 shell command (ctrl+o to expand)

● Módulo 2 implementado e validado. Resposta no formato da RULES REGRA 8:

  Contexto

  Implementação do Módulo 2 — CRC32 por bloco, fechando a Fundação (E1),
  seguindo o mesmo fluxo dos Módulos 0 e 1 (claudeSession3/4). O CRC32 detecta
  corrupção: grava-se o checksum do bloco original na compressão e recalcula-se
  na descompressão para comparar.

  Decisões

  - Política de CRC32 (REGRA 5, documentada ANTES de codificar), no topo de
  crc32.h e no DIARIO: cálculo sobre o conteúdo original (descomprimido) de cada
  bloco.
  - Variante CRC-32/ISO-HDLC (zlib/gzip/PNG): polinômio refletido 0xEDB88320,
  init/xorout 0xFFFFFFFF — padrão com vetores de teste públicos.
  - Table-driven com tabela de 256 entradas em lazy init (rápido para o teste de
  fogo de 1 GB).
  - API incremental (crc32_init/update/finalize) além do atalho crc32_buffer,
  para acumular um bloco lido em pedaços. Toda a API é NULL-safe.
  - crc32.c ainda não entra em COMMON_SRCS — só será linkado ao czip/cunzip no
  Módulo 9.

  Arquivos

  - include/crc32.h — API + política documentada (novo)
  - src/crc32.c — implementação table-driven (novo)
  - tests/test_crc32.c — testes da REGRA 10 (novo)
  - Makefile — alvo test_crc32 + .PHONY (modificado)
  - docs/modulos/modulo_02.md — doc técnica para a defesa (novo)
  - DIARIO.md — entrada do Módulo 2 (modificado)

  Testes

  mingw32-make test — 0 warnings (-Werror), heap (53 verificações) e crc32 (7
  verificações) com 0 falhas. Corretude confirmada contra vetores públicos:
  CRC32("123456789") == 0xCBF43926 e a frase do raposo == 0x414FA339. Cobre:
  buffer vazio, valores conhecidos fixos, sensibilidade a 1 byte e equivalência
  one-shot vs. incremental.

  Próximos passos

  Módulo 3 — Construção da Árvore de Huffman (início da E2), que passará a usar
  o heap do Módulo 1.

  Commit sugerido

  Após seu OK (Conventional Commits, padrão do histórico):
  feat: adiciona CRC32 por bloco com politica documentada

  Entrada do DIARIO

  Adicionada a entrada 2026-06-22 — Módulo 2: CRC32 por bloco com prompt,
  resumo, erros (nenhum) e decisões (política de CRC32, variante, table-driven,
  API incremental).

  Quer que eu faça o commit?

✻ Crunched for 11m 47s

❯ commita e faça o git push para o repositorio                                  

● Vou registrar o módulo no MEMORY.md (índice) não é necessário — a convenção já
  existe. Faço o commit e o push.

  Committed a7e1874, pushed to main, ran 1 shell command (ctrl+o to expand)

● Pronto. Commit e push concluídos com sucesso.

  - Commit: a7e1874 — feat: adiciona CRC32 por bloco com politica documentada
  - Push: 460e5a8..a7e1874 main -> main enviado para
  github.com/murilo5341/Compressor-de-Arquivos-com-Arvore-de-Huffman
  - 6 arquivos: 3 novos (crc32.h, crc32.c, test_crc32.c + doc), Makefile e
  DIARIO atualizados.

  O Módulo 2 (CRC32 por bloco) está versionado e publicado. Próximo passo do
  plano é o Módulo 3 — Construção da Árvore de Huffman (início da E2).

✻ Cooked for 41s

※ recap: Building a Huffman file compressor in C, module by module. Just finished Module 2 (per-block CRC32) with tests passing, and committed and pushed it. Next up is Module 3, building the Huffman tree.