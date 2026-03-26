# Especificação da Gramática

Este documento define as regras léxicas e sintáticas da linguagem suportada pelo **InterpretadorC**.


## 1. Analisador Léxico (Tokens)

### 1.1 Palavras-Chave (Keywords)
| Token | Descrição |
| :--- | :--- |
| `int` | Declaração de número inteiro |
| `float` | Declaração de número de ponto flutuante |
| `if` | Estrutura condicional |
| `else` | Desvio condicional |

### 1.2 Operadores e Símbolos
* **Aritméticos:** `+`, `-`, `*`, `/`
* **Relacionais:** `==`, `!=`, `<`, `>`, `<=`, `>=`
* **Atribuição:** `=`
* **Delimitadores:** `(`, `)`, `{`, `}`, `;`, `,`
