# Análise Léxica — InterpretadorC

Especificação do módulo `lexerC.l` (Flex): como o código-fonte é convertido em tokens, como o valor semântico de cada token chega ao parser, como o rastreamento de linha/coluna funciona para as mensagens de erro, e quais particularidades/limitações conhecidas existem na implementação atual.

## 1. Papel do lexer no pipeline

```
código-fonte (.c)
      │
      ▼
  lexerC.l (Flex) ──── yylex() é chamado repetidamente pelo parser (Bison)
      │
      │  para cada chamada: devolve um TOKEN (int) e, se aplicável,
      │  preenche yylval com o valor semântico associado
      ▼
  parserC.y (Bison)
```

O lexer não constrói nenhuma estrutura de dados própria — sua única responsabilidade é reconhecer o próximo lexema do fluxo de caracteres, devolver o token correspondente, e (quando o token carrega um valor) preencher a união `yylval` declarada em `parserC.y`. O lexer também mantém, paralelamente, um rastreamento manual de linha/coluna usado pelas mensagens de erro léxico e sintático.

## 2. Integração com o parser: `yylval` e tipos de token

A união semântica é declarada no `%union` de `parserC.y`:

```c
%union {
    int    intValue;
    float  floatValue;
    char  *id;
    NoAST *ast;
    Param *param;
}
```

O lexer só usa três desses campos (os outros dois — `ast` e `param` — são preenchidos pelas ações do próprio parser, não pelo lexer):

| Token | Campo de `yylval` preenchido | Como é calculado |
|---|---|---|
| `INT_NUM` | `intValue` | `atoi(yytext)` |
| `FLOAT_NUM` | `floatValue` | `atof(yytext)` |
| `CHAR_NUM` | `intValue` | `yytext[1]` (o caractere entre as aspas simples, como `int`) |
| `BOOL_VAL` | `intValue` | `1` para `true`, `0` para `false` |
| `ID` | `id` | `strdup(yytext)` (string alocada dinamicamente — liberada depois pelas ações do parser com `free()`) |
| `STRING_LITERAL` | `id` | cópia do conteúdo **sem as aspas externas** (alocada com `malloc`) |

Todos os demais tokens (palavras-chave, operadores, pontuação) não carregam valor semântico — só o próprio código do token importa.

## 3. Regras léxicas, por categoria

### 3.1 Controle de linha, espaço e comentário
```
\n        → incrementa linha, reseta coluna e o buffer de exibição (ver seção 4)
"//".*    → comentário de linha até o fim da linha (descartado)
[ \t]+    → espaços e tabs (descartados, mas contam para a coluna)
```
**Não existe comentário de bloco (`/* */`)** — não há regra alguma para isso na especificação.

### 3.2 Palavras-chave
```
if else while for return break void
int float char bool
printf scanf
true false
```
Cada palavra-chave tem sua própria regra literal (`"if"`, `"while"`, etc.), todas declaradas **antes** da regra genérica de identificador (`[a-zA-Z_][a-zA-Z0-9_]*`). Isso é o que garante que `if`, `while` etc. sejam reconhecidas como palavras-chave e não como nomes de variável — ver seção 5.1 sobre como o Flex decide isso.

### 3.3 Literais
```
\"(([^\"\\]|\\.)*)\"          → STRING_LITERAL (aceita \" e \\ escapados dentro da string)
([0-9]+\.[0-9]*|\.[0-9]+)     → FLOAT_NUM       (cobre "3.14", "3." e ".5")
[0-9]+                        → INT_NUM
[a-zA-Z_][a-zA-Z0-9_]*        → ID (ou palavra-chave, se bater com uma regra mais específica)
\'[^\']\'                     → CHAR_NUM (exatamente 1 caractere entre aspas simples — '\n' como char de 2 bytes NÃO é aceito)
```

### 3.4 Operadores e pontuação
```
== != <= >= < =          (relacionais e atribuição simples)
++ -- += -= *= /= %=     (incremento/decremento e atribuição composta)
+ - * / %                (aritméticos)
! && || &                (lógicos e "endereço")
; ( ) { } [ ] ,          (pontuação estrutural)
```
Os operadores de 2 caracteres (`==`, `!=`, `<=`, `>=`, `++`, `--`, `+=`, `-=`, `*=`, `/=`, `%=`, `&&`, `||`) estão todos declarados **antes** dos seus prefixos de 1 caractere (`=`, `<`, `>`, `+`, `-`, `*`, `/`, `%`, `&`, `|`) — isso não é coincidência: é necessário para o "maximal munch" funcionar corretamente (ver seção 5.1).

