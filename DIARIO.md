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