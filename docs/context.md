# Contexto do Projeto - Tema 11

## Identificacao

- Projeto: Compressor de Arquivos com Arvore de Huffman
- Tema: 11
- Disciplinas integradas: Estrutura de Dados em C e Sistemas Operacionais
- Instituicao: Ifes Cachoeiro de Itapemirim
- Linguagem obrigatoria: C, padrao C11
- Compilacao obrigatoria: `gcc -Wall -Wextra -Werror`
- Utilitarios finais esperados: `czip` e `cunzip`

## O que foi entendido do tema

O tema consiste em construir um compressor e descompressor de arquivos baseado em Arvore de Huffman. O programa deve ler arquivos grandes, dividir o conteudo em blocos independentes, comprimir cada bloco usando codificacao de Huffman e gravar o resultado em um arquivo compactado proprio, com extensao sugerida `.cz`.

Na descompressao, o utilitario deve ler o arquivo `.cz`, reconstruir a arvore de Huffman de cada bloco a partir do cabecalho gravado no arquivo e restaurar o conteudo original bit a bit. O projeto tambem precisa detectar corrupcao por bloco usando CRC32, sem impedir que os demais blocos validos sejam descomprimidos.

O trabalho nao e apenas um compressor simples. Ele exige a integracao entre uma estrutura de dados nao trivial, a Arvore de Huffman construida com heap binario, e mecanismos reais de Sistemas Operacionais, como threads, filas bloqueantes, condition variables, pipeline concorrente e reordenacao da saida.

## Proposta da atividade

A proposta da atividade e desenvolver, em C, um sistema de media a alta complexidade que demonstre dominio pratico de Estrutura de Dados e Sistemas Operacionais. O projeto deve ser construido de forma incremental, com commits pequenos, testes automatizados, documentacao das decisoes de projeto e registro do uso de IA no `DIARIO.md`.

Para o Tema 11, a entrega esperada e um par de programas:

- `czip`: compacta arquivos grandes em blocos usando Huffman.
- `cunzip`: descompacta arquivos `.cz`, valida os blocos e restaura o arquivo original.

O sistema precisa manter a ordem correta dos blocos mesmo quando a compressao paralela terminar fora de ordem. Isso significa que o escritor precisa funcionar como reordenador, aguardando ou armazenando temporariamente blocos ate que possam ser gravados na sequencia correta.

## Parte de Estrutura de Dados

O nucleo de Estrutura de Dados e a Arvore de Huffman. Para cada bloco do arquivo de entrada, o programa deve:

1. Contar a frequencia de cada byte, considerando os 256 valores possiveis.
2. Criar um heap binario minimo com os simbolos que aparecem no bloco.
3. Remover repetidamente os dois nos de menor frequencia.
4. Combinar esses dois nos em um novo no interno.
5. Reinserir o novo no no heap ate restar apenas a raiz da arvore.
6. Percorrer a arvore para gerar a tabela de codigos de cada byte.
7. Usar a tabela para transformar bytes em sequencias de bits.
8. Serializar a arvore no cabecalho do bloco para permitir a descompressao.

A decodificacao deve ser feita descendo a arvore bit a bit. Cada `0` ou `1` leva para um filho da arvore ate chegar em uma folha, que representa um byte original.

## Parte de Sistemas Operacionais

O nucleo de Sistemas Operacionais e o pipeline concorrente. A arquitetura esperada e:

- Uma thread leitora le blocos do arquivo de entrada.
- N threads codificadoras comprimem blocos de forma independente.
- Uma thread escritora recebe os blocos comprimidos e grava na ordem correta.

Para coordenar as threads, o projeto deve usar:

- Filas limitadas.
- `pthread_mutex_t`.
- `pthread_cond_t`.
- Controle de fim de producao.
- Reordenacao por indice de bloco.
- Ausencia de data races, validada com ThreadSanitizer.

Cada bloco deve ser independente para permitir paralelismo real. Isso tambem facilita a recuperacao parcial quando um bloco estiver corrompido.

## Formato geral esperado do arquivo compactado

O arquivo `.cz` deve conter informacoes suficientes para que `cunzip` consiga reconstruir o arquivo original sem depender da memoria ou do processo que gerou a compressao.

Decisao de projeto (nao exigencia do edital): o edital nao define extensao de arquivo nem magic number. A extensao `.cz` e o magic number `CZHF` sao escolhas da equipe e podem ser alteradas, desde que documentadas e justificaveis na defesa oral. O edital exige apenas que cada bloco carregue sua arvore de Huffman serializada no cabecalho.

Um formato razoavel deve conter:

- Assinatura/magic number do arquivo, por exemplo `CZHF`.
- Versao do formato.
- Tamanho de bloco usado na compressao.
- Quantidade total de blocos.
- Para cada bloco:
  - indice do bloco;
  - tamanho original;
  - tamanho comprimido;
  - CRC32 do conteudo original ou comprimido, conforme decisao documentada;
  - tamanho da arvore serializada;
  - arvore serializada;
  - payload comprimido em bits.