### 3.5 Caractere inválido
```
.   → ERROR LEXER (qualquer caractere que não bateu com nenhuma regra anterior)
```
Essa é a regra de "captura geral": qualquer símbolo não coberto pelas regras acima (ex.: `@`, `$`, `?`, `~`, `^` fora de contexto) cai aqui. O caractere é simplesmente descartado e o lexer continua a partir do próximo.

## 4. Rastreamento de linha/coluna para mensagens de erro

Três variáveis globais, mantidas manualmente (não usam o suporte nativo do Flex):

```c
int  linha   = 1;          // linha atual (1-indexada)
int  coluna  = 1;          // coluna atual (1-indexada)
char linhaAtual[1024];     // buffer com o texto da linha atual, para exibir no erro
int  posLinha = 0;         // posição de escrita dentro de linhaAtual
```

A macro `ADD_TEXTO()` é o mecanismo usado para ir acumulando o texto da linha atual em `linhaAtual`, para que as mensagens de erro possam mostrar a linha inteira com um `^` apontando a coluna do problema:

```c
#define ADD_TEXTO() \
    if (posLinha + yyleng < 1023) { \
        strncpy(&linhaAtual[posLinha], yytext, yyleng); \
        posLinha += yyleng; \
        linhaAtual[posLinha] = '\0'; \
    }
```

`coluna` é incrementada por `yyleng` em **toda** regra que reconhece um token (inclusive espaços e comentários) — isso funciona de forma consistente e correta em todas as regras.

### ⚠️ 4.1 Bug conhecido: `linhaAtual` fica incompleto

`ADD_TEXTO()` **não é chamada por todas as regras** — só por estas:
```
\n, "//".*, [ \t]+, "int", "=", ";",
FLOAT_NUM, INT_NUM, ID, CHAR_NUM
```

Ou seja, **nenhuma outra palavra-chave** (`if else while for return break void printf scanf float char bool true false`) e **nenhum outro operador/pontuação** (`== != <= >= < > ++ -- += -= *= /= %= + - * / % ! && || & ( ) { } [ ] ,`) chama `ADD_TEXTO()`. O texto desses tokens nunca é copiado para `linhaAtual` — mas a `coluna` continua avançando normalmente para eles.

**Consequência prática:** a linha de contexto impressa numa mensagem de erro (léxico ou sintático) frequentemente **não reflete o conteúdo real da linha**, tendo "buracos" onde havia palavras-chave ou operadores, mesmo que o `^` aponte para a coluna numericamente correta (que pode acabar apontando bem depois do fim do texto exibido). Exemplo reproduzido:

```c
if (x == 1) @
```
produz:
```
ERROR LEXER
Line: 3 Column: 17 | error: invalid character '@'
         x  1 
                    ^
```
(`if`, `(`, `==`, `)` desaparecem da linha mostrada — só `x` e `1`, que vieram de regras com `ADD_TEXTO`, aparecem.)

### ⚠️ 4.2 Bug conhecido: buffer de linha não é limpo na quebra de linha

A regra de `\n` faz:
```c
\n {
    linhaAtual[posLinha] = '\0';   // apenas TRUNCA no índice atual
    linha++;
    coluna  = 1;
    posLinha = 0;                  // reseta o índice de escrita...
                                    // ...mas NÃO zera o conteúdo do buffer
}
```
Como o conteúdo de `linhaAtual` **não é apagado**, só o índice de escrita é reiniciado, se um erro (léxico ou sintático) ocorrer em uma nova linha **antes** que qualquer token cuja regra chame `ADD_TEXTO()` tenha sido lido nessa linha, a mensagem de erro mostra o **conteúdo residual da linha anterior**, não da linha atual. Reprodução:
```c
if (x == 1) @
}
```
```
ERROR LEXER
Line: 3 Column: 17 | error: invalid character '@'
         x  1 
                    ^
ERROR PARSER
Line: 4 Column: 2 | error: unexpected token: '}'
         x  1            ← deveria ser o conteúdo da linha 4, mas mostra a linha 3 (stale)
     ^
```
**Importante para quem for projetar testes em torno de mensagens de erro:** o número de `Line:`/`Column:` em si está sempre correto; é só o **texto de contexto exibido abaixo da mensagem** que pode estar incompleto ou desatualizado. Se o teste comparar a saída inteira (incluindo essas linhas de contexto), seja cauteloso ao prever esse texto — ele depende exatamente de quais tokens já passaram por `ADD_TEXTO()` desde o último `\n`.

## 5. Outras particularidades

