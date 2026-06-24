# Gramática da Linguagem — InterpretadorC

Especificação formal da gramática aceita pelo `InterpretadorC`, conforme implementada em `parserC.y` (Bison, `%glr-parser`) e `lexerC.l` (Flex).

## 1. Convenção de notação

- `não-terminal` → minúsculo
- `TERMINAL` → maiúsculo (token vindo do lexer)
- `A | B` → alternativa
- `[A]` → opcional
- `'x'` → literal exibido por conveniência (o lexer sempre devolve o token correspondente)

## 2. Tokens (léxico)

### 2.1 Palavras-chave

| Token | Lexema |
|---|---|
| `IF` | `if` |
| `ELSE` | `else` |
| `WHILE` | `while` |
| `FOR` | `for` |
| `RETURN` | `return` |
| `BREAK` | `break` |
| `VOID` | `void` |
| `INT` | `int` |
| `FLOAT` | `float` |
| `CHAR` | `char` |
| `BOOL` | `bool` |
| `PRINTF` | `printf` |
| `SCANF` | `scanf` |

### 2.2 Literais

| Token | Regra léxica | Exemplo |
|---|---|---|
| `INT_NUM` | `[0-9]+` | `42` |
| `FLOAT_NUM` | `[0-9]+\.[0-9]*` ou `\.[0-9]+` | `3.14`, `.5` |
| `CHAR_NUM` | `'[^']'` (exatamente 1 caractere) | `'a'` |
| `BOOL_VAL` | `true` \| `false` | `true` |
| `STRING_LITERAL` | `"(...)"` (com escapes `\"`, `\\` etc.) | `"texto"` |
| `ID` | `[a-zA-Z_][a-zA-Z0-9_]*` | `minhaVar` |

### 2.3 Operadores e pontuação

| Token | Lexema | Token | Lexema |
|---|---|---|---|
| `EQUAL` | `=` | `PONTO_VIRGULA` | `;` |
| `VIRGULA` | `,` | `OPEN_PAREN` / `CLOSE_PAREN` | `(` `)` |
| `ABRE_CHAVES` / `FECHA_CHAVES` | `{` `}` | `ABRE_COLCHETE` / `FECHA_COLCHETE` | `[` `]` |
| `PLUS` / `MINUS` | `+` `-` | `MULT` / `DIV` / `MOD` | `*` `/` `%` |
| `INCREMENT` / `DECREMENT` | `++` `--` | `ADD_EQUAL` ... `MOD_EQUAL` | `+= -= *= /= %=` |
| `AND` / `OR` / `NOT` | `&& \|\| !` | `AMP` | `&` |
| `DEQ` / `NEQ` | `== !=` | `LE` / `GE` / `LT` / `GT` | `<= >= < >` |

### 2.4 Outras regras léxicas

- Comentário de linha: `// até o fim da linha` (ignorado)
- Espaços e tabs: ignorados (contabilizados para a coluna do erro)
- Quebra de linha: incrementa o contador manual de linha, reseta coluna
- **Não existe** comentário de bloco (`/* */`)
- Caractere não reconhecido gera `ERROR LEXER` com linha/coluna apontadas; o caractere é descartado e o lexer continua

## 3. Precedência e associatividade (da menor para a maior)

```
%right  =  +=  -=  *=  /=  %=
%left   ||
%left   &&
%left   ==  !=
%left   <  >  <=  >=
%left   +  -
%left   *  /  %
%right  !  -(unário, %prec UMINUS)
%right  ++  --
```

- `&&` tem precedência maior que `||`; ambos têm precedência menor que os relacionais.
- O menos unário (`-x`) usa o pseudo-token `%prec UMINUS` para não conflitar com a subtração binária.
- A linha de atribuição (`%right EQUAL ...`) declara precedência para o analisador, mas **atribuição não é uma produção de `expressao`** — ela é sempre uma produção própria de `atribuicao`, terminada em `;` (ver seção 4.9). Ou seja, não existe `x = (y = 5);`.

## 4. Regras da gramática

### 4.1 Programa

```
programa
    : lista
```

Ao final do parse de `lista`: análise semântica → (se sem erros) impressão opcional de tabela de símbolos/TAC (modo `--debug`) → interpretação.

### 4.2 Lista de elementos de nível superior

```
lista
    : lista elemento
    | elemento

elemento
    : comando
    | def_funcao
```

Comandos comuns (incluindo declarações globais) e definições de função podem se intercalar livremente no nível mais externo do arquivo — **exceto** que uma função só pode ser chamada depois de já ter sido definida no texto (restrição semântica, não sintática).

### 4.3 Definição de função

