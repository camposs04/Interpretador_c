# Análise Semântica (semantic.md)

## 1. Visão Geral

Este módulo descreve as **ações semânticas** associadas às regras da gramática.

A implementação atual realiza a **avaliação de expressões aritméticas em tempo de parsing**.

---

## 2. Representação de Valores

```c
%union {
    int intValue;
}
```

* Todos os valores são tratados como inteiros

---

## 3. Avaliação de Expressões

### Soma

```c
$$ = $1 + $3;
```

### Subtração

```c
$$ = $1 - $3;
```

### Multiplicação

```c
$$ = $1 * $3;
```

### Divisão

```c
$$ = $1 / $3;
```

### Parênteses

```c
$$ = $2;
```

### Número

```c
$$ = $1;
```

---

## 4. Execução

```c
expressao PONTO_VIRGULA {
    printf("%d\n", $1);
}
```

* Expressões são avaliadas e impressas imediatamente

---

## 5. Fluxo de Execução

1. O analisador léxico gera tokens
2. O parser reconhece a estrutura
3. As ações semânticas calculam os valores
4. O resultado é exibido

---

## 6. Limitações Atuais

* Não há tabela de símbolos
* Variáveis não armazenam valores
* Não há verificação de tipos
* Não há tratamento de erros em tempo de execução
* Estruturas como `if` não possuem execução real

---

## 7. Funcionalidades Ausentes

### 7.1 Tabela de Símbolos

* Necessária para armazenar variáveis

### 7.2 Sistema de Tipos

* Garantir consistência entre tipos

### 7.3 Controle de Fluxo

* Execução real de `if`, `for`, etc.

### 7.4 Validação em Tempo de Execução

* Divisão por zero
* Variáveis não declaradas

---

## 8. Exemplo

### Entrada

```c
10 + 2 * 3;
```

### Avaliação

* `2 * 3 = 6`
* `10 + 6 = 16`

### Saída

```text
16
```

---

## 9. Conclusão

O sistema atual funciona como:

* Um **validador sintático**
* Um **avaliador de expressões aritméticas**

Ainda não representa um interpretador completo da linguagem C, mas constitui uma base sólida para evolução futura.

---
