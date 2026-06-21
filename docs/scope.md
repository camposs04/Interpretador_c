# Escopo e Funcionalidades (scope.md)

## 1. Visão Geral

Este documento define o que a linguagem **suporta no seu estado atual**, refletindo um projeto maduro.

O sistema é um pipeline completo:
* **Análise Léxica (Flex)**.
* **Análise Sintática e AST (Bison)**.
* **Análise Semântica Robusta** (Checagem de tipos, controle hierárquico de símbolos).
* **Interpretador em Tempo Real** (Executa nativamente as lógicas lidas).
* **Geração de TAC** (Transpilação para código intermediário).

---

## 2. Tipos e Estruturas Suportados

* `int`, `float`, `char`, `bool`
* Funções com suporte a tipo `void`
* Variáveis e matrizes unidimensionais (Vetores / Arrays)

---

## 3. Escopo Lexical e Semântico

* **Escopo Aninhado**: Suportado totalmente. Blocos `{}` criam um novo espaço de contexto (via pilha de tabelas de símbolos `EscopoRT` e `Escopo` estático). Variáveis declaradas dentro do bloco não vazam, permitindo *shadowing*.
* **Funções Globais**: Funções possuem escopos isolados onde argumentos entram por cópia no contexto de tempo de execução.

---

## 4. Declarações e Atribuições

Variáveis primitivas e vetores:

```c
int x, y;
int arr[10];
arr[0] = 5;
x = arr[0] += 2;
```
As atribuições compostas (`+=`, etc) e incrementos unários (`x++`, `--x`) são plenamente funcionais e alteram o estado no interpretador.

---

## 5. Laços e Condicionais

Amplo controle de fluxo implementado:
* `if` / `if-else`
* `while` e `for`
* Interrupção de laços com `break`

---

## 6. Funções e Retornos

As funções podem ser declaradas com múltiplos parâmetros ou `void`.
O interpretador manipula pilhas de chamada, garantindo que o `return` interrompa o fluxo apenas da função atual, devolvendo o controle.

```c
int soma(int a, int b) {
    return a + b;
}
```

---

## 7. Entrada e Saída (I/O)

Injeção das funções nativas da linguagem:
* **`printf("string literal", args...)`**: Capaz de printar formatações como no C clássico.
* **`scanf("string", &var)`**: Capta entradas interativamente durante a interpretação via CLI.

---

## 8. Avaliação de Expressões (Constant Folding)

A construção de expressões conta com lógicas robustas que evitam criação de galhos em excesso na AST. Literais matemáticos puros (`3 + 5 * 2`) se fundem na própria análise sintática em um nó de resultado único para ganhos de performance. Expressões envolvendo booleanos (`x && false`) são atalhadas.

---

## 9. Execução Real

O programa de fato **EXECUTA** os scripts enviados (quando não ocorrem erros semânticos na compilação estática). Uma pilha paralela em `interpreter.c` resolve os saltos condicionais e manipula os ponteiros associados às alocações de `tabsym` para simular o _runtime_ em memória.