```
def_funcao
    : tipo_ret ID OPEN_PAREN params_formais CLOSE_PAREN bloco
    | tipo_ret ID OPEN_PAREN CLOSE_PAREN bloco

tipo_ret
    : tipo
    | VOID

params_formais
    : param_formal
    | params_formais VIRGULA param_formal

param_formal
    : tipo ID
```

Exemplo:
```c
int soma(int a, int b) {
    return a + b;
}

void saudacao() {
    printf("oi\n");
}
```

### 4.4 Bloco e lista de comandos

```
bloco
    : ABRE_CHAVES comandos FECHA_CHAVES
    | ABRE_CHAVES FECHA_CHAVES

comandos
    : comandos comando
    | comando
```

Todo `bloco` define um novo escopo léxico. Um bloco vazio `{ }` é permitido.

### 4.5 Comandos

```
comando
    : expressao PONTO_VIRGULA
    | atribuicao
    | declaracao
    | bloco
    | error PONTO_VIRGULA                                  -- recuperação de erro sintático

    | IF OPEN_PAREN expressao CLOSE_PAREN bloco
    | IF OPEN_PAREN expressao CLOSE_PAREN bloco ELSE bloco

    | WHILE OPEN_PAREN expressao CLOSE_PAREN bloco

    | FOR OPEN_PAREN for_init expressao PONTO_VIRGULA incr_expr CLOSE_PAREN bloco

    | PRINTF OPEN_PAREN STRING_LITERAL CLOSE_PAREN PONTO_VIRGULA
    | PRINTF OPEN_PAREN STRING_LITERAL VIRGULA args_printf CLOSE_PAREN PONTO_VIRGULA
    | PRINTF OPEN_PAREN expressao CLOSE_PAREN PONTO_VIRGULA

    | RETURN expressao PONTO_VIRGULA
    | RETURN PONTO_VIRGULA

    | SCANF OPEN_PAREN STRING_LITERAL VIRGULA args_scanf CLOSE_PAREN PONTO_VIRGULA
    | SCANF OPEN_PAREN STRING_LITERAL CLOSE_PAREN PONTO_VIRGULA

    | BREAK PONTO_VIRGULA
```

Notas:

- **`if`/`while`/`for` exigem chaves** — a forma "sem bloco" do C tradicional (`if (x) y = 1;`) **não é aceita**.
- `printf` tem três formas: string fixa sem argumentos; string de formato com argumentos (`%d %i %f %g %c %s %%`); ou uma expressão única sem formatação (imprime o valor seguido de `\n` automático).
- `error PONTO_VIRGULA`: ao encontrar erro sintático dentro de um `comando`, o parser descarta tokens até o próximo `;` e continua analisando o restante do arquivo, em vez de abortar.

### 4.6 `for`: inicialização e incremento

```
for_init
    : declaracao
    | atribuicao
    | PONTO_VIRGULA      -- inicialização vazia

incr_expr
    : ID ADD_EQUAL  expressao
    | ID SUB_EQUAL  expressao
    | ID MULT_EQUAL expressao
    | ID DIV_EQUAL  expressao
    | ID MOD_EQUAL  expressao
    | ID INCREMENT
    | ID DECREMENT
    | INCREMENT ID
    | DECREMENT ID
    | expressao
    | /* vazio */
```

Como `declaracao` e `atribuicao` já consomem seu próprio `PONTO_VIRGULA`, a gramática do `for` não precisa de um `;` extra entre `for_init` e a condição:

```c
for (int i = 0; i < 10; i++) { ... }
for (;;) { ... }                       // init, cond e incr vazios
```

### 4.7 Argumentos de `printf`, chamada de função e `scanf`

```
args_printf
    : expressao
    | args_printf VIRGULA expressao

args_chamada
    : expressao
    | args_chamada VIRGULA expressao

args_scanf
    : AMP ID
    | ID
    | args_scanf VIRGULA AMP ID
    | args_scanf VIRGULA ID
```

`scanf` aceita tanto a forma "correta" em C (`&variavel`) quanto a forma simplificada sem `&` (`variavel`) — ambas têm efeito idêntico no interpretador (não há endereços reais).

### 4.8 Tipos e declarações

```
tipo
    : INT
    | FLOAT
    | CHAR
    | BOOL

declaracao
    : tipo lista_ids PONTO_VIRGULA
    | tipo ID ABRE_COLCHETE INT_NUM FECHA_COLCHETE PONTO_VIRGULA
    | tipo ID ABRE_COLCHETE INT_NUM FECHA_COLCHETE EQUAL ABRE_CHAVES lista_valores FECHA_CHAVES PONTO_VIRGULA

lista_valores
    : expressao
    | lista_valores VIRGULA expressao

lista_ids
    : ID
    | ID EQUAL expressao
    | lista_ids VIRGULA ID
    | lista_ids VIRGULA ID EQUAL expressao
```

