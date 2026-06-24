# Análise Semântica — InterpretadorC

Especificação do módulo `semantic.c` / `semantic.h`: o que ele valida, como interage com a tabela de símbolos (`tabsym.c`), quais tipos de erro produz e qual o efeito de cada erro sobre a execução do programa.

## 1. Papel da análise semântica no pipeline

```
AST (já construída pelo parser)
        │
        ▼
analisarSemantica(root)     ← percorre a AST inteira, uma única vez, recursivamente
        │
        ▼
errosSemanticos() > 0 ?
        │
   sim ─┴─ não
   │        │
   ▼        ▼
 aborta   interpretarPrograma(root)
```

A análise semântica é chamada **uma única vez**, logo depois que o parser termina de construir a árvore completa (na ação da regra `programa` em `parserC.y`). Ela não gera código nem produz uma árvore nova — ela **percorre e anota** a AST existente (preenchendo o campo `tipo` de certos nós) e acumula um contador de erros.

> **Se `errosSemanticos() > 0` ao final, o programa não é executado.** Nenhuma saída de `printf`/`scanf` do programa-fonte aparece nesse caso — só as mensagens de erro semântico e, ao final, a contagem total em `stderr`.

## 2. Interface pública (`semantic.h`)

```c
void erroSemantico(int codigoErro, int linha);  // registra um erro "pré-definido" (hoje só divisão por zero)
void analisarSemantica(NoAST *raiz);            // percorre a AST a partir da raiz
int  errosSemanticos(void);                     // retorna a contagem total acumulada
```

- `erroSemantico()` é a única função de erro que recebe um **código** em vez de imprimir a mensagem diretamente. Hoje só existe um código definido (`ERR_DIVISAO_POR_ZERO`, em `tipos.h`), usado quando o *constant folding* do `ast.c` detecta uma divisão (ou módulo) por um literal `0` **no momento da construção da árvore**, antes mesmo de `analisarSemantica` ser chamada. Por isso esse erro específico pode aparecer mesmo que a árvore ainda nem tenha sido percorrida semanticamente.
- Todos os outros erros semânticos são reportados diretamente com `printf(...)` seguido de `numErros++` dentro de `analisarSemantica` — não passam por `erroSemantico()`.
- `errosSemanticos()` é consultado pela regra `programa` do parser para decidir se interpreta ou aborta.

## 3. Estado interno mantido pelo módulo

```c
static int  numErros          = 0;       // contador global de erros semânticos
static int  profundidadeLaco  = 0;       // >0 quando dentro de while/for (para validar 'break')
static Tipo tipoFuncaoAtual   = T_VOID;  // tipo de retorno da função sendo analisada agora
static int  dentroDeFuncao    = 0;       // 1 enquanto analisa o corpo de uma função
```

Esse estado é **global ao módulo** (não está na AST nem na tabela de símbolos) e é salvo/restaurado manualmente ao entrar/sair do corpo de uma função, para suportar (em teoria) funções aninhadas — embora a gramática atual nunca produza esse caso, já que `def_funcao` só aparece no nível superior do arquivo.

## 4. Integração com a tabela de símbolos (`tabsym.c`)

A análise semântica é a única fase que chama `entrarEscopo()`/`sairEscopo()` **com efeito real** sobre as decisões de tipo/declaração (o parser também chama essas funções dentro da regra `bloco`, mas isso só afeta o estado da tabela durante o *parse*, que é descartado depois — ver seção 4 do documento de escopo geral do projeto).

A análise semântica abre um escopo novo exatamente nestes pontos:

| Nó da AST | Escopo aberto? |
|---|---|
| `'Z'` (definição de função) | Sim — parâmetros são inseridos nesse escopo antes de analisar o corpo |
| `'f'` (if) — ramo `then` | Sim |
| `'f'` (if) — ramo `else` (se existir) | Sim, separado do `then` |
| `'W'` (while) | Sim |
| `'F'` (for) | Sim, em **dois níveis**: um escopo externo (envolve init/cond/incr) e um interno por iteração léxica do corpo |
| `'Q'` (bloco solto `{ }` usado como comando autônomo) | Sim |

Dentro de cada escopo, duas operações da tabela de símbolos são usadas:
- `searchSymbolEscopoAtual(nome)` — só no escopo mais interno, usada para detectar **redeclaração**.
- `searchSymbol(nome)` — em toda a pilha de escopos (do mais interno ao mais externo), usada para validar **uso** de uma variável/função já declarada (sombreamento é resolvido naturalmente: o símbolo mais interno "ganha").

## 5. Verificações por construção (caso a caso)

