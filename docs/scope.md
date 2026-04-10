# Escopo da Linguagem (scope.md)

## 1. Visão Geral

Este documento define o escopo funcional do interpretador de C implementado neste projeto.

Seu objetivo é estabelecer claramente:

* Quais funcionalidades são suportadas
* Quais limitações existem
* Até onde o interpretador se propõe a funcionar

Este escopo serve como referência para desenvolvimento, testes e evolução do sistema.

---

## 2. Objetivo do Interpretador

O interpretador tem como objetivo:

* Validar sintaticamente programas escritos em um subconjunto da linguagem C
* Avaliar expressões aritméticas
* Servir como base para evolução incremental de um interpretador completo

---

## 3. Funcionalidades Suportadas

### 3.1 Tipos de Dados

* `int`
* `float` 
* `char` 

---

### 3.2 Declaração de Variáveis

Suporte a declarações simples:

```c
int x;
float y;
char c;
```

---

### 3.3 Atribuição

* Atribuição no momento da declaração:

```c
int x = 10;
float y = 5;
char c = 'a';
```

* Atribuição simples:

```c
x = 5 + 3;
```

---

### 3.4 Expressões Aritméticas

Suporte aos operadores:

* `+`
* `-`
* `*`
* `/`

Com precedência correta:

* Multiplicação e divisão possuem maior prioridade

Exemplo:

```c
10 + 2 * 3;
```

---

### 3.5 Estruturas de Controle

#### `if`

```c
if (10) x = 5;
```

* Reconhecido sintaticamente
* Não há execução semântica real

---

### 3.6 Entrada e Saída

Tokens reconhecidos:

* `printf`
* `scanf`

Não há implementação funcional dessas operações

---

## 4. Funcionalidades Parcialmente Suportadas

* `float`: tratado como inteiro na avaliação
* `char`: suporte apenas em atribuições simples
* Operadores relacionais: reconhecidos no lexer, mas não integrados ao parser

---

## 5. Funcionalidades Não Suportadas

### 5.1 Estruturas de Controle

* `else`
* `for`
* `while`
* `switch`

---

### 5.2 Sistema de Tipos

* Verificação de tipo
* Conversão de tipos
* Tipos compostos

---

### 5.3 Memória e Estado

* Não há tabela de símbolos
* Variáveis não armazenam valores
* Não há escopo de variáveis

---

### 5.4 Funções

* Declaração de funções
* Chamadas de função
* Parâmetros

---

### 5.5 Strings e Arrays

* Strings (`"texto"`)
* Vetores (`int v[10]`)

---

### 5.6 Comentários

* `// comentário`
* `/* comentário */`
