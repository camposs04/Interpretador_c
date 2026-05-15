# Análise Semântica (semantic.md)

## 1. Visão Geral

O sistema realiza:

* Construção da AST
* Inferência simples de tipos
* Armazenamento de símbolos via tabela hash (`tabsym`)
* Geração de TAC a partir da AST

O sistema **não executa** o código — a saída final é o TAC impresso.

---

## 2. Estrutura de Tipos

```c
typedef enum {
    T_INT,
    T_FLOAT,
    T_CHAR,
    T_BOOL
} Tipo;
```

---

## 3. Inferência de Tipos

Regra aplicada em `criarNoOp`:

* Se algum operando é `T_FLOAT` → resultado `T_FLOAT`
* Caso contrário → `T_INT`

---

## 4. Declarações

O tipo é propagado para todos os nós da lista após a criação da AST no parser. Cada identificador declarado é inserido na tabela de símbolos com nome e tipo correspondentes.

```c
int x, y;   // dois nós 'd' com tipo T_INT inseridos na tabsym
```

---

## 5. Tabela de Símbolos

A sprint 4 introduziu a tabela de símbolos (`tabsym.h` / `tabsym.c`) conectada às fases de análise.

### Estrutura

```c
typedef struct Symb {
    char *name;
    char *type;
    struct Symb *next;  // encadeamento para colisões
} Symb;
```

### Funcionamento

* Baseada em **hash table** com `TABLE_SIZE = 211` buckets
* Colisões resolvidas por **encadeamento separado** (listas ligadas por bucket)
* Função de hash: somatório dos valores ASCII dos caracteres do nome

### Operações disponíveis

| Função           | Descrição                                                  |
|------------------|------------------------------------------------------------|
| `insertSymbol`   | Insere símbolo com nome e tipo; rejeita duplicatas         |
| `searchSymbol`   | Busca símbolo pelo nome via índice de hash                 |
| `imprimirTabela` | Percorre e exibe todos os buckets preenchidos              |

### Validações semânticas

* **Redeclaração**: `insertSymbol` emite erro se o símbolo já existe no escopo atual
* **Uso sem declaração**: `searchSymbol` retorna `NULL` quando o símbolo não foi declarado

---

## 6. Estrutura do Nó AST

```c
struct noAST {
    char  operador;
    Tipo  tipo;
    union { int i; float f; char c; } valor;
    char  nome[32];
    struct noAST *esquerda;
    struct noAST *direita;
}
```

---

## 7. Geração de TAC

Após a construção da AST, `gerarTAC` percorre a árvore e emite instruções de três endereços. Cada expressão gera temporários (`t1`, `t2`, …) e cada bloco de controle gera labels (`L1`, `L2`, …).

Exemplo de saída para `int x = 3 + 5;`:

```
decl int x
t1 = 3
t2 = 5
t3 = t1 + t2
x = t3
```

---

## 8. Limitações

* Sem escopo aninhado — a tabela atual é global
* Sem validação de tipos em atribuições
* Sem execução real do código
* Sem suporte a funções

---

## 9. Próximos Passos

* Controle de escopo (`entrarEscopo` / `sairEscopo`) para blocos `if/else` e funções
* Verificação de tipos em atribuições e expressões
* Execução real via travessia da AST