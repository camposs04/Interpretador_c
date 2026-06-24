# InterpretadorC — Escopo Completo da Linguagem e Arquitetura

> Este documento tem como objetivo permitir que alguém **sem acesso ao código-fonte** consiga escrever programas de teste válidos (e inválidos, propositalmente) e prever com exatidão a saída esperada do interpretador — incluindo mensagens de erro, formato de impressão de valores e comportamento em casos de borda.

## 1. Visão geral e pipeline

O `InterpretadorC` interpreta diretamente um subconjunto de C (não gera binário). O fluxo de processamento de um arquivo-fonte é:

```
.c  →  Lexer (Flex)  →  Parser (Bison, %glr-parser)  →  AST
                                                          │
                                  ┌───────────────────────┴────────────────────┐
                                  ▼                                            ▼
                     Análise Semântica (sempre executa)         Geração de TAC (só com --debug)
                                  │
                       erros > 0? ──── sim ──→ aborta (não executa nada)
                                  │
                                 não
                                  │
                                  ▼
                  Interpretação direta da AST (executar/avaliar)
```

Pontos cruciais para quem for escrever testes:

- **Se houver qualquer erro semântico, o programa NÃO é executado.** A saída nesse caso é só as mensagens de erro semântico mais a contagem final — nenhuma saída de `printf` do programa aparece.
- Erros de léxico e de sintaxe são reportados mas **não impedem necessariamente a interpretação do restante** do arquivo (o parser tem recuperação de erro em `;` — ver seção 8). Erros léxicos isolados também não abortam a leitura do arquivo.
- A interpretação acontece **direto sobre a AST**, nunca sobre o TAC. O TAC é só uma representação textual auxiliar impressa com `--debug`/`-d`; gerar TAC errado nunca afeta o resultado da execução real.
- O `main()` do programa-fonte só é localizado e chamado **depois** que todo o código de nível superior (declarações globais e definições de função) já foi executado/registrado.

## 2. Como invocar (para gerar os `.expected`)

```sh
./programa arquivo.c            # execução normal
./programa -d arquivo.c         # com tabela de símbolos + TAC + rastros de execução
./programa --debug arquivo.c    # idem (forma longa)
```

Sem nenhum argumento de arquivo, o `yyin` não é redirecionado e o lexer lê de **stdin**.

## 3. Tipos primitivos e como cada um é impresso

| Tipo | Literais aceitos | Formato em `printf(expr)` simples | Formato em `printf("%X", expr)` |
|---|---|---|---|
| `int` | `42`, `0`, `-7` (via menos unário) | `%d` | `%d` ou `%i` |
| `float` | `3.14`, `.5`, `5.` | `%g` (**não** `%f`!) | `%f` (padrão `printf` do C, 6 decimais) ou `%g` |
| `char` | `'a'` (exatamente 1 caractere entre aspas simples) | `%c` | `%c` |
| `bool` | `true`, `false` | `true`/`false` (texto) | `%s` → `true`/`false`; **qualquer outro especificador (`%d`, `%i`) imprime `0` ou `1` numérico** |
| `void` | — (só como tipo de retorno de função, nunca de variável) | — | — |

⚠️ **Armadilha para testes:** `printf("%s", minhaBool)` imprime `true`/`false`, mas `printf("%s", minhaIntOuOutroTipo)` **não imprime uma string** — o interpretador trata `%s` em valores não-`bool` como se fosse `%d` (limitação conhecida: não existe tipo string-variável na linguagem, só literais de formato).

### 3.1 Promoção de tipo em expressões aritméticas
- `int OP int` → `int` (divisão `/` é inteira, truncada)
- Se **qualquer um** dos operandos de `+ - * /` for `float`, o resultado é `float` (o outro lado é promovido)
- `%` (módulo) é sempre tratado como operação inteira (usa `.dado.i` dos dois lados, mesmo que o tipo declarado seja outro)
- Comparações (`== != < > <= >=`) sempre resultam em `bool`, comparando os dois lados como `float` internamente (sem perda perceptível para valores inteiros pequenos)

### 3.2 Conversão em atribuição
Atribuir um valor de um tipo a uma variável de outro tipo **converte automaticamente** (não há erro de tipo incompatível em atribuição):
- `int → float`: conversão numérica direta
- `float → int`: truncamento (não arredonda)
- `qualquer → bool`: `0`/`0.0` → `false`; qualquer outro valor → `true`
- `qualquer → char`: trunca para o byte baixo do valor numérico

