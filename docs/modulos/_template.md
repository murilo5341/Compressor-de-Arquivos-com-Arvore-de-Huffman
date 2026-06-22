# Módulo NN — <Nome do módulo>

> Esqueleto padrão para documentar cada módulo. Copie este arquivo para
> `docs/modulos/modulo_NN.md`, preencha as seções e remova esta citação.
> Objetivo: dar base técnica para a **defesa oral** (peso 50%). Não explicar
> linha por linha — focar no que faz, por que existe e nas estruturas/funções principais.

Resumo em 1–2 linhas: o que este módulo entrega.

## O que faz

- <bullet objetivo>
- <bullet objetivo>

## Por que existe

Qual papel cumpre no projeto e qual necessidade do edital / regra do `RULES.md` atende.

## Arquivos

| Arquivo | Papel |
|---------|-------|
| `include/<x>.h` | <descrição> |
| `src/<x>.c` | <descrição> |
| `tests/test_<x>.c` | <descrição> |

## Estruturas principais

As `struct`s / tipos centrais do módulo e o porquê de cada campo importante.

```c
// exemplo
typedef struct { ... } Tipo;
```

## Funções principais

| Função | O que faz |
|--------|-----------|
| `tipo_funcao(args)` | <uma linha> |

## Como compilar e testar

```sh
# Linux
make            # compila
make test       # roda os testes unitarios

# Windows (MinGW)
mingw32-make all
```

Saída/critério esperado do teste: <o que comprova que funciona>.

## Como explicar na defesa

- <ponto que o aluno precisa dominar>
- Pergunta provável: "<pergunta>" → <resposta curta>

## Decisões de projeto / referências

- Decisão: <…> (registrada no `DIARIO.md`).
- Ver também: `modularizacao.md` (Módulo NN), `RULES.md` (REGRA X).
