# Política de Economia de Tokens

Objetivo: reduzir o uso de tokens **sem prejudicar** os entregáveis avaliados
> do Tema 11 — defesa oral (50%) e a documentação de uso de IA exigida pelo
> edital (`RULES.md` REGRA 1).
>
> Este arquivo fica no repositório (vai junto no `git clone`), diferente da
> memória interna da IA, que é apenas local.

## Princípio

Separar **custo de chat** (saída na conversa, custo único da sessão) de **custo
recorrente** (arquivos lidos no início de toda sessão). Cortar agressivamente o
que é chat e o que não é avaliado; preservar o que vale nota.

| Onde corta | Custo que economiza | Risco de nota |
|------------|---------------------|---------------|
| Saída de chat (diffs, resumos) | sessão atual | nenhum |
| Arquivos não lidos a cada sessão (docs de módulo) | ~zero recorrente | alto se capar — material da defesa |
| `DIARIO.md` (lido toda sessão) | recorrente real | médio — exigido pelo edital |

## Regras adotadas como estão (ganho puro, zero risco)

1. Não imprimir diffs, snippets longos, conteúdo de arquivos criados nem
   previews de `Write`/`Edit` no chat.
2. Por edição, responder só: **arquivo alterado, objetivo, motivo**.
3. Logs de teste: só comando e resultado. Se falhar, mostrar o **menor erro
   decisivo**, não o log inteiro.
4. Resumo final do turno: **máximo 8 linhas**.
5. **Não** criar/exportar `docs/TerminalSession/claudeSession*.md` salvo pedido
   explícito (a REGRA 14 do `RULES.md` está desativada; `/export` é manual).
6. **Não** reler claudeSessions antigas salvo pedido — são apenas histórico.
7. Usar `git status --short` e `git diff --stat`; evitar diff completo.

## Regras ajustadas (para não custar nota)

### `DIARIO.md` — encurtar, mas preservar a exigência do edital

- É exigido pelo edital (REGRA 1) com 4 campos e é lido no início de toda
  sessão, então encurtar dá ganho recorrente real.
- **Manter os 4 campos**: prompt do aluno, resposta da IA, erros/correções,
  decisões de projeto.
- Prompt do aluno = **resumo de 1 linha** (a REGRA 1 permite "resumo fiel").
- Resposta da IA = enxuta (o que foi feito + resultado dos testes).
- Decisões de projeto = bullets curtos, **mantidas** (munição da defesa oral).
- Alvo prático ~15 linhas; **sem** cap rígido quando as decisões exigirem mais.
- Não cortar o prompt por completo nem capar em 12 linhas: descumpriria a
  REGRA 1 e apagaria decisões que alimentam a defesa.

### `docs/modulos/modulo_XX.md` — **não** capar em 50 linhas

- Esse arquivo **não** é lido a cada sessão (a task lê só `_template.md`), então
  capar economiza ~zero recorrente e arrisca a defesa.
- Manter focado e sem enchimento, mas **preservar** as seções
  "Como explicar na defesa" e "Decisões de projeto" — são o material de estudo
  do aluno para a banca.

## Resultado esperado

- Economia principal vem do chat (regras 1–4, 7) + parar de gerar
  `claudeSession*.md` + não reler sessões antigas.
- `DIARIO.md` encurtado ~metade mantendo conformidade com o edital.
- Docs de módulo intactos no que vale nota.
- Sem impacto negativo na entrega avaliada.

## Histórico

- 2026-06-22: política proposta pelo aluno (9 regras de economia de tokens),
  analisada e refinada (regras de `DIARIO.md` e docs de módulo ajustadas para
  não violar o edital nem enfraquecer a defesa). Registrada também na memória
  local da IA para valer nas próximas sessões.