### 5.1 Declaração de variável escalar — `'d'`
```c
declaration(NoAST *raiz):
    se searchSymbolEscopoAtual(nome) != NULL:
        Erro Semantico: variavel 'nome' ja declarada neste escopo.
    senão:
        insertSymbol(nome, tipoStr(raiz->tipo))
    se houver inicializador (raiz->direita):
        analisa recursivamente o inicializador
        verifica se o inicializador não é uma chamada de função void (ver 5.9)
```
Não há checagem de **tipo** entre o tipo declarado e o tipo do inicializador — qualquer combinação passa a análise semântica (a conversão acontece em runtime, no interpretador).

### 5.2 Declaração de vetor — `'V'`
```c
se searchSymbolEscopoAtual(nome) != NULL:
    Erro Semantico: variavel 'nome' ja declarada neste escopo.
se tamanho <= 0:
    Erro Semantico: vetor 'nome' deve ter tamanho positivo.
senão:
    insertVetor(nome, tipo, tamanho)
    para cada valor inicial na lista:
        analisa recursivamente, verifica não-void
    se quantidade_de_valores > tamanho:
        Erro Semantico: vetor 'nome' tem N valor(es) inicial(is) para M posicao(oes).
```
Observação: a checagem de tamanho positivo e a inserção na tabela de símbolos acontecem **antes** da checagem de excesso de inicializadores — ou seja, mesmo com excesso de valores, o vetor já foi registrado (o erro de excesso não impede o uso posterior do nome).

### 5.3 Uso de identificador — `'i'`
```c
se searchSymbol(nome) == NULL:
    Erro Semantico: variavel 'nome' nao declarada.
    (retorna sem definir raiz->tipo)
senão:
    raiz->tipo ← tipo do símbolo encontrado (convertido de string para Tipo)
```

### 5.4 Acesso de leitura a vetor — `'X'` (`v[i]`)
```c
se símbolo não existe OU não é vetor:
    Erro Semantico: 'nome' nao e um vetor declarado.
senão:
    analisa o índice (esquerda) recursivamente
    raiz->tipo ← tipo do elemento do vetor
```
Não há checagem estática do **valor** do índice (isso só é possível em runtime, pois o índice pode ser uma expressão dinâmica).

### 5.5 Atribuição a elemento de vetor — `'Y'` (`v[i] = expr;`)
Mesma checagem de "é vetor declarado" que `'X'`, mais a análise recursiva do índice e do valor, e a verificação de não-void em ambos.

### 5.6 Atribuição simples e composta — `'=' 'a' 's' 'm' 'v' 'r'`
```c
analisa esquerda (identificador) e direita (expressão) recursivamente
verifica se o lado direito não é chamada de função void
```
Não há checagem de que o identificador do lado esquerdo já foi declarado **nesta** regra especificamente — essa checagem acontece quando a recursão chega ao nó `'i'` da esquerda (seção 5.3).

### 5.7 Incremento/decremento — `'I' 'D'`
Apenas analisa recursivamente o operando esquerdo (não há verificação adicional além da que `'i'` já faz).

### 5.8 Sequência de comandos — `';'`
Apenas analisa esquerda e depois direita, em ordem — é assim que a ordem de declaração/uso no código-fonte é respeitada durante a análise (inclusive a restrição de "função deve ser definida antes de usada", que decorre diretamente desta ordem sequencial de visita).

### 5.9 Verificação de retorno `void` usado como valor
Função auxiliar `verificarNaoVoid(expr)`, chamada depois de qualquer expressão já ter sido analisada (para que o tipo de retorno do símbolo já esteja resolvido):
```c
se expr é uma chamada de função ('C') E a função tem retorno void:
    Erro Semantico: funcao 'nome' tem retorno void e nao pode ser usada como valor.
```
É chamada em todos os contextos onde um valor é esperado: inicializador de declaração, valores de vetor, lado direito de atribuição/atribuição composta, argumentos de `printf`/`scanf`/chamada de função, e expressão de `return`.

### 5.10 `if`/`else` — `'f'`
```c
analisa a condição
entra em escopo novo → analisa o ramo 'then' → sai do escopo
se houver ramo 'else':
    entra em outro escopo novo → analisa o ramo 'else' → sai do escopo
```

### 5.11 `while` — `'W'`
```c
analisa a condição
entra em escopo novo
profundidadeLaco++
analisa o corpo
profundidadeLaco--
sai do escopo
```

