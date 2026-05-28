#include "ast.h"
#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylineno;

/* Aloca e zera um nó — evita campos com lixo de memória */
static NoAST *alocarNo(void) {
    NoAST *novo = calloc(1, sizeof(NoAST));
    return novo;
}

NoAST *criarNoInt(int valor) {
    NoAST *novo = alocarNo();
    novo->operador = 'n';
    novo->tipo     = T_INT;
    novo->valor.i  = valor;
    return novo;
}

NoAST *criarNoFloat(float valor) {
    NoAST *novo = alocarNo();
    novo->operador = 'n';
    novo->tipo     = T_FLOAT;
    novo->valor.f  = valor;
    return novo;
}

NoAST *criarNoChar(char valor) {
    NoAST *novo = alocarNo();
    novo->operador = 'n';
    novo->tipo     = T_CHAR;
    novo->valor.c  = valor;
    return novo;
}

NoAST *criarNoBool(int valor) {
    NoAST *novo = alocarNo();
    novo->operador = 'n';
    novo->tipo     = T_BOOL;
    novo->valor.i  = valor ? 1 : 0;
    return novo;
}

NoAST *criarNoId(char *nome) {
    NoAST *novo = alocarNo();
    novo->operador = 'i';
    strncpy(novo->nome, nome, 63);
    novo->nome[63] = '\0';
    return novo;
}

/* O tipo da operação é resolvido APENAS com literais 'n'.
   Para identificadores ('i'), o tipo real só vem após análise semântica,
   por isso adiamos a inferência — o nó fica T_INT por padrão e a semântica
   corrige depois via analisarSemantica(). */
