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