```c
int x = 3.9;     // x vira 3 (truncado, sem erro)
bool b = 5;      // b vira true
float f = 2;     // f vira 2.0
```

## 4. Declarações de variável

```c
int x;                 // não inicializada → valor padrão 0
float y = 1.5;
float a, b = 2.0, c;   // lista: tipo propagado a todos, inicializadores independentes e opcionais
```

- Variável não inicializada começa com valor numérico `0` (equivalente a `0`, `0.0`, `'\0'` ou `false` dependendo do tipo).
- **Redeclaração no mesmo escopo é erro semântico**, mesmo que o tipo seja diferente:
  ```
  Erro Semantico: variavel 'x' ja declarada neste escopo.
  ```
- **Uso antes da declaração** (ou em escopo que não alcança a declaração) é erro:
  ```
  Erro Semantico: variavel 'x' nao declarada.
  ```
- Sombreamento (mesmo nome em escopo interno) é permitido e não gera erro — ver seção 7 sobre quais construções abrem escopo novo.

## 5. Vetores

```c
int v[5];                       // todos os elementos = 0
int notas[3] = {7, 8, 9};       // inicialização completa
int parcial[5] = {1, 2};        // só os 2 primeiros definidos; o resto fica 0
```

Regras e erros:
- O tamanho **precisa ser um literal inteiro** (`INT_NUM`), nunca uma variável ou expressão: `int v[n];` com `n` variável é erro de sintaxe, não de semântica.
- Tamanho `0` ou negativo:
  ```
  Erro Semantico: vetor 'v' deve ter tamanho positivo.
  ```
- Mais valores de inicialização do que o tamanho declarado:
  ```
  Erro Semantico: vetor 'v' tem 4 valor(es) inicial(is) para 3 posicao(oes).
  ```
  (Menos valores que o tamanho **não é erro** — o restante fica `0`.)
- Acesso (`v[i]`) ou atribuição (`v[i] = x;`) a um nome que não é vetor declarado:
  ```
  Erro Semantico: 'v' nao e um vetor declarado.
  ```
- Índice fora dos limites **em tempo de execução** (não é detectado na análise semântica, já que o índice pode ser uma expressão dinâmica):
  ```
  Erro RT: indice 7 fora dos limites de 'v' (tamanho 5).
  ```
  Esse erro **não aborta a execução** — o programa continua, e a expressão de leitura retorna `0` (ou a escrita é simplesmente ignorada).
- **Não há vetores multidimensionais nem vetor como parâmetro de função.**

## 6. Funções

```c
int soma(int a, int b) {
    return a + b;
}

void saudacao() {
    printf("oi\n");
}
```

- Tipo de retorno: `int`, `float`, `char`, `bool` ou `void`.
- Parâmetros são sempre escalares (`tipo nome`); não há parâmetro vetor.
- **Recursão simples funciona normalmente** (ex.: Fibonacci recursivo).
- **Funções precisam estar definidas antes de serem chamadas no texto-fonte.** Não há pré-varredura de assinaturas, então isto **falha**:
  ```c
  int a() { return b() + 1; }   // erro: 'b' ainda não foi registrada
  int b() { return 1; }
  ```
  ```
  Erro Semantico: funcao 'b' nao declarada.
  ```
  Inverter a ordem de definição resolve. **Não há suporte a recursão mútua** entre duas funções diferentes.
- Função redefinida (mesmo nome usado em dois `def_funcao`):
  ```
  Erro Semantico: funcao 'nome' ja declarada.
  ```
- Chamada com número de argumentos diferente do número de parâmetros:
  ```
  Erro Semantico: funcao 'nome' espera 2 argumento(s), recebeu 1.
  ```
  (Não há checagem de **tipo** dos argumentos — só de quantidade; os valores passados são convertidos automaticamente para o tipo do parâmetro, igual à conversão de atribuição da seção 3.2.)
- Usar o retorno de uma função `void` como se fosse um valor (em atribuição, em outra expressão, como argumento, como valor de inicialização de vetor etc.):
  ```
  Erro Semantico: funcao 'nome' tem retorno void e nao pode ser usada como valor.
  ```