NoAST *criarNoOp(char operador, NoAST *esq, NoAST *dir) {

    /* ── 1. int × int ── */
    if (esq->operador == 'n' && dir->operador == 'n'
        && esq->tipo == T_INT && dir->tipo == T_INT) {

        int a = esq->valor.i, b = dir->valor.i;

        switch (operador) {
            case '+': return criarNoInt(a + b);
            case '-': return criarNoInt(a - b);
            case '*': return criarNoInt(a * b);
            case '/':
                if (b == 0) { 
                    erroSemantico(ERR_DIVISAO_POR_ZERO, yylineno); 
                    return criarNoInt(0); 
                }
                return criarNoInt(a / b);
            case '%': return criarNoInt(a % b);
            case '<': return criarNoBool(a <  b);
            case '>': return criarNoBool(a >  b);
            case 'e': return criarNoBool(a == b);
            case '!': return criarNoBool(a != b);
            case 'L': return criarNoBool(a <= b);
            case 'G': return criarNoBool(a >= b);
        }
    }

    /* ── 2. float × float ── */
    else if (esq->operador == 'n' && dir->operador == 'n'
             && esq->tipo == T_FLOAT && dir->tipo == T_FLOAT) {

        float a = esq->valor.f, b = dir->valor.f;

        switch (operador) {
            case '+': return criarNoFloat(a + b);
            case '-': return criarNoFloat(a - b);
            case '*': return criarNoFloat(a * b);
            case '/':
                if (b == 0) { 
                    erroSemantico(ERR_DIVISAO_POR_ZERO, yylineno); 
                    return criarNoFloat(0);
                }
                return criarNoFloat(a / b);
            case '<': return criarNoBool(a <  b);
            case '>': return criarNoBool(a >  b);
            case 'e': return criarNoBool(a == b);
            case '!': return criarNoBool(a != b);
            case 'L': return criarNoBool(a <= b);
            case 'G': return criarNoBool(a >= b);
        }
    }

    /* ── 3. misto int+float (promoção para float) ── */
    else if (esq->operador == 'n' && dir->operador == 'n'
             && (esq->tipo == T_FLOAT || dir->tipo == T_FLOAT)
             && (esq->tipo == T_INT   || esq->tipo == T_FLOAT)
             && (dir->tipo == T_INT   || dir->tipo == T_FLOAT)) {

        float a = (esq->tipo == T_FLOAT) ? esq->valor.f : (float)esq->valor.i;
        float b = (dir->tipo == T_FLOAT) ? dir->valor.f : (float)dir->valor.i;

        switch (operador) {
            case '+': return criarNoFloat(a + b);
            case '-': return criarNoFloat(a - b);
            case '*': return criarNoFloat(a * b);
            case '/':
                if (b == 0) { 
                    erroSemantico(ERR_DIVISAO_POR_ZERO, yylineno); 
                    return criarNoFloat(0.0f);
                }
                return criarNoFloat(a / b);
            case '<': return criarNoBool(a <  b);
            case '>': return criarNoBool(a >  b);
            case 'e': return criarNoBool(a == b);
            case '!': return criarNoBool(a != b);
            case 'L': return criarNoBool(a <= b);
            case 'G': return criarNoBool(a >= b);
        }
    }

    /* ── 4. identidades algébricas (um operando é literal) ── */

    /* Auxiliares para extrair valor numérico de um literal 'n' */
    #define LIT_INT(n)   ((n)->operador == 'n' && (n)->tipo == T_INT)
    #define LIT_FLOAT(n) ((n)->operador == 'n' && (n)->tipo == T_FLOAT)
    #define LIT_BOOL(n)  ((n)->operador == 'n' && (n)->tipo == T_BOOL)
    #define VAL_I(n)     ((n)->valor.i)
    #define VAL_F(n)     ((n)->valor.f)

    if (operador == '+') {
        /* x + 0  →  x */
        if (LIT_INT(dir)   && VAL_I(dir) == 0)   return esq;
        if (LIT_FLOAT(dir) && VAL_F(dir) == 0.0f) return esq;
        /* 0 + x  →  x */
        if (LIT_INT(esq)   && VAL_I(esq) == 0)   return dir;
        if (LIT_FLOAT(esq) && VAL_F(esq) == 0.0f) return dir;
    }

    if (operador == '-') {
        /* x - 0  →  x */
        if (LIT_INT(dir)   && VAL_I(dir) == 0)   return esq;
        if (LIT_FLOAT(dir) && VAL_F(dir) == 0.0f) return esq;
    }

    if (operador == '*') {
        /* x * 1  →  x */
        if (LIT_INT(dir)   && VAL_I(dir) == 1)   return esq;
        if (LIT_FLOAT(dir) && VAL_F(dir) == 1.0f) return esq;
        /* 1 * x  →  x */
        if (LIT_INT(esq)   && VAL_I(esq) == 1)   return dir;
        if (LIT_FLOAT(esq) && VAL_F(esq) == 1.0f) return dir;
        /* x * 0  →  0  (seguro: sem efeitos colaterais no estado atual) */
        if (LIT_INT(dir)   && VAL_I(dir) == 0)   return criarNoInt(0);
        if (LIT_FLOAT(dir) && VAL_F(dir) == 0.0f) return criarNoFloat(0.0f);
        if (LIT_INT(esq)   && VAL_I(esq) == 0)   return criarNoInt(0);
        if (LIT_FLOAT(esq) && VAL_F(esq) == 0.0f) return criarNoFloat(0.0f);
    }

    if (operador == '/') {
        /* x / 1  →  x */
        if (LIT_INT(dir)   && VAL_I(dir) == 1)   return esq;
        if (LIT_FLOAT(dir) && VAL_F(dir) == 1.0f) return esq;
    }

    #undef LIT_INT
    #undef LIT_FLOAT
    #undef LIT_BOOL
    #undef VAL_I
    #undef VAL_F

    /* ── 5. ao menos um operando não é literal → nó binário normal ── */
    NoAST *novo = alocarNo();
    novo->operador = operador;
    novo->esquerda = esq;
    novo->direita  = dir;

    if (operador == 'I' || operador == 'D') {
        novo->tipo = (esq && esq->operador == 'n' && esq->tipo == T_FLOAT)
                     ? T_FLOAT : T_INT;
        return novo;
    }

    Tipo te = (esq && esq->operador == 'n') ? esq->tipo : T_INT;
    Tipo td = (dir && dir->operador == 'n') ? dir->tipo : T_INT;
    novo->tipo = (te == T_FLOAT || td == T_FLOAT) ? T_FLOAT : T_INT;
    return novo;
}

NoAST *criarNoSeq(NoAST *esq, NoAST *dir) {
    /* Sequências com NULL (oriundas de erros de parser) são toleradas:
       retornamos o lado não-nulo para não propagar NULL pelo pipeline. */
    if (esq == NULL) return dir;
    if (dir == NULL) return esq;

    NoAST *novo = alocarNo();
    novo->operador = ';';
    novo->tipo     = T_VOID;
    novo->esquerda = esq;
    novo->direita  = dir;
    return novo;
}

NoAST *criarNoAtrib(char *nome, NoAST *valor) {
    NoAST *id = criarNoId(nome);
    NoAST *novo = alocarNo();
    novo->operador = '=';
    novo->esquerda = id;
    novo->direita  = valor;
    return novo;
}

NoAST *criarNoDecl(Tipo tipo, char *nome, NoAST *valor) {
    NoAST *novo = alocarNo();
    novo->operador = 'd';
    novo->tipo     = tipo;
    novo->esquerda = criarNoId(nome);
    novo->direita  = valor;
    return novo;
}

