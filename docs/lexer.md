# Analisador Léxico (lexer.md)

## 1. Visão Geral

Este arquivo documenta a  implementação do analisador léxico utilizando Flex. Sua principal responsabilidade é ler o código-fonte de entrada e convertê-lo em uma sequência de tokens que serão consumidos pelo analisador sintático.

O analisador reconhece:

* Palavras-chave
* Identificadores
* Literais numéricos
* Operadores
* Delimitadores

---

## 2. Dependências

```c
#include "parserC.tab.h"
#include <stdio.h>
#include <stdlib.h>
```

* `parserC.tab.h`: definição dos tokens compartilhados com o parser
* Bibliotecas padrão da linguagem C

---

## 3. Definição de Tokens

### 3.1 Palavras-chave

| Lexema | Token  |
| ------ | ------ |
| if     | IF     |
| else   | ELSE   |
| int    | INT    |
| float  | FLOAT  |
| char   | CHAR   |
| bool   | BOOL   |
| for    | FOR    |
| return | RETURN |
| break  | BREAK  |
| void   | VOID   |
| printf | PRINTF |
| scanf  | SCANF  |

---

### 3.2 Identificadores

```text
[a-zA-Z][a-zA-Z0-9]*
```

* Devem iniciar com uma letra
* Podem conter letras e números
* Token retornado: `ID`

---

### 3.3 Literais Numéricos

```text
[0-9]+"."[0-9]+
[0-9]+
```

* Suporte a inteiros e ponto flutuante
* Ambos retornam o token `NUM`

---

### 3.4 Operadores Aritméticos

| Símbolo | Token |
| ------- | ----- |
| +       | PLUS  |
| -       | MINUS |
| *       | MULT  |
| /       | DIV   |
| %       | MOD   |

---

### 3.5 Operadores de Atribuição

| Símbolo | Token      |
| ------- | ---------- |
| =       | EQUAL      |
| +=      | ADD_EQUAL  |
| -=      | SUB_EQUAL  |
| *=      | MULT_EQUAL |
| /=      | DIV_EQUAL  |
| %=      | MOD_EQUAL  |

---

### 3.6 Incremento e Decremento

| Símbolo | Token     |
| ------- | --------- |
| ++      | INCREMENT |
| --      | DECREMENT |

---

### 3.7 Operadores Lógicos

| Símbolo | Token |
| ------- | ----- |
| !       | NOT   |
| &&      | AND   |
| \|\|    | OR    |

---

### 3.8 Operadores Relacionais

Os operadores abaixo são reconhecidos, porém **ATUALMENTE** utilizados apenas para depuração:

```text
== != <= >= < >
```

Eles não retornam tokens, apenas imprimem mensagens no console.

---

### 3.9 Delimitadores

| Símbolo | Token         |
| ------- | ------------- |
| ;       | PONTO_VIRGULA |
| (       | OPEN_PAREN    |
| )       | CLOSE_PAREN   |
| {       | ABRE_CHAVES   |
| }       | FECHA_CHAVES  |
| ,       | VIRGULA       |
| '       | ASPASSIMPLES  |

---

## 4. Padrões Ignorados

```text
[ \t\n]+
```

* Espaços em branco são ignorados
* Não geram tokens

---

## 5. Tratamento de Erros

```c
. { printf("DESCONHECIDO(%s)\n", yytext); }
```

* Caracteres não reconhecidos são reportados como erro léxico

---

## 6. Fim de Arquivo

```c
int yywrap() {
    return 1;
}
```

* Indica o fim da entrada

