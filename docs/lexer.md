# Analisador Léxico (lexer.md)

## 1. Visão Geral

O analisador léxico foi implementado com **Flex**.

Responsabilidades:

* Ler o código fonte
* Identificar tokens
* Controlar posição (linha/coluna)
* Reportar erros léxicos

---

## 2. Estrutura Interna

Variáveis globais:

```c
int linha = 1;
int coluna = 1;
char linhaAtual[1024];
```

* `linha`: número da linha atual
* `coluna`: posição do caractere
* `linhaAtual`: conteúdo da linha (para exibição de erros)

### Dependências incluídas

```c
#include "parserC.tab.h"
#include "tabsym.h"
```

A inclusão de `tabsym.h` conecta o lexer à tabela de símbolos, permitindo que identificadores reconhecidos sejam consultados durante a análise léxica.

---

## 3. Tokens Reconhecidos

### 3.1 Palavras-chave

| Palavra   | Token    |
|-----------|----------|
| `if`      | IF       |
| `else`    | ELSE     |
| `int`     | INT      |
| `float`   | FLOAT    |
| `char`    | CHAR     |
| `bool`    | BOOL     |
| `for`     | FOR      |
| `return`  | RETURN   |
| `break`   | BREAK    |
| `void`    | VOID     |
| `printf`  | PRINTF   |
| `scanf`   | SCANF    |

---

### 3.2 Identificadores

```text
[a-zA-Z][a-zA-Z0-9]*
```

* Devem iniciar com letra
* Podem conter números
* Valor enviado ao parser via `yylval.id` com `strdup`

---

### 3.3 Literais

| Tipo    | Exemplo | Token     |
|---------|---------|-----------|
| Inteiro | `10`    | INT_NUM   |
| Float   | `10.5`  | FLOAT_NUM |
| Char    | `'a'`   | CHAR_NUM  |

---

### 3.4 Operadores

#### Aritméticos

`+` `-` `*` `/` `%`

#### Relacionais

`==` `!=` `<` `>` `<=` `>=`

#### Lógicos

`&&` `||` `!`

#### Atribuição simples

`=`

#### Atribuição composta

`+=` `-=` `*=` `/=` `%=`

#### Incremento e decremento

`++` `--`

---

## 4. Comentários

```c
// comentário de linha
```

* Ignorados pelo parser
* Mantidos no buffer `linhaAtual` para exibição em mensagens de erro

---

## 5. Controle de Posição

Cada token atualiza `coluna` com `coluna += yyleng`. O texto do token é armazenado em `linhaAtual` via macro:

```c
#define ADD_TEXTO() \
    if (posLinha + yyleng < 1023) { \
        strncpy(&linhaAtual[posLinha], yytext, yyleng); \
        posLinha += yyleng; \
        linhaAtual[posLinha] = '\0'; \
    }
```

Ao encontrar `\n`, `linhaAtual` é reiniciado e `linha` é incrementado.

---

## 6. Integração com Parser

Valores enviados via `yylval`:

| Campo         | Tipo    | Usado por          |
|---------------|---------|--------------------|
| `intValue`    | `int`   | INT_NUM, CHAR_NUM  |
| `floatValue`  | `float` | FLOAT_NUM          |
| `id`          | `char*` | ID                 |

---

## 7. Integração com Tabela de Símbolos

O lexer inclui `tabsym.h` diretamente. Isso permite que futuras regras léxicas consultem ou pré-registrem símbolos conforme os identificadores são reconhecidos, sem depender de fases posteriores.

---

## 8. Tratamento de Erros

Qualquer caractere não reconhecido cai na regra padrão:

```c
. { erro }
```

Saída exibida:

```
ERROR LEXER
Line: X Column: Y | error: invalid character '?'
    código fonte
    ^
```

---

## 9. Fim de Arquivo

```c
int yywrap() {
    return 1;
}
```