NoAST *criarNoIf(NoAST *condicao, NoAST *blocoTrue, NoAST *blocoFalse) {
    NoAST *corpo = alocarNo();
    corpo->operador = 'c';
    corpo->esquerda = blocoTrue;
    corpo->direita  = blocoFalse;

    NoAST *novo = alocarNo();
    novo->operador = 'f';
    novo->esquerda = condicao;
    novo->direita  = corpo;
    return novo;
}

/* 'W' = while(cond) corpo */
NoAST *criarNoWhile(NoAST *condicao, NoAST *corpo) {
    NoAST *novo = alocarNo();
    novo->operador = 'W';
    novo->esquerda = condicao;
    novo->direita  = corpo;
    return novo;
}

/* 'F' = for(init; cond; incr) corpo
   Estrutura: esquerda = seq(init, incr), direita = seq(cond, corpo) */
NoAST *criarNoFor(NoAST *init, NoAST *cond, NoAST *incr, NoAST *corpo) {
    NoAST *meta  = alocarNo();
    meta->operador = 'M';   /* metadados do for */
    meta->esquerda = init;
    meta->direita  = incr;

    NoAST *novo = alocarNo();
    novo->operador = 'F';
    novo->esquerda = meta;
    novo->direita  = criarNoSeq(cond, corpo);
    return novo;
}

/* 'P' = printf(expr) */
NoAST *criarNoPrintf(NoAST *expr) {
    NoAST *novo = alocarNo();
    novo->operador = 'P';
    novo->esquerda = expr;
    return novo;
}

/* Operadores lógicos: 'A'=&&, 'O'=||, 'N'=! */
NoAST *criarNoAnd(NoAST *esq, NoAST *dir) {
    /* identidades && */
    if (esq->operador == 'n' && esq->tipo == T_BOOL) {
        if (esq->valor.i == 1) return dir;          /* true  && x  →  x     */
        if (esq->valor.i == 0) return criarNoBool(0); /* false && x  →  false */
    }
    if (dir->operador == 'n' && dir->tipo == T_BOOL) {
        if (dir->valor.i == 1) return esq;          /* x && true   →  x     */
        if (dir->valor.i == 0) return criarNoBool(0); /* x && false  →  false */
    }
    NoAST *novo = alocarNo();
    novo->operador = 'A';
    novo->tipo     = T_BOOL;
    novo->esquerda = esq;
    novo->direita  = dir;
    return novo;
}

NoAST *criarNoOr(NoAST *esq, NoAST *dir) {
    /* identidades || */
    if (esq->operador == 'n' && esq->tipo == T_BOOL) {
        if (esq->valor.i == 0) return dir;          /* false || x  →  x    */
        if (esq->valor.i == 1) return criarNoBool(1); /* true  || x  →  true */
    }
    if (dir->operador == 'n' && dir->tipo == T_BOOL) {
        if (dir->valor.i == 0) return esq;          /* x || false  →  x    */
        if (dir->valor.i == 1) return criarNoBool(1); /* x || true   →  true */
    }
    NoAST *novo = alocarNo();
    novo->operador = 'O';
    novo->tipo     = T_BOOL;
    novo->esquerda = esq;
    novo->direita  = dir;
    return novo;
}

NoAST *criarNoNot(NoAST *operando) {
    /* dupla negação: !!x  →  x */
    if (operando->operador == 'N')
        return operando->esquerda;
    NoAST *novo = alocarNo();
    novo->operador = 'N';
    novo->tipo     = T_BOOL;
    novo->esquerda = operando;
    return novo;
}