Exemplos:
```c
int x;
float a, b = 2.0, c;
int v[5];
int notas[3] = {7, 8, 9};
```

`lista_ids` permite múltiplas variáveis do mesmo tipo na mesma declaração, cada uma com inicializador independente e opcional (o tipo declarado é propagado para todos os IDs da lista). O tamanho de um vetor (`INT_NUM`) precisa ser um literal inteiro — não pode ser uma expressão ou variável.

### 4.9 Atribuição

```
atribuicao
    : ID EQUAL      expressao PONTO_VIRGULA
    | ID ADD_EQUAL  expressao PONTO_VIRGULA
    | ID SUB_EQUAL  expressao PONTO_VIRGULA
    | ID MULT_EQUAL expressao PONTO_VIRGULA
    | ID DIV_EQUAL  expressao PONTO_VIRGULA
    | ID MOD_EQUAL  expressao PONTO_VIRGULA
    | INCREMENT ID PONTO_VIRGULA
    | DECREMENT ID PONTO_VIRGULA
    | ID INCREMENT PONTO_VIRGULA
    | ID DECREMENT PONTO_VIRGULA
    | ID EQUAL error PONTO_VIRGULA                          -- recuperação de erro
    | ID ABRE_COLCHETE expressao FECHA_COLCHETE EQUAL expressao PONTO_VIRGULA
```

`atribuicao` é uma produção **separada** de `expressao` — atribuição nunca aparece dentro de uma expressão maior (não existe `if ((x = f()) > 0)`); é sempre um comando completo, terminado em `;`.

### 4.10 Expressões

```
expressao
    : expressao PLUS  expressao
    | expressao MINUS expressao
    | expressao MULT  expressao
    | expressao DIV   expressao
    | expressao MOD   expressao
    | expressao DEQ   expressao
    | expressao NEQ   expressao
    | expressao LT    expressao
    | expressao GT    expressao
    | expressao LE    expressao
    | expressao GE    expressao
    | expressao AND   expressao
    | expressao OR    expressao
    | NOT expressao
    | MINUS expressao                      %prec UMINUS
    | OPEN_PAREN expressao CLOSE_PAREN
    | INT_NUM
    | FLOAT_NUM
    | CHAR_NUM
    | BOOL_VAL
    | ID OPEN_PAREN args_chamada CLOSE_PAREN
    | ID OPEN_PAREN CLOSE_PAREN
    | ID ABRE_COLCHETE expressao FECHA_COLCHETE
    | ID
```

A gramática de `expressao` é a única parte do parser que segue o estilo clássico de gramática de expressão recursiva com precedência resolvida via `%left`/`%right` (seção 3), em vez de produções explicitamente estratificadas por nível.

## 5. Visão geral simplificada da derivação

```
programa
 └─ lista
     ├─ elemento → def_funcao → tipo_ret ID ( params ) bloco
     │                                              └─ comandos*
     └─ elemento → comando
                     ├─ declaracao
                     ├─ atribuicao
                     ├─ bloco { comandos* }
                     ├─ if (expressao) bloco [else bloco]
                     ├─ while (expressao) bloco
                     ├─ for (for_init expressao ; incr_expr) bloco
                     ├─ printf(...) / scanf(...)
                     ├─ return [expressao] ;
                     └─ break ;
```

## 6. Ambiguidade conhecida da gramática

Existe **um conflito shift/reduce real** (confirmado com `bison -Wcounterexamples`), no token `ID` imediatamente após `tipo`:

- Ramo A (declaração): `tipo • ID ABRE_COLCHETE ...` / `tipo • ID PONTO_VIRGULA` / `tipo • ID EQUAL ...`
- Ramo B (função): `tipo_ret • ID OPEN_PAREN params_formais CLOSE_PAREN bloco`

Ambas as produções compartilham o prefixo `tipo ID`, e a decisão correta só pode ser tomada **um token depois do `ID`** (se vier `(`, é função; senão, é declaração). Por isso o parser usa `%glr-parser`: ele mantém os dois ramos vivos brevemente e descarta o errado assim que o próximo token desambiguar. É uma ambiguidade genuína da gramática (LALR(1) puro não resolveria com 1 token de lookahead nesse ponto), não um erro de especificação.

## 7. O que a gramática **não** aceita

- `if`/`while`/`for` sem chaves (`{}` é obrigatório)
- Atribuição dentro de expressão (`x = (y = 5);`)
- Vetores multidimensionais ou vetor como parâmetro de função
- `switch`, `do...while`, `continue`
- Comentários de bloco `/* */`
- Diretivas de pré-processador (`#include`, `#define`)
- Declaração de função sem corpo (protótipo) — toda `def_funcao` exige `bloco`
- Ponteiros, `struct`, `union`, `enum`, tipos definidos pelo usuário