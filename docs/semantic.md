# Análise Semântica (semantic.md)

## 1. Visão Geral

O sistema atual NÃO executa código.

Ele realiza:

* Construção da AST
* Inferência simples de tipos

---

## 2. Estrutura de Tipos

```c
typedef enum {
    T_INT,
    T_FLOAT,
    T_CHAR,
    T_BOOL
}
```

---

## 3. Inferência de Tipos

Regra:

* Se algum operando é `float` → resultado `float`
* Caso contrário → `int`

---

## 4. Declarações

Tipos são propagados na AST:

```c
int x, y;
```

---

## 5. Estrutura do Nó AST

```c
struct noAST {
    operador
    tipo
    valor
    nome
    esquerda
    direita
}
```

---

## 6. Impressão

A AST é convertida para código:

```c
int x = 5;
```

---

## 7. Limitações

* Sem tabela de símbolos
* Sem validação de tipos
* Sem execução
* Sem escopo

---

## 8. Próximos Passos

* Tabela de símbolos
* Verificação de tipos
* Execução real
* Controle de escopo

---