### 5.1 Resolução de ambiguidade léxica (maximal munch)
O Flex resolve automaticamente qual regra usar quando mais de uma poderia bater com a entrada, segundo duas regras clássicas de geração de scanners:
1. **A regra que casa o lexema mais longo ganha.** É por isso que `"<="` precisa estar declarada (ou simplesmente "existir" na tabela de regras) e é reconhecida como um único token `LE`, em vez de `LT` seguido de `EQUAL` — o Flex sempre tenta estender o match o quanto for possível.
2. **Em caso de empate de tamanho, a regra declarada primeiro no arquivo ganha.** É por isso que `"if"` (declarada antes da regra genérica de `ID`) é reconhecida como a palavra-chave `IF`, e não como um identificador comum de 2 letras — mesmo que ambas as regras casassem exatamente "if" com o mesmo comprimento.

### 5.2 Strings: aspas removidas, escapes **não** interpretados no lexer
A regra de `STRING_LITERAL` remove as aspas externas mas **mantém a barra invertida bruta** (`\n`, `\t` etc. continuam como dois caracteres literais — backslash + letra — dentro da string armazenada). A **interpretação** dessas sequências de escape (convertendo `\n` em quebra de linha real, `\t` em tab etc.) só acontece depois, no momento de **imprimir** a string formatada (em `interpreter.c`, ao processar `printf`/`scanf` com formato) — não faz parte do trabalho do lexer.

### 5.3 Char literal: só 1 byte, sem suporte a escape
`\'[^\']\'` aceita **exatamente um caractere** entre aspas simples. Isso significa que `'\n'` (que em C tem 2 caracteres entre as aspas: barra invertida e `n`) **não bate com essa regra** — na prática, um char-literal de escape como `'\n'` cairia na regra de caractere inválido pra o segundo caractere, ou produziria comportamento inesperado, dependendo de como o conteúdo é fatiado. Evite usar escapes dentro de literais `char` ao escrever testes; use apenas caracteres literais simples como `'a'`, `'Z'`, `'5'`.

### 5.4 Regra duplicada (`"&"`)
A regra `"&" { coluna += yyleng; return AMP; }` aparecia duplicada no arquivo original (uma vez antes da regra de identificadores, outra depois — esta segunda ocorrência era inalcançável e o próprio `flex` avisa isso como "regra não casada" / "rule cannot be matched"). Trata-se de um resquício de edição sem efeito funcional; já foi identificado e a remoção da ocorrência redundante foi sugerida anteriormente (ver histórico de correções do projeto).

### 5.5 `yylineno` declarado, mas nunca incrementado
O arquivo não declara `%option yylineno`. Ainda assim, o Flex gera (no `.c` resultante) uma variável global `yylineno` inicializada em `1`, simplesmente porque outro módulo do projeto (`ast.c`, para a mensagem de divisão por zero detectada em literais) faz `extern int yylineno;` e a referencia. Como a opção que faria o Flex incrementá-la automaticamente a cada `\n` não foi habilitada, **`yylineno` permanece `1` durante toda a execução do programa**. O contador de linha que de fato funciona corretamente (usado em todas as mensagens de `ERROR LEXER` e `ERROR PARSER`) é a variável manual `linha`, mantida só dentro deste arquivo — são dois mecanismos de contagem de linha completamente independentes, e só um deles funciona. Esse é a causa raiz do problema, documentado em outros arquivos deste projeto, de a mensagem "divisao por zero na linha X" sempre reportar `X = 1`.

### 5.6 `yywrap`
```c
int yywrap() { return 1; }
```
Implementação trivial que sinaliza "fim de entrada, não há mais arquivos para encadear" — o interpretador sempre processa um único arquivo de entrada por execução (não há suporte a múltiplos arquivos/`#include`).

## 6. Resumo para quem for escrever testes

1. O **número** de linha/coluna nas mensagens `ERROR LEXER`/`ERROR PARSER` é confiável.
2. O **texto de contexto exibido** nessas mensagens pode estar incompleto (faltando palavras-chave/operadores) ou desatualizado (conteúdo de uma linha anterior) — não assuma que ele reflete fielmente a linha real, especialmente em casos de erro logo após `if`, `while`, operadores relacionais/lógicos, ou no início de uma nova linha.
3. A mensagem de "divisão por zero" entre literais (gerada em `ast.c`, não aqui) sempre relata linha `1`, por causa do `yylineno` nunca incrementado — não é um problema deste arquivo especificamente, mas a causa está aqui.
4. Não use escapes (`'\n'`) dentro de literais `char` — só caracteres simples.
5. Comentários só na forma `// linha`; `/* bloco */` não é reconhecido (cairia em erro léxico em `/` ou `*` isoladamente, dependendo do contexto).