### 5.12 `for` — `'F'`
```c
entra em escopo novo (nível externo: init/cond/incr)
    analisa init, condição e incremento
    entra em OUTRO escopo novo (nível interno: corpo)
    profundidadeLaco++
    analisa o corpo
    profundidadeLaco--
    sai do escopo interno
sai do escopo externo
```

### 5.13 `break` — `'B'`
```c
se profundidadeLaco == 0:
    Erro Semantico: 'break' usado fora de um laco.
```

### 5.14 `printf` simples e com formato — `'P' 'R'`
Analisa recursivamente a expressão (`'P'`) ou cada argumento da lista (`'R'`), verificando não-void em cada um.

### 5.15 Operadores lógicos — `'A' 'O' 'N'`
```c
analisa esquerda (e direita, se houver)
raiz->tipo ← T_BOOL
```

### 5.16 Menos unário — `'u'`
```c
analisa o operando
raiz->tipo ← tipo do operando analisado
```

### 5.17 Definição de função — `'Z'`
```c
se searchSymbolEscopoAtual(nome) != NULL:
    Erro Semantico: funcao 'nome' ja declarada.
senão:
    insertFuncao(nome, tipoRetorno, params, corpo)   ← registrada ANTES de analisar o corpo
                                                         (permite recursão direta/simples)
entra em escopo novo
    insere cada parâmetro formal como símbolo nesse escopo
    salva tipoFuncaoAtual/dentroDeFuncao anteriores
    tipoFuncaoAtual ← tipo de retorno desta função
    dentroDeFuncao  ← 1
    analisa o corpo
    restaura tipoFuncaoAtual/dentroDeFuncao anteriores
sai do escopo
```
Como a função é inserida no escopo **antes** de o corpo ser analisado, **recursão direta funciona** (a função pode se chamar dentro do próprio corpo). Como a inserção só acontece quando o parser/semântica chega ao nó `'Z'` correspondente (em ordem sequencial pela regra `';'`, seção 5.8), **uma função só pode chamar outra que já tenha sido definida antes dela no arquivo** — não há pré-varredura de assinaturas, então recursão mútua entre duas funções não é suportada.

### 5.18 Chamada de função — `'C'`
```c
se símbolo não existe OU não é função:
    Erro Semantico: funcao 'nome' nao declarada.
    (retorna sem analisar argumentos)
senão:
    raiz->tipo ← tipo de retorno da função (propagado para o nó de chamada)
    conta parâmetros formais (nparams) e argumentos passados (nargs)
    se nargs != nparams:
        Erro Semantico: funcao 'nome' espera N argumento(s), recebeu M.
    analisa cada argumento recursivamente, verificando não-void em cada um
```
Note que a checagem de **quantidade** de argumentos acontece mesmo que dê erro, mas a análise dos argumentos individuais **só ocorre se a função foi encontrada** — se a função não existe, os argumentos passados na chamada não são analisados (não geram erros próprios de "variável não declarada" mesmo que estejam errados).

### 5.19 `return` — `'K'`
```c
analisa a expressão de retorno (se houver), verifica não-void
se NÃO está dentro de função (dentroDeFuncao == 0):
    retorna sem checagem adicional (return no nível global é tolerado, sem efeito)
senão se tipoFuncaoAtual == T_VOID:
    se há expressão de retorno:
        Erro Semantico: funcao void nao pode retornar um valor.
senão (função não-void):
    se NÃO há expressão de retorno:
        Erro Semantico: funcao do tipo 'X' deve retornar um valor.
```
**Não há verificação de que o `return` realmente aparece em todos os caminhos possíveis** (não existe checagem de "função não-void pode terminar sem `return`" do tipo que compiladores reais costumam fazer) — isso é responsabilidade do programador.

### 5.20 Caso padrão (`default`) — operadores aritméticos/relacionais binários
Cobre os operadores que não têm `case` próprio: `+ - * / % e ! < > L G` (os caracteres internos para `== != <= >=`).
```c
analisa esquerda e direita recursivamente
verifica não-void em ambos os lados
se ambos os lados existem:
    raiz->tipo ← T_FLOAT se qualquer lado for T_FLOAT, senão T_INT
```
⚠️ **Particularidade conhecida:** esse cálculo de tipo (`T_FLOAT`/`T_INT`) é aplicado tanto aos operadores aritméticos (`+ - * /` — correto) quanto aos **relacionais** (`e ! < > L G`, ou seja `== != < > <= >=`), que deveriam logicamente resultar em `T_BOOL`. Isso significa que o campo `tipo` de um nó de comparação fica marcado como `T_INT`/`T_FLOAT` em vez de `T_BOOL` após a análise semântica. **Na prática isso não afeta o resultado de nenhum programa**, porque o interpretador (`interpreter.c`) recalcula o tipo `T_BOOL` desses operadores diretamente em tempo de execução, sem depender do campo `tipo` anotado pela semântica. É uma inconsistência interna que só importaria se outro código (futuro) viesse a confiar nesse campo para decidir o tipo de uma comparação antes de rodar o programa.

