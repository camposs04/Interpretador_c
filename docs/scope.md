# Escopo da Linguagem (scope.md)

## 1. Visão Geral

Este documento define o que a linguagem suporta ao final da sprint 4.

O sistema funciona como:

* Analisador léxico (Flex)
* Analisador sintático e construtor de AST (Bison)
* Tabela de símbolos (hash table)
* Gerador de TAC

Não executa código.

---

## 2. Tipos Suportados

* `int`
* `float`
* `char`
* `bool`

---

## 3. Declarações

```c
int x;
int x, y;
int x = 10;
int x = 10, y = 20;
float f = 3.14;
char c = 'a';
```

Cada variável declarada é inserida na tabela de símbolos com seu tipo.

---

## 4. Atribuições

```c
x = 5;
x = y + 2;
```

---

## 5. Expressões

### Aritméticas

```c
x + y
x - y
x * y
x / y
```

### Relacionais

```c
x == y
x != y
x < y
x > y
x <= y
x >= y
```

---

## 6. Controle de Fluxo

```c
if (x > 10) {
    y = 5;
}

if (x > 10) {
    y = 5;
} else {
    y = 0;
}
```

---

## 7. Blocos

```c
{
    int x;
    x = 1;
}
```

---

## 8. Tabela de Símbolos

Introduzida nesta sprint. Armazena todas as variáveis declaradas com nome e tipo, usando uma hash table de 211 buckets com encadeamento separado para colisões.

Validações ativas:

* Rejeita redeclaração de variável com mesmo nome
* Permite consulta por nome via `searchSymbol`

---

## 9. Geração de TAC

A partir desta sprint, ao final do parsing a AST é percorrida e o TAC é impresso. Exemplo:

**Entrada:**
```c
int x = 3 + 5;
if (x > 2) { x = x - 1; }
```

**Saída:**
```
TAC do programa:
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

## 10. Não Suportado

* Execução real do programa
* Escopo aninhado (blocos criam escopo sintático, não semântico)
* Funções
* Strings
* Arrays
* `while` e `for`

---

## 11. Objetivo

Base para:

* Escopo semântico com `entrarEscopo` / `sairEscopo`
* Verificação de tipos em atribuições e expressões
* Interpretador com execução real