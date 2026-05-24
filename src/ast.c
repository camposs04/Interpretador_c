#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    NoAST *novo = alocarNo();
    novo->operador  = operador;
    novo->esquerda  = esq;
    novo->direita   = dir;

    /* Para ++/--, o nó carrega o operador 'I'/'D' diretamente.
       O tipo é o do filho esquerdo (ou INT por padrão). */
    if (operador == 'I' || operador == 'D') {
        novo->tipo = (esq && esq->operador == 'n' && esq->tipo == T_FLOAT)
                     ? T_FLOAT : T_INT;
        return novo;
    }

    /* Inferência segura: só usa tipo se o nó é literal */
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
    NoAST *novo = alocarNo();
    novo->operador = 'A';
    novo->tipo     = T_BOOL;
    novo->esquerda = esq;
    novo->direita  = dir;
    return novo;
}

NoAST *criarNoOr(NoAST *esq, NoAST *dir) {
    NoAST *novo = alocarNo();
    novo->operador = 'O';
    novo->tipo     = T_BOOL;
    novo->esquerda = esq;
    novo->direita  = dir;
    return novo;
}

NoAST *criarNoNot(NoAST *operando) {
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