A serializacao da arvore e obrigatoria, pois cada bloco pode ter uma arvore diferente.

## Teste de fogo

O teste de fogo definido pelo edital para o Tema 11 exige:

- Compactar um arquivo de 1 GB.
- Demonstrar speedup quase linear ate o numero de nucleos disponiveis.
- Identificar quando o estagio de escrita passa a serializar o pipeline.
- Corromper manualmente um bloco do arquivo `.cz`.
- Detectar o bloco corrompido por CRC32.
- Continuar descomprimindo os demais blocos validos.

Esse teste impede uma solucao trivial, porque exige desempenho, concorrencia correta, integridade por bloco e recuperacao parcial.

## Estado atual do repositorio

O repositorio contem principalmente documentacao inicial e regras de trabalho. Foram identificados os seguintes arquivos relevantes:

- `README.md`: contem apenas o nome e uma descricao curta do projeto.
- `RULES.md`: define regras obrigatorias, escopo do Tema 11, cronograma, criterios de avaliacao e padrao de registro no diario.
- `DIARIO.md`: contem a primeira entrada de uso de IA.
- `Makefile`: ja referencia os binarios `czip` e `cunzip` e arquivos esperados em `src/` e `tests/`.
- `docs/trabalho.txt`: contem a transcricao do edital do trabalho interdisciplinar.
- `docs/Whatbuild.md`: contem um resumo simples do que precisa ser construido.
- `docs/Trabalho_Interdisciplinar_ED_SO_Ifes.pdf`: edital original.

No momento da analise, nao foram encontrados arquivos em `src/` nem em `tests/`. Portanto, o `Makefile` ja indica uma arquitetura esperada, mas a implementacao em C ainda precisa ser criada.

## Dificuldade de implementacao

A dificuldade do projeto e alta para um trabalho academico porque ele mistura varios problemas que precisam funcionar corretamente ao mesmo tempo:

- Manipulacao de bits para gravar e ler codigos de Huffman.
- Implementacao correta de heap binario minimo.
- Construcao, percurso, serializacao e liberacao de memoria da Arvore de Huffman.
- Projeto de um formato binario proprio para o `.cz`.
- Uso de CRC32 por bloco.
- Compressao e descompressao por blocos independentes.
- Pipeline com threads, filas limitadas e condition variables.
- Reordenacao da saida quando trabalhadores terminam fora de ordem.
- Testes com arquivos pequenos, grandes, texto, binarios e dados aleatorios.
- Validacao com AddressSanitizer, Valgrind e ThreadSanitizer.

O ponto mais dificil provavelmente nao e construir a arvore de Huffman isoladamente, mas integrar Huffman, bit I/O, formato binario, CRC32 e concorrencia sem corromper dados.

## Tempo estimado de conclusao usando IA

Usando IA como apoio para gerar partes incrementais, revisar codigo, criar testes e ajudar na depuracao, uma estimativa realista e:

- Fundacao do projeto, structs, heap, CRC32 e testes iniciais: 1 a 2 dias.
- Huffman completo single-thread, incluindo tabela de codigos, bit I/O e serializacao: 2 a 4 dias.
- `czip` e `cunzip` single-thread funcionando com arquivos pequenos e medios: 1 a 2 dias.
- Pipeline concorrente com filas limitadas, condvars e reordenador: 2 a 4 dias.
- Testes de stress, deteccao de corrupcao, ajustes de formato e correcao de bugs: 2 a 4 dias.
- Relatorio experimental, graficos e preparacao da defesa: 2 a 3 dias.

Estimativa total com IA: aproximadamente 10 a 19 dias de trabalho incremental.

Essa estimativa considera que o grupo vai testar frequentemente e corrigir bugs durante o desenvolvimento. Tentar gerar tudo de uma vez aumentaria muito o risco de codigo incorreto, dificil de explicar e dificil de defender oralmente.

## Riscos principais

- Implementar Huffman sem serializar a arvore no arquivo, o que impediria a descompressao independente.
- Usar uma unica arvore global para o arquivo inteiro, reduzindo o paralelismo por blocos.
- Nao tratar corretamente o ultimo byte parcial do fluxo comprimido.
- Nao registrar tamanhos originais e comprimidos por bloco.
- Nao manter a ordem dos blocos na escrita.
- Criar data races no pipeline concorrente.
- Ignorar blocos corrompidos de forma incorreta, perdendo sincronizacao do arquivo.
- Nao conseguir explicar o codigo na defesa oral.

## Conclusao

O Tema 11 pede um compressor realista em C, com foco em Huffman e concorrencia. A entrega deve provar que o grupo entende tanto a estrutura de dados quanto os mecanismos de SO envolvidos. A melhor estrategia e implementar o projeto em etapas pequenas: primeiro heap e Huffman, depois formato binario e bit I/O, em seguida `czip/cunzip` single-thread e, por fim, pipeline com threads e testes de robustez.
