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
* `linhaAtual`: conteúdo da linha (para debug)

---

## 3. Tokens Reconhecidos

### 3.1 Palavras-chave

| Palavra | Token  |
| ------- | ------ |
| if      | IF     |
| else    | ELSE   |
| int     | INT    |
| float   | FLOAT  |
| char    | CHAR   |
| bool    | BOOL   |
| for     | FOR    |
| return  | RETURN |
| break   | BREAK  |
| void    | VOID   |
| printf  | PRINTF |
| scanf   | SCANF  |

---

### 3.2 Identificadores

```text
[a-zA-Z][a-zA-Z0-9]*
```

* Devem iniciar com letra
* Podem conter números

---

### 3.3 Literais

| Tipo    | Exemplo | Token     |
| ------- | ------- | --------- |
| Inteiro | 10      | INT_NUM   |
| Float   | 10.5    | FLOAT_NUM |
| Char    | 'a'     | CHAR_NUM  |

---

### 3.4 Operadores

#### Aritméticos

`+ - * / %`

#### Relacionais

`== != < > <= >=`

#### Lógicos

`&& || !`

#### Atribuição

`= += -= *= /= %=`

#### Incremento

`++ --`

---

## 4. Comentários

```c
// comentário
```

* Ignorados
* Mantidos no buffer para exibição de erro

---

## 5. Controle de Posição

Cada token:

* Atualiza coluna
* Armazena texto na linha

Macro:

```c
ADD_TEXTO()
```

---

## 6. Integração com Parser

Valores enviados via:

```c
yylval
```

Tipos:

* `int`
* `float`
* `char`
* `string` (ID)

---

## 7. Tratamento de Erros

```c
. { erro }
```

Saída:

```
ERROR LEXER
Line X Column Y
    código
        ^
```

---

## 8. Fim de Arquivo

```c
int yywrap() {
    return 1;
}
```

---