- `return` incompatível com a assinatura da função:
  - função `void` com `return expr;` →
    ```
    Erro Semantico: funcao void nao pode retornar um valor.
    ```
  - função não-`void` com `return;` (sem valor) →
    ```
    Erro Semantico: funcao do tipo 'int' deve retornar um valor.
    ```
    (o nome do tipo no texto varia: `int`, `float`, `char`, `bool`)
  - `return` fora de qualquer função (no nível global do arquivo) **não gera erro** — é simplesmente ignorado/tolerado.
- Chamada de uma função que não existe:
  ```
  Erro Semantico: funcao 'nome' nao declarada.
  ```

## 7. Escopo: onde um novo escopo é aberto

Um novo escopo (tanto para análise semântica quanto em tempo de execução) é criado exatamente nestes pontos:

| Construção | Abre escopo novo? |
|---|---|
| Corpo de função (parâmetros + corpo) | Sim — parâmetros entram nesse escopo |
| Corpo de `if` (ramo `then`) | Sim |
| Corpo de `else` | Sim (escopo separado do `then`) |
| Corpo de `while` | Sim (um novo escopo **por iteração** em tempo de execução) |
| Corpo de `for` | Sim, em dois níveis: um escopo externo para `init`/condição/incremento, e um interno por iteração para o corpo |
| Bloco solto `{ ... }` usado como comando autônomo (não ligado a `if`/`while`/`for`) | Sim |
| Nível global do arquivo | É o escopo raiz, nunca fechado durante a execução |

```c
void main() {
    int x = 5;
    {
        int x = 10;             // sombra válida — escopo próprio do bloco solto
        printf("%d\n", x);      // imprime 10
    }
    printf("%d\n", x);          // imprime 5 — variável externa intacta
}
```

## 8. Tratamento de erros: léxico, sintático, semântico e de execução

Há **quatro categorias** de erro, cada uma com formato de mensagem fixo. É importante saber qual delas se aplica a cada situação de teste, porque elas têm consequências diferentes sobre se o programa chega a executar.

### 8.1 Erro léxico
Disparado por qualquer caractere não reconhecido pelo lexer (ex.: `@`, `$`, `?` fora de contexto válido). Formato exato:
```
ERROR LEXER
Line: <L> Column: <C> | error: invalid character '<ch>'
    <conteúdo da linha até aquele ponto>
    ^
```
O caractere é descartado e a leitura continua — **não interrompe o parsing do restante do arquivo**.

### 8.2 Erro sintático
Disparado pelo parser ao encontrar um token inesperado. Formato exato:
```
ERROR PARSER
Line: <L> Column: <C> | error: unexpected token: '<token>'
    <conteúdo da linha>
    ^

```
Há recuperação de erro em dois pontos da gramática (`error PONTO_VIRGULA`): tanto um `comando` quanto o lado direito de uma atribuição (`ID EQUAL error PONTO_VIRGULA`) podem ser "engolidos" até o próximo `;`, permitindo que o restante do arquivo seja parseado. Isso significa que **um erro de sintaxe isolado, seguido de `;`, não necessariamente impede a execução do resto do programa** (mas o comando malformado em si é descartado, como se fosse `NULL`).

### 8.3 Erro semântico
Formato: `Erro Semantico: <descrição>.` (ver listagem completa nas seções 4–6). Ao final, se `errosSemanticos() > 0`:
```
<contagem> erro(s) semantico(s) encontrado(s).
```
(impresso em `stderr`). **Qualquer erro semântico impede a interpretação do programa inteiro.**

⚠️ **Caso especial — divisão por zero detectada em "tempo de compilação":** se uma divisão (`/`) ou módulo (apenas `/`, não `%`) acontecer entre **dois literais constantes** (ex.: `5 / 0`, `3.0 / 0.0`), o *constant folding* da construção da AST já detecta isso **antes mesmo da análise semântica formal** e conta como erro semântico:
```
Erro Semantico: divisao por zero na linha 1.
```
**Atenção:** por uma particularidade da implementação (o contador de linha do Bison/Flex não está habilitado), **esse número de linha específico sempre será `1`, independentemente de onde a divisão por zero literal realmente esteja no arquivo.** Isso só afeta esta mensagem específica — as mensagens de erro léxico e sintático (seções 8.1/8.2) usam um contador de linha manual e reportam a linha correta normalmente.

### 8.4 Erro de execução ("RT" = runtime)
Formato: `Erro RT: <descrição>.` Acontece **apenas depois** que a análise semântica passou sem erros (ou seja, nunca aparece junto com erros semânticos no mesmo programa). Listagem completa:

| Situação | Mensagem |
|---|---|
| Variável usada não encontrada em tempo de execução | `Erro RT: variavel 'x' nao encontrada.` |
| Vetor não encontrado | `Erro RT: vetor 'v' nao encontrado.` |
| Índice de vetor fora dos limites | `Erro RT: indice 7 fora dos limites de 'v' (tamanho 5).` |
| Divisão por zero (operando não-literal, ex. variável) | `Erro RT: divisao por zero.` |
| Módulo por zero | `Erro RT: modulo por zero.` |
| Chamada de função não encontrada em tempo de execução | `Erro RT: funcao 'f' nao encontrada.` |
| Variável de `scanf` não encontrada | `Erro RT: variavel nao encontrada no scanf.` (via `stderr`) |
| Especificador de `scanf` não suportado (qualquer um além de `%d %i %f %c`) | `Aviso: especificador '%X' nao suportado no scanf.` (via `stderr`) |

**Nenhum desses erros de execução aborta o programa** — a expressão problemática simplesmente retorna/usa o valor padrão `0` (ou a instrução é ignorada, no caso de atribuição a índice inválido) e a execução continua normalmente a partir do próximo comando.

## 9. Comandos de entrada/saída

### 9.1 `printf` — três formas
```c
printf("texto fixo\n");                    // sem formatação, sem args
printf("valor: %d, %f\n", x, y);           // com formato e argumentos
printf(expr);                              // forma "crua": imprime o valor + \n automático
```
- Especificadores suportados na forma com formato: `%d` `%i` (inteiro), `%f` (float, formato `printf` padrão do C), `%g` (float compacto), `%c` (char), `%s` (só funciona corretamente para `bool`; ver seção 3), `%%` (escapa um `%` literal).
- Sequências de escape reconhecidas na string de formato: `\n` `\t` `\r` `\\` `\"`. Qualquer outra barra invertida seguida de caractere é impressa literalmente (`\` + caractere).
- Se houver mais especificadores `%` do que argumentos fornecidos, o especificador excedente é impresso literalmente (ex.: `%d` vira o texto `%d`).
- A forma `printf(expr)` (sem aspas) sempre adiciona uma quebra de linha ao final automaticamente.

### 9.2 `scanf`
```c
scanf("%d", &x);
scanf("%d %f", &x, y);     // o '&' é opcional/estilístico — não há diferença de comportamento
```
- Especificadores suportados: `%d` `%i` (int), `%f` (float), `%c` (char). **`%s` não é suportado** (gera aviso em `stderr`, não preenche nada).
- A leitura é feita diretamente do terminal (`/dev/tty`), então **funciona mesmo que a entrada padrão (`stdin`) do processo tenha sido redirecionada de um arquivo** — importante saber se os testes de `scanf` tentarem usar redirecionamento (`< entrada.txt`): isso **não vai funcionar** como entrada para o `scanf` deste interpretador, pois ele ignora `stdin` e lê do `/dev/tty` sempre que possível.

## 10. Estruturas de controle

```c
if (cond) { ... }
if (cond) { ... } else { ... }

while (cond) { ... }

for (init; cond; incr) { ... }
for (; ; ) { ... }              // os três campos podem ser vazios

break;                          // só dentro de while/for
```

- **Chaves são obrigatórias** em `if`, `while`, `for` e `else` — não existe a forma de uma linha sem `{}` do C tradicional.
- `break` fora de qualquer laço:
  ```
  Erro Semantico: 'break' usado fora de um laco.
  ```
- `break` interrompe **apenas o laço mais interno** (não há `break` com rótulo nem `continue`).
- `for`: o campo de inicialização aceita uma `declaracao` (com seu próprio `;`), uma `atribuicao` (idem), ou nada (só o `;`); a condição é uma `expressao` qualquer; o incremento aceita atribuição composta, `++`/`--` (prefixo ou sufixo), uma expressão qualquer (avaliada e descartada) ou nada.
- Não existem `switch/case`, `do...while` nem `continue`.

## 11. Operadores e expressões — tabela de referência rápida

| Categoria | Operadores | Observação |
|---|---|---|
| Aritméticos | `+ - * / %` | `/` é inteira entre dois `int`; `%` sempre inteiro |
| Relacionais | `== != < > <= >=` | resultado sempre `bool` |
| Lógicos | `&& \|\| !` | curto-circuito real (o lado direito nem é avaliado se desnecessário) |
| Atribuição composta | `+= -= *= /= %=` | só como comando — não retornam valor utilizável em expressão |
| Incremento/decremento | `++x` `x++` `--x` `x--` | prefixo e sufixo têm efeito idêntico nesta implementação (não há diferença de valor retornado, pois sempre são usados como comando, nunca como sub-expressão) |
| Unários | `-x` (menos), `!x` (negação lógica) | |
| Vetor | `v[i]` | leitura (expressão) e escrita (`v[i] = x;`, comando) |
| Chamada de função | `f(a, b)` | como expressão (em qualquer lugar que aceite `expressao`) ou como comando isolado (`f(a, b);`) |
| Parênteses | `(expr)` | só para agrupamento de precedência — **atribuição não pode aparecer dentro de parênteses como sub-expressão** |

Precedência (da menor para a maior): `= += -= *= /= %=` < `||` < `&&` < `== !=` < `< > <= >=` < `+ -` < `* / %` < `! -(unário)` < `++ --`.

## 12. Otimizações em tempo de "compilação" (constant folding)

A construção da AST já simplifica certas expressões **antes** da análise semântica, o que pode ser relevante para prever comportamento de testes que combinem literais:

- Operações aritméticas/relacionais entre dois literais do mesmo tipo (`int×int` ou `float×float`) ou mistos (`int×float`) são calculadas direto, virando um único nó literal.
- Identidades algébricas com um literal neutro são simplificadas mesmo quando o outro operando **não** é literal: `x + 0 → x`, `0 + x → x`, `x - 0 → x`, `x * 1 → x`, `1 * x → x`, `x * 0 → 0`, `0 * x → 0`, `x / 1 → x`.
- `true && x → x`, `false && x → false`, `x && true → x`, `x && false → false` (e o equivalente para `||`).
- `!!x → x` (dupla negação lógica), `-(-x) → x` (dupla negação aritmética).
- Divisão/módulo entre dois literais com divisor `0` é capturada **nesse momento** como erro semântico (ver seção 8.3, incluindo o detalhe do número de linha sempre `1`). Divisão por zero envolvendo qualquer variável só é detectada em tempo de execução (seção 8.4).

Essas simplificações são transparentes para o resultado final (não mudam o valor calculado), mas explicam por que, por exemplo, `printf(x * 0);` nunca gera um erro de "variável não usada" mesmo que `x` nunca seja avaliada de fato em tempo de execução.

## 13. O que **não** é aceito pela linguagem

- Ponteiros, `struct`, `union`, `enum`, tipos definidos pelo usuário (`typedef`)
- Vetores multidimensionais e vetor como parâmetro de função
- `switch/case`, `do...while`, `continue`
- `if`/`while`/`for` sem chaves
- Atribuição como sub-expressão (`if ((x = f()) > 0)`)
- Comentário de bloco `/* ... */` (só `// linha`)
- Pré-processador (`#include`, `#define`, macros)
- Sobrecarga de função, recursão mútua entre funções fora de ordem, protótipos/declaração antecipada de função
- Strings como tipo de variável (só existem como literais de formato de `printf`/`scanf`)
- `%s` funcional para tipos não-`bool` em `printf`
- `scanf` com `%s`
- Conversão de tipo verificada em argumento de função (só a quantidade é checada)

## 14. Resumo rápido para quem for escrever `.expected`

1. Erro semântico → **nenhuma saída do programa**, só as mensagens de erro + contagem final (em `stderr`).
2. Erro de execução (`Erro RT:`) → o programa **continua rodando**; a saída inclui tanto a mensagem de erro quanto o restante da execução normal.
3. `float` impresso "cru" (`printf(x)` ou início de `printf("%s", boolValue)` aparte) usa `%g`, não `%f` — atenção à diferença de formatação (`%g` omite zeros à direita, `%f` sempre mostra 6 decimais).
4. Divisão por zero entre dois literais é pega na hora do parse, não em runtime, e sempre relata "linha 1".
5. Use sempre `{}` em `if`/`while`/`for`.
6. Funções devem ser definidas antes de usadas no arquivo.
7. `scanf` não funciona com `stdin` redirecionado de arquivo — só interativo via terminal.