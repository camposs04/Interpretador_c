# Analisador Sintático (grammar.md)

## 1. Visão Geral

Este módulo define a **gramática da linguagem** utilizando o gerador de parser **Bison**.

O parser é responsável por:

* Validar a estrutura sintática do programa
* Construir uma **Árvore Sintática Abstrata (AST)**
* Tratar erros sintáticos com indicação de linha e coluna

A gramática implementa um subconjunto da linguagem C com foco em:

* Declarações
* Atribuições
* Expressões
* Estruturas condicionais (`if / else`)
* Blocos `{}`

---

## 2. Estrutura Geral do Programa

### Regra principal

```c
programa:
    lista
;
```

* Um programa é composto por uma sequência de elementos
* Ao final do parsing, a AST é impressa

---

## 3. Sequência de Instruções

```c
lista:
    lista elemento
    | elemento
;
```

* Implementa múltiplas instruções
* Utiliza recursão à esquerda
* Internamente gera nós de sequência (`;`) na AST

---

## 4. Elementos

```c
elemento:
    comando
;
```

* Todo elemento do programa é tratado como comando

---

## 5. Blocos

```c
bloco:
    ABRE_CHAVES comandos FECHA_CHAVES
;
```

```c
comandos:
    comandos comando
    | comando
;
```

* Representam agrupamento de instruções
* Não há escopo semântico ainda (apenas estrutura sintática)

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
;
```

### Tipos suportados:

* Expressões simples
* Declarações de variáveis
* Atribuições
* Blocos `{ }`
* Estruturas condicionais:

  * `if`
  * `if-else`

---

## 7. Tipos

```c
tipo:
    INT
    | FLOAT
    | CHAR
    | BOOL
;
```

Tipos suportados:

* `int`
* `float`
* `char`
* `bool`

---

## 8. Declarações

```c
declaracao:
    tipo lista_ids PONTO_VIRGULA
;
```

```c
lista_ids:
    ID
    | ID EQUAL expressao
    | lista_ids VIRGULA ID
    | lista_ids VIRGULA ID EQUAL expressao
;
```

### Exemplos válidos:

```c
int x;
int x, y, z;
int x = 10;
int x = 10, y = 20;
```

### Observação importante

* O tipo é aplicado **após a criação da AST**
* Cada identificador vira um nó de declaração (`d`)

---

## 9. Atribuição

```c
atribuicao:
    ID EQUAL expressao PONTO_VIRGULA
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

### Suporte completo:

* Aritméticos: `+ - * /`
* Relacionais: `== != < > <= >=`
* Parênteses
* Literais e identificadores

---

## 11. Precedência de Operadores

```c
%left DEQ NEQ
%left LT GT LE GE
%left PLUS MINUS
%left MULT DIV
```

| Prioridade | Operadores           |
| ---------- | -------------------- |
| Alta       | `*`, `/`             |
| Média      | `+`, `-`             |
| Baixa      | `<`, `>`, `<=`, `>=` |
| Mais baixa | `==`, `!=`           |

### Associatividade

* Todos são **associativos à esquerda**

---

## 12. Construção da AST

Cada regra cria nós específicos:

| Estrutura     | Nó                        |
| ------------- | ------------------------- |
| Número        | `n`                       |
| Identificador | `i`                       |
| Operação      | operador (`+`, `-`, etc.) |
| Declaração    | `d`                       |
| Atribuição    | `=`                       |
| Sequência     | `;`                       |
| If            | `f`                       |

---

## 13. Tratamento de Erros

```c
| error PONTO_VIRGULA { yyerrok; yyclearin; }
```

* Permite continuar parsing após erro
* Evita abortar o programa inteiro

### Função de erro

```c
void yyerror(...)
```

Exibe:

* Linha
* Coluna
* Token inesperado
* Linha do código
* Indicador `^`

---

## 14. Técnica Utilizada

* Parser **LALR(1)**
* Gerado automaticamente pelo Bison
* Usa look-ahead de 1 token
* Alta eficiência e baixo uso de memória

---
