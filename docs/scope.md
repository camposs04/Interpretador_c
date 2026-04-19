# Escopo da Linguagem (scope.md)

## 1. Visão Geral

Este documento define o que a linguagem suporta atualmente.

O sistema funciona como:

* Parser
* Gerador de AST

Não executa código.

---

## 2. Tipos Suportados

* int
* float
* char
* bool

---

## 3. Declarações

```c
int x;
int x, y;
int x = 10;
```

---

## 4. Atribuições

```c
x = 5;
```

---

## 5. Expressões

Suporte completo:

* Aritméticas
* Relacionais

---

## 6. Controle de Fluxo

```c
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
}
```

---

## 8. Não Suportado

* Execução real
* Variáveis armazenadas
* Funções
* Strings
* Arrays
* Loops reais

---

## 9. Objetivo

Base para:

* Interpretador
* Compilador
* Análise semântica futura

---
