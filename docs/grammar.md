# Analisador Sintático (grammar.md)

## 1. Visão Geral

Este arquivo define a documentação da gramática livre de contexto da linguagem utilizando Bison.

Sua função é validar a estrutura sintática do programa com base nas regras definidas.

---

## 2. Ponto de Entrada

```c
programa:
    lista
;
```

* Um programa é composto por uma lista de elementos

---

## 3. Estrutura do Programa

### 3.1 Lista de Elementos

```c
lista:
    lista elemento
    | elemento
;
```

* Permite múltiplas instruções por meio de recursão à esquerda

---

### 3.2 Elementos

```c
elemento:
    declaracao
    | atribuicao
    | comando
;
```

---

## 4. Declarações

```c
declaracao:
    INT ID PONTO_VIRGULA
    | FLOAT ID PONTO_VIRGULA
    | CHAR ID PONTO_VIRGULA
;
```

* Declaração de variáveis sem inicialização

---

## 5. Atribuições

```c
atribuicao:
    INT ID EQUAL NUM PONTO_VIRGULA
    | FLOAT ID EQUAL NUM PONTO_VIRGULA
    | CHAR ID EQUAL ASPASSIMPLES ID ASPASSIMPLES PONTO_VIRGULA
;
```

* Inicialização de variáveis no momento da declaração

---

## 6. Comandos

```c
comando:
    expressao PONTO_VIRGULA
    | IF OPEN_PAREN expressao CLOSE_PAREN comando
    | ID EQUAL expressao PONTO_VIRGULA
;
```

Tipos suportados:

* Avaliação de expressões
* Estrutura condicional (`if`)
* Atribuição

---

## 7. Expressões

```c
expressao:
      expressao PLUS expressao
    | expressao MINUS expressao
    | expressao MULT expressao
    | expressao DIV expressao
    | OPEN_PAREN expressao CLOSE_PAREN
    | NUM
;
```

---

## 8. Precedência de Operadores

```c
%left PLUS MINUS
%left MULT DIV
```

| Prioridade | Operadores |
| ---------- | ---------- |
| Alta       | `*`, `/`   |
| Baixa      | `+`, `-`   |

---

## 9. Tipagem Semântica

```c
%union {
    int intValue;
}
```

* Todas as expressões são tratadas como inteiras

---

## 10. Tratamento de Erros

```c
void yyerror(const char *s) {
    printf("Erro sintatico: %s\n", s);
}
```

## 11. Regras de Gramática e Precedência

Para garantir que expressões matemáticas sejam calculadas na ordem correta, a gramática define níveis de prioridade para os operadores. Operadores com maior precedência são "agrupados" antes dos de menor precedência.

| Operador | Descrição | Precedência | Associatividade |
| ---------- | ---------- | ---------- | ---------- |
| `*`, `/`, `%` | Multiplicação, Divisão e Módulo | Alta | Esquerda |
| `+`, `-` | Soma e Subtração | Baixa | Esquerda |

### Associatividade à Esquerda

A gramática utiliza **associatividade à esquerda** para operadores de mesma precedência. Isso significa que cálculos em sequência são resolvidos da esquerda para a direita.
- **Exemplo:** A expressão `10 - 5 - 2` é interpretada pelo parser como `(10 - 5) - 2`, resultando em `3`, e não como `10 - (5 - 2)`.

## Detalhamento Técnico

O interpretador utiliza o algoritmo **LALR (Look-Ahead Left-to-Right)**, gerado pelo Bison. Este método é o padrão para linguagens de programação modernas por combinar dois fatores críticos:

1. **Eficiência de Memória**: Utiliza tabelas de estados reduzidas em comparação ao LR(1) canônico, mantendo o compilador leve.

2. **Poder de Antecipação (Look-Ahead):** O parser consegue "olhar" o próximo token da entrada para decidir qual regra da gramática aplicar, o que ajuda a evitar ambiguidades (conflitos) e permite processar gramáticas complexas de forma determinística.