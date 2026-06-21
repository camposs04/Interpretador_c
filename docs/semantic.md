# Análise Semântica (semantic.md)

## 1. Visão Geral

O sistema realiza:

* Construção da AST
* Inferência simples de tipos e validações semânticas avançadas
* Armazenamento de símbolos e controle de escopo (`tabsym`)
* Interpretação direta da AST (`interpreter`)
* Geração de TAC a partir da AST

Diferente das versões iniciais, o sistema agora **executa** o código através de um interpretador (`interpreter`), além de realizar a análise semântica considerando controle de escopo e declaração de funções e vetores.

---

## 2. Estrutura de Tipos

```c
typedef enum {
    T_INT,
    T_FLOAT,
    T_CHAR,
    T_BOOL,
    T_VOID,
    T_FUNC
} Tipo;
```

---

## 3. Inferência de Tipos e Validações

Regra aplicada em operações binárias comuns (`+`, `-`, etc):
* Se algum operando é `T_FLOAT` → resultado `T_FLOAT`
* Caso contrário → `T_INT`

O analisador semântico (`semantic.c`) também realiza validações robustas, incluindo:
* Verificação do uso indevido de funções `void` em expressões.
* Verificação de tipos em retornos de funções de acordo com a assinatura.
* Validação semântica de acesso e atribuição em elementos de vetores.
* Restrição de uso da instrução `break` apenas para dentro de laços de repetição.
* Verificação da quantidade de parâmetros em chamadas de função.

### 3.1 Constant Folding

Durante a construção da AST, expressões compostas inteiramente por literais são calculadas em tempo de compilação e reduzidas a um único nó literal.

Exemplos:
- `2 + 3 * 4`  →  nó INT(14)
- `5 > 3`      →  nó BOOL(true)

Identidades algébricas também são simplificadas quando um dos operandos é literal:

| Expressão    | Resultado |
|--------------|-----------|
| `x + 0`      | `x`       |
| `x * 1`      | `x`       |
| `x * 0`      | `0`       |
| `x / 1`      | `x`       |
| `x && true`  | `x`       |
| `x && false` | `false`   |
| `x || true`  | `true`    |
| `x || false` | `x`       |
| `!!x`        | `x`       |

---

## 4. Declarações e Escopos

O tipo é propagado para todos os nós da lista após a criação da AST no parser. Cada identificador declarado é inserido na tabela de símbolos do **escopo atual** com nome e tipo correspondentes.

Há suporte ampliado para vetores e funções:
```c
int x, y;          // Variáveis simples
int v[10];         // Vetor (aloca tamanho no símbolo)
int soma(int a) {} // Função (abre novo escopo, insere parâmetros)
```

---

## 5. Tabela de Símbolos

A tabela de símbolos (`tabsym.h` / `tabsym.c`) suporta múltiplos escopos hierárquicos.

### Estrutura

```c
typedef struct Symb {
    char        *name;
    char        *type;       /* "int", "float", etc. ou "func" */
    Tipo         retorno;    /* tipo de retorno, se função */
    int          isFuncao;
    Param       *params;     /* lista de parâmetros */
    struct noAST *corpo;     /* AST do corpo da função */
    int          isVetor;    /* 1 se for vetor */
    int          tamanho;    /* tamanho se for vetor */
    struct Symb  *next;      /* encadeamento para colisões */
} Symb;

typedef struct Escopo {
    Symb          *tabela[TABLE_SIZE];
    struct Escopo *anterior;
} Escopo;
```

### Funcionamento

* Baseada em **hash table** com `TABLE_SIZE = 211` buckets
* Pilha de escopos controlada através de `entrarEscopo()` e `sairEscopo()`
* Colisões resolvidas por **encadeamento separado** (listas ligadas)

### Operações disponíveis

| Função                    | Descrição                                                      |
|---------------------------|----------------------------------------------------------------|
| `insertSymbol`            | Insere variável no escopo atual                                |
| `insertVetor`             | Insere um vetor no escopo atual registrando o seu tamanho      |
| `insertFuncao`            | Insere uma função no escopo global (com parâmetros e corpo)    |
| `searchSymbol`            | Busca um símbolo subindo hierarquicamente pela pilha de escopos|
| `searchSymbolEscopoAtual` | Busca um símbolo restrito ao escopo local (evita redeclaração) |
| `imprimirTabela`          | Exibe todos os símbolos por escopo para debug                  |

### Validações semânticas

* **Redeclaração**: Emite erro se o símbolo já existe no mesmo escopo local.
* **Uso sem declaração**: Emite erro ao buscar um identificador que não consta em nenhum escopo acima.

---

## 6. Estrutura do Nó AST

O nó da AST foi evoluído significativamente para acomodar parâmetros e manipulações textuais maiores.

```c
typedef struct noAST {
    char operador;
    Tipo tipo;

    union {
        int   i;
        float f;
        char  c;
    } valor;

    char nome[256];

    struct noAST *esquerda;
    struct noAST *direita;

    /* usado apenas em nós de definição de função ('Z') */
    Param *params;
} NoAST;
```

---

## 7. Interpretação e Geração de TAC

Após a construção e validação semântica:
1. **Geração de TAC**: A função `gerarTAC` percorre a árvore para emitir as instruções de três endereços representativas.
2. **Interpretador**: A função `interpretarPrograma` (no módulo `interpreter`) avalia a AST. As variáveis e vetores passam a possuir uma contraparte em "Tempo de Execução" (`VarRT`, `Valor`), alocados sob controle de `EscopoRT`. Expressões resultam em computações efetivas, loops saltam e chamadas de função desviam a execução repassando seus argumentos ao escopo correspondente.

---

## 8. Funcionalidades Alcançadas (Antigas Limitações)

O sistema superou as antigas limitações das primeiras Sprints:
* **Escopo Aninhado**: Totalmente suportado (pilhas de escopos resolvem `if/else`, laços e funções).
* **Funções**: Implementado suporte a parâmetros, retorno válido e travessia interligada.
* **Vetores**: Validados semanticamente e instanciados em *runtime*.
* **Execução Real**: O código fonte lido pelo parser agora ganha "vida" via o Interpretador C.

---

## 9. Próximos Passos (Evoluções Possíveis)

* Validação de *tipo estrito* em atribuições (sem coerções automáticas implícitas silenciosas).
* Suporte a strings dinâmicas manipuláveis.
* Introdução de novos tipos compostos, como ponteiros ou `structs`.