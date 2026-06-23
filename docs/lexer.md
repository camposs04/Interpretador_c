# Analisador Léxico (lexer.md)

## 1. Visão Geral

O analisador léxico foi implementado com **Flex**.

Responsabilidades:

* Ler o código fonte.
* Identificar tokens (palavras-chave, literais, operadores).
* Controlar posição (linha/coluna) com precisão.
* Reportar erros léxicos e fornecer o trecho falho para feedback visual.

---

## 2. Estrutura Interna

Variáveis globais mantidas e expostas via `extern` no Parser:

```c
int linha = 1;
int coluna = 1;
char linhaAtual[1024];
```

A inclusão de `tabsym.h` conecta o lexer ao gerenciador semântico, enquanto a própria `linhaAtual` mantém o buffer de contexto da linha para impressão dos erros apontados pelo Lexer ou Parser.

---

## 3. Tokens Reconhecidos

### 3.1 Palavras-chave

| Palavra   | Token    |
|-----------|----------|
| `if`      | IF       |
| `else`    | ELSE     |
| `while`   | WHILE    |
| `for`     | FOR      |
| `int`     | INT      |
| `float`   | FLOAT    |
| `char`    | CHAR     |
| `bool`    | BOOL     |
| `return`  | RETURN   |
| `break`   | BREAK    |
| `void`    | VOID     |
| `printf`  | PRINTF   |
| `scanf`   | SCANF    |

---

### 3.2 Identificadores

```text
[a-zA-Z_][a-zA-Z0-9_]*
```

* Devem iniciar com letra ou sublinhado `_`.
* Podem conter letras, números e sublinhados.
* Valor enviado via `yylval.id` alocando dinamicamente com `strdup()`.

---

### 3.3 Literais

| Tipo    | Exemplo   | Token          |
|---------|-----------|----------------|
| Inteiro | `10`      | INT_NUM        |
| Float   | `10.5`    | FLOAT_NUM      |
| Char    | `'a'`     | CHAR_NUM       |
| Boolean | `true`    | BOOL_VAL       |
| String  | `"texto"` | STRING_LITERAL |

* As **Strings** são capturadas eliminando as aspas delimitadoras (para serem enviadas nativamente limpas para os nós de `printf`/`scanf`).

---

### 3.4 Operadores e Delimitadores

#### Aritméticos
`+` `-` `*` `/` `%`

#### Lógicos e Relacionais
`==` `!=` `<` `>` `<=` `>=` `&&` `||` `!`

#### Atribuição e Compostos
`=` `+=` `-=` `*=` `/=` `%=`

#### Incremento e Decremento
`++` `--`

#### Delimitadores
`;` `,` `(` `)` `{` `}` `[` `]` `&` (para uso no scanf)

---

## 4. Comentários

```c
// comentário de linha
```

Ignorados sintaticamente, mas o texto consumido é salvo no buffer `linhaAtual` para manter a precisão das colunas.

---

## 5. Controle de Posição

Cada token atualiza `coluna` dinamicamente: `coluna += yyleng`.
O macro `ADD_TEXTO()` acumula os caracteres no buffer:

```c
#define ADD_TEXTO() \
    if (posLinha + yyleng < 1023) { \
        strncpy(&linhaAtual[posLinha], yytext, yyleng); \
        posLinha += yyleng; \
        linhaAtual[posLinha] = '\0'; \
    }
```

Ao encontrar `\n`, as posições são zeradas e `linha` é incrementada.

---

## 6. Tratamento de Erros

Caracteres inválidos (como `@`, `#`) disparam uma regra coringa `.`:

```
ERROR LEXER
Line: X Column: Y | error: invalid character '?'
    codigo fonte com erro
    ^
```
A coluna é incrementada em seguida, mas o erro chama atenção com a formatação em ponteiro.