## 6. Tabela completa de mensagens de erro semântico

| # | Situação | Mensagem exata |
|---|---|---|
| 1 | Variável redeclarada no escopo atual | `Erro Semantico: variavel '<nome>' ja declarada neste escopo.` |
| 2 | Variável usada sem declaração | `Erro Semantico: variavel '<nome>' nao declarada.` |
| 3 | Vetor com tamanho ≤ 0 | `Erro Semantico: vetor '<nome>' deve ter tamanho positivo.` |
| 4 | Excesso de valores na inicialização do vetor | `Erro Semantico: vetor '<nome>' tem <N> valor(es) inicial(is) para <M> posicao(oes).` |
| 5 | Acesso/atribuição a algo que não é vetor declarado | `Erro Semantico: '<nome>' nao e um vetor declarado.` |
| 6 | `break` fora de laço | `Erro Semantico: 'break' usado fora de um laco.` |
| 7 | Função redefinida | `Erro Semantico: funcao '<nome>' ja declarada.` |
| 8 | Chamada de função não declarada (ou ainda não definida no arquivo) | `Erro Semantico: funcao '<nome>' nao declarada.` |
| 9 | Número de argumentos incorreto | `Erro Semantico: funcao '<nome>' espera <N> argumento(s), recebeu <M>.` |
| 10 | Retorno `void` usado como valor | `Erro Semantico: funcao '<nome>' tem retorno void e nao pode ser usada como valor.` |
| 11 | `return expr;` dentro de função `void` | `Erro Semantico: funcao void nao pode retornar um valor.` |
| 12 | `return;` (sem valor) dentro de função não-`void` | `Erro Semantico: funcao do tipo '<tipo>' deve retornar um valor.` |
| 13 | Divisão/módulo por literal `0` (detectado na construção da AST, antes da análise formal) | `Erro Semantico: divisao por zero na linha <L>.` — **`<L>` é sempre `1`** nesta implementação, pois o contador de linha do Bison/Flex (`yylineno`) nunca é incrementado (ver observação na seção 7) |

Ao final, se houver pelo menos um erro (de qualquer tipo da tabela acima), é impressa em `stderr`:
```
<contagem> erro(s) semantico(s) encontrado(s).
```

## 7. Observações e particularidades para quem for testar

- **A ordem das mensagens de erro segue a ordem de visita da AST**, que por sua vez segue a ordem em que as construções aparecem no código-fonte (a sequência `';'` é visitada esquerda-antes-direita). Para arquivos de teste com múltiplos erros, a ordem esperada na saída é a ordem em que os problemas aparecem no texto, de cima para baixo.
- **Um único arquivo pode acumular erros de fontes diferentes** (ex.: uma variável não declarada *e* um `break` fora de laço *e* uma função com número errado de argumentos) — todos são reportados, e a contagem final soma todos eles. A análise **não para no primeiro erro**.
- **Erro de "divisão por zero" da seção 6, item 13, é o único que pode aparecer mesmo antes da árvore ser visitada por `analisarSemantica`** — ele vem do *constant folding* em `ast.c`, executado durante a própria construção da árvore pelo parser. Isso também explica por que o número da linha está sempre incorreto (fixo em `1`): esse código usa a variável `yylineno` do Flex, que nunca chega a ser incrementada nesta implementação porque a opção `%option yylineno` não foi declarada em `lexerC.l`. Nenhuma outra mensagem de erro (sintático, léxico, semântico "normal") tem esse problema — todas as outras usam o contador de linha manual (`linha`) mantido manualmente em `lexerC.l`, que funciona corretamente.
- **A análise semântica não verifica compatibilidade de tipo em atribuições nem em parâmetros de função** — só existência de símbolo, escopo, contagem de argumentos e as regras específicas de `return`/`break`/vetor listadas acima. Qualquer combinação de tipos numéricos passa livremente (a conversão acontece em runtime).
- **Não há verificação de "todos os caminhos retornam um valor"** para funções não-`void` — uma função pode terminar sem nunca executar um `return` e isso não gera erro semântico (em runtime, o valor de retorno nesse caso seria o padrão zerado).
- **Self-recursão funciona, recursão mútua não** — consequência direta da ordem sequencial de registro de funções (seção 5.17).