/* ── impressão da AST ── */
void imprimirAST(NoAST *raiz) {
    if (raiz == NULL) return;

    switch (raiz->operador) {
        case 'n':
            switch (raiz->tipo) {
                case T_INT:   printf("%d",  raiz->valor.i); break;
                case T_FLOAT: printf("%g",  raiz->valor.f); break;
                case T_CHAR:  printf("'%c'",raiz->valor.c); break;
                case T_BOOL:  printf("%s",  raiz->valor.i ? "true" : "false"); break;
                default: break;
            }
            break;
        case 'i': printf("%s", raiz->nome); break;
        case 'd':
            switch (raiz->tipo) {
                case T_INT:   printf("int ");   break;
                case T_FLOAT: printf("float "); break;
                case T_CHAR:  printf("char ");  break;
                case T_BOOL:  printf("bool ");  break;
                default: break;
            }
            imprimirAST(raiz->esquerda);
            if (raiz->direita) { printf(" = "); imprimirAST(raiz->direita); }
            break;
        case '=':
            imprimirAST(raiz->esquerda);
            printf(" = ");
            imprimirAST(raiz->direita);
            break;
        case ';':
            imprimirAST(raiz->esquerda);
            printf(";\n");
            imprimirAST(raiz->direita);
            break;
        case 'f':
            printf("if ("); imprimirAST(raiz->esquerda); printf(") {\n");
            imprimirAST(raiz->direita->esquerda);
            printf("\n}");
            if (raiz->direita->direita) {
                printf(" else {\n");
                imprimirAST(raiz->direita->direita);
                printf("\n}");
            }
            break;
        case 'W':
            printf("while ("); imprimirAST(raiz->esquerda); printf(") {\n");
            imprimirAST(raiz->direita);
            printf("\n}");
            break;
        case 'F': {
            NoAST *meta = raiz->esquerda;
            NoAST *rest = raiz->direita;
            printf("for (");
            imprimirAST(meta->esquerda); printf("; ");
            imprimirAST(rest->esquerda); printf("; ");
            imprimirAST(meta->direita);
            printf(") {\n");
            imprimirAST(rest->direita);
            printf("\n}");
            break;
        }
        case 'P':
            printf("printf("); imprimirAST(raiz->esquerda); printf(")");
            break;
        case 'A':
            printf("("); imprimirAST(raiz->esquerda);
            printf(" && "); imprimirAST(raiz->direita); printf(")");
            break;
        case 'O':
            printf("("); imprimirAST(raiz->esquerda);
            printf(" || "); imprimirAST(raiz->direita); printf(")");
            break;
        case 'N':
            printf("!"); imprimirAST(raiz->esquerda);
            break;
        default:
            printf("("); imprimirAST(raiz->esquerda);
            switch (raiz->operador) {
                case 'e': printf(" == "); break;
                case '!': printf(" != "); break;
                case 'L': printf(" <= "); break;
                case 'G': printf(" >= "); break;
                default:  printf(" %c ", raiz->operador); break;
            }
            imprimirAST(raiz->direita);
            printf(")");
            break;
    }
}

/* Nó que guarda uma string literal no campo nome[] (até 255 chars).
   Para strings maiores seria necessário alocação dinâmica — suficiente
   para strings de formato de printf acadêmico. */
NoAST *criarNoString(const char *texto) {
    NoAST *novo = alocarNo();
    novo->operador = 'S';
    strncpy(novo->nome, texto, 255);
    novo->nome[255] = '\0';
    return novo;
}

/* printf com formato: operador 'R'
   esquerda = nó 'S' com a string de formato
   direita  = lista de args encadeada com ';' (ou NULL) */
NoAST *criarNoPrintfFmt(NoAST *fmt, NoAST *args) {
    NoAST *novo = alocarNo();
    novo->operador = 'R';
    novo->esquerda = fmt;
    novo->direita  = args;
    return novo;
}

/* Lista de argumentos do printf — usa operador 'L' para não conflitar com ';' de sequência.
   esquerda = argumento atual, direita = próximo nó 'L' ou NULL */
NoAST *criarListaArgs(NoAST *arg, NoAST *resto) {
    if (arg == NULL) return resto;
    NoAST *novo = alocarNo();
    novo->operador = 'L';
    novo->esquerda = arg;
    novo->direita  = resto;
    return novo;
}
/* ── nós de função ── */

Param *criarParam(Tipo tipo, const char *nome) {
    Param *p = calloc(1, sizeof(Param));
    p->tipo = tipo;
    strncpy(p->nome, nome, 63);
    p->nome[63] = '\0';
    p->prox = NULL;
    return p;
}

/* Adiciona 'novo' ao FINAL da lista para preservar ordem dos parâmetros */
Param *adicionarParam(Param *lista, Param *novo) {
    if (lista == NULL) return novo;
    Param *p = lista;
    while (p->prox) p = p->prox;
    p->prox = novo;
    return lista;
}

/* 'Z' = definição de função */
NoAST *criarNoDefFuncao(Tipo retorno, const char *nome, Param *params, NoAST *corpo) {
    NoAST *novo = calloc(1, sizeof(NoAST));
    novo->operador = 'Z';
    novo->tipo     = retorno;
    strncpy(novo->nome, nome, 255);
    novo->nome[255] = '\0';
    novo->params   = params;
    novo->esquerda = corpo;
    novo->direita  = NULL;
    return novo;
}

/* 'C' = chamada de função */
NoAST *criarNoChamada(const char *nome, NoAST *args) {
    NoAST *novo = calloc(1, sizeof(NoAST));
    novo->operador = 'C';
    novo->tipo     = T_VOID;   /* resolvido pela semântica */
    strncpy(novo->nome, nome, 255);
    novo->nome[255] = '\0';
    novo->esquerda = args;
    novo->direita  = NULL;
    return novo;
}

/* 'K' = return */
NoAST *criarNoReturn(NoAST *expr) {
    NoAST *novo = calloc(1, sizeof(NoAST));
    novo->operador = 'K';
    novo->tipo     = T_VOID;
    novo->esquerda = expr;
    novo->direita  = NULL;
    return novo;
}