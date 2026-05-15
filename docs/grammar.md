# Analisador Sintático (grammar.md)

## 1. Visão Geral

Este módulo define a **gramática da linguagem** utilizando o gerador de parser **Bison**.

O parser é responsável por:

* Validar a estrutura sintática do programa
* Construir uma **Árvore Sintática Abstrata (AST)**
* Disparar a geração de TAC ao final do parsing
* Tratar erros sintáticos com indicação de linha e coluna

A gramática implementa um subconjunto da linguagem C com foco em:

* Declarações de variáveis
* Atribuições
* Expressões aritméticas e relacionais
* Estruturas condicionais (`if` / `if-else`)
* Blocos `{}`

### Dependências incluídas

```c
#include "ast.h"
#include "tac.h"
```

---

## 2. Estrutura Geral do Programa

### Regra principal

```c
programa:
    lista
;
```

Ao final do parsing, o TAC é gerado e impresso:

```c
printf("\nTAC do programa:\n");
gerarTAC(root);
```

---

## 3. Sequência de Instruções

```c
lista:
    lista elemento
    | elemento
;
```

* Implementa múltiplas instruções via recursão à esquerda
* Internamente gera nós de sequência (`;`) na AST

---

## 4. Elementos

```c
elemento:
    comando
;
```

---

## 5. Blocos

```c
bloco:
    ABRE_CHAVES comandos FECHA_CHAVES
;

comandos:
    comandos comando
    | comando
;
```

* Representam agrupamento de instruções
* Sem escopo semântico nesta sprint — apenas estrutura sintática

---

## 6. Comandos

```c
comando:
    expressao PONTO_VIRGULA
    | atribuicao
    | declaracao
    | bloco
    | IF OPEN_PAREN expressao CLOSE_PAREN bloco
    | IF OPEN_PAREN expressao CLOSE_PAREN bloco ELSE bloco
    | error PONTO_VIRGULA
;
```

### Tipos suportados

* Expressões simples
* Declarações de variáveis
* Atribuições
* Blocos `{}`
* `if` e `if-else`
* Recuperação de erro via `error PONTO_VIRGULA`

---

## 7. Tipos

```c
tipo:
    INT | FLOAT | CHAR | BOOL
;
```

Retorna o valor do enum `Tipo` correspondente (`T_INT`, `T_FLOAT`, `T_CHAR`, `T_BOOL`).

---

## 8. Declarações

```c
declaracao:
    tipo lista_ids PONTO_VIRGULA
;

lista_ids:
    ID
    | ID EQUAL expressao
    | lista_ids VIRGULA ID
    | lista_ids VIRGULA ID EQUAL expressao
;
```

### Exemplos válidos

```c
int x;
int x, y, z;
int x = 10;
int x = 10, y = 20;
```

### Propagação de tipo

O tipo é aplicado após a criação dos nós via travessia da lista:

```c
NoAST *aux = $$;
while (aux != NULL) {
    if (aux->operador == ';') {
        if (aux->esquerda) aux->esquerda->tipo = $1;
        aux = aux->direita;
    } else {
        aux->tipo = $1;
        break;
    }
}
```

---

## 9. Atribuição

```c
atribuicao:
    ID EQUAL expressao PONTO_VIRGULA
    | ID EQUAL error PONTO_VIRGULA
;
```

Exemplo:

```c
x = 5 + 3;
```

---

## 10. Expressões

```c
expressao:
    expressao PLUS expressao
    | expressao MINUS expressao
    | expressao MULT expressao
    | expressao DIV expressao
    | expressao DEQ expressao
    | expressao NEQ expressao
    | expressao LT expressao
    | expressao GT expressao
    | expressao LE expressao
    | expressao GE expressao
    | OPEN_PAREN expressao CLOSE_PAREN
    | INT_NUM
    | FLOAT_NUM
    | CHAR_NUM
    | ID
;
```

---

## 11. Precedência de Operadores

```c
%left DEQ NEQ
%left LT GT LE GE
%left PLUS MINUS
%left MULT DIV
```

| Prioridade | Operadores           |
|------------|----------------------|
| Alta       | `*`, `/`             |
| Média      | `+`, `-`             |
| Baixa      | `<`, `>`, `<=`, `>=` |
| Mais baixa | `==`, `!=`           |

Todos associativos à esquerda.

---

## 12. Construção da AST

Cada regra cria nós específicos via funções de `ast.h`:

| Estrutura     | Nó  | Função criadora   |
|---------------|-----|-------------------|
| Número        | `n` | `criarNoInt`, `criarNoFloat`, `criarNoChar` |
| Identificador | `i` | `criarNoId`       |
| Operação      | operador (`+`, `-`, etc.) | `criarNoOp` |
| Declaração    | `d` | `criarNoDecl`     |
| Atribuição    | `=` | `criarNoAtrib`    |
| Sequência     | `;` | `criarNoSeq`      |
| If            | `f` | `criarNoIf`       |

---

## 13. Geração de TAC

Ao final do parsing, a função `gerarTAC` (definida em `tac.c`) percorre a AST e emite instruções de três endereços. Exemplo:

**Entrada:**
```c
int x = 3 + 5;
if (x > 2) { x = x - 1; }
```

**TAC gerado:**
```
decl int x
t1 = 3
t2 = 5
t3 = t1 + t2
x = t3
t4 = x
t5 = 2
t6 = t4 > t5
if_false t6 goto L1
t7 = x
t8 = 1
t9 = t7 - t8
x = t9
L1:
```

---

## 14. Tratamento de Erros

```c
| error PONTO_VIRGULA { yyerrok; yyclearin; }
```

* Permite continuar o parsing após um erro
* Evita abortar o programa inteiro

### Função de erro

```c
void yyerror(const char *s)
```

Exibe linha, coluna, token inesperado e indicador `^` no código fonte.

---

## 15. Técnica Utilizada

* Parser **LALR(1)** gerado automaticamente pelo Bison
* Look-ahead de 1 token
* Alta eficiência e baixo uso de memória