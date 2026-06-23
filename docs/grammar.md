# Analisador Sintático (grammar.md)

## 1. Visão Geral

Este módulo define a **gramática da linguagem** utilizando o gerador de parser **Bison**.

O parser é responsável por:

* Validar a estrutura sintática do programa.
* Construir uma **Árvore Sintática Abstrata (AST)**.
* Disparar a análise semântica e, se não houver erros, a execução (via Interpretador) e a geração de TAC.
* Tratar erros sintáticos com indicação precisa de linha e coluna.

A gramática implementa um subconjunto da linguagem C com suporte a:

* Declarações de variáveis e vetores.
* Funções (definição, parâmetros e chamada).
* Atribuições simples, compostas e incremento/decremento.
* Expressões aritméticas, lógicas e relacionais.
* Estruturas condicionais (`if` / `if-else`).
* Laços de repetição (`while`, `for`).
* Blocos de escopo delimitados por `{}`.
* Instruções I/O (`printf`, `scanf`).

---

## 2. Estrutura Geral do Programa

### Regra principal

```c
programa:
    lista
;
```

Ao final do parsing, o interpretador e gerador de TAC são chamados se não houverem erros semânticos:

```c
root = $1;
analisarSemantica(root);
// Se sucesso:
interpretarPrograma(root);
```

---

## 3. Sequência de Instruções

```c
lista:
    lista elemento
    | elemento
;
```

* Implementa múltiplas instruções e/ou funções.
* Internamente gera nós de sequência (`;`) na AST.

---

## 4. Elementos

```c
elemento:
    comando
  | def_funcao
;
```
Um elemento de escopo global pode ser um comando direto ou a definição de uma função.

---

## 5. Blocos

```c
bloco:
    ABRE_CHAVES comandos FECHA_CHAVES
;
```

* Representam agrupamento de instruções.
* **Escopo Semântico**: Na criação do bloco no parser, `entrarEscopo()` e `sairEscopo()` são chamados, refletindo o escopo hierárquico autêntico da linguagem.

---

## 6. Comandos principais

A regra de `comando` expandiu para englobar as novas estruturas:

* `expressao ;`
* `atribuicao`
* `declaracao`
* `bloco`
* `if`, `if-else`
* `while`, `for`
* `printf`, `scanf`
* `return`, `break`

---

## 7. Tipos Suportados

```c
tipo:
    INT | FLOAT | CHAR | BOOL
;
```
As funções podem retornar esses tipos ou `VOID`.

---

## 8. Declarações

Com suporte a vetores (arrays):

```c
declaracao:
    tipo lista_ids PONTO_VIRGULA
  | tipo ID '[' INT_NUM ']' PONTO_VIRGULA
  | tipo ID '[' INT_NUM ']' '=' '{' lista_valores '}' PONTO_VIRGULA
;
```

Exemplos:
```c
int x = 10, y = 20;
int arr[5];
int arr[3] = {1, 2, 3};
```

---

## 9. Atribuição

Com suporte a operadores compostos, incremento e acesso a vetores:

```c
atribuicao:
    ID EQUAL expressao PONTO_VIRGULA
  | ID ADD_EQUAL expressao PONTO_VIRGULA // +=, -=, *=, /=, %=
  | INCREMENT ID PONTO_VIRGULA           // ++x
  | ID INCREMENT PONTO_VIRGULA           // x++
  | ID '[' expressao ']' EQUAL expressao PONTO_VIRGULA // arr[i] = x
;
```

---

## 10. Expressões e Precedência

Expressões expandidas com lógicos e unários.

### Precedência de Operadores

```c
%right EQUAL ADD_EQUAL SUB_EQUAL MULT_EQUAL DIV_EQUAL MOD_EQUAL
%left  OR
%left  AND
%left  DEQ NEQ
%left  LT GT LE GE
%left  PLUS MINUS
%left  MULT DIV MOD
%right NOT UMINUS
%right INCREMENT DECREMENT
```

| Prioridade | Operadores                      |
|------------|---------------------------------|
| Alta       | `++`, `--`, `!`, `-` (unário)   |
| Média-Alta | `*`, `/`, `%`                   |
| Média      | `+`, `-`                        |
| Média-Baixa| `<`, `>`, `<=`, `>=`            |
| Baixa      | `==`, `!=`                      |
| Lógica     | `&&`, `||`                      |
| Atribuição | `=`, `+=`, `-=`, etc. (à dir.)  |

---

## 11. Otimização e Construção da AST

A AST acomoda nós complexos como `Z` (funções), `V` (vetores), e laços. A função `criarNoOp` aplica **constant folding** antes de alocar um nó binário:

* Quando ambos os filhos são literais, retorna diretamente um nó literal.
* Simplifica identidades (`x + 0`, `x * 1`, etc.).

---

## 12. Tratamento de Erros

A recuperação de erros do Bison evita abortar o programa instantaneamente:

```c
| error PONTO_VIRGULA { yyerrok; yyclearin; }
```

Função de erro enriquecida:
Exibe linha, coluna, token inesperado, e uma representação visual com o cursor `^` apontando no código fonte.