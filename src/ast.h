#ifndef AST_H
#define AST_H

#include "tipos.h"

NoAST *criarNoInt(int valor);
NoAST *criarNoFloat(float valor);
NoAST *criarNoChar(char valor);
NoAST *criarNoBool(int valor);
NoAST *criarNoId(char *nome);
NoAST *criarNoOp(char operador, NoAST *esq, NoAST *dir);
NoAST *criarNoSeq(NoAST *esq, NoAST *dir);
NoAST *criarNoAtrib(char *nome, NoAST *valor);
NoAST *criarNoDecl(Tipo tipo, char *nome, NoAST *valor);
NoAST *criarNoIf(NoAST *condicao, NoAST *blocoTrue, NoAST *blocoFalse);
NoAST *criarNoWhile(NoAST *condicao, NoAST *corpo);
NoAST *criarNoFor(NoAST *init, NoAST *cond, NoAST *incr, NoAST *corpo);
NoAST *criarNoPrintf(NoAST *expr);
NoAST *criarNoAnd(NoAST *esq, NoAST *dir);
NoAST *criarNoOr(NoAST *esq, NoAST *dir);
NoAST *criarNoNot(NoAST *operando);
void   imprimirAST(NoAST *raiz);
NoAST *criarNoString(const char *texto);
NoAST *criarNoPrintfFmt(NoAST *fmt, NoAST *args);
NoAST *criarListaArgs(NoAST *arg, NoAST *resto);

/* ── funções ── */
/* 'Z' = definição de função
   nome  = nome da função
   tipo  = tipo de retorno
   params = lista de Param
   esquerda = corpo (bloco)
   direita  = NULL */
NoAST *criarNoDefFuncao(Tipo retorno, const char *nome, Param *params, NoAST *corpo);

/* 'C' = chamada de função
   nome     = nome da função
   esquerda = lista de argumentos ('L') ou NULL */
NoAST *criarNoChamada(const char *nome, NoAST *args);

/* 'K' = return
   esquerda = expressão de retorno (ou NULL para void) */
NoAST *criarNoReturn(NoAST *expr);

/* utilitários para parâmetros */
Param *criarParam(Tipo tipo, const char *nome);
Param *adicionarParam(Param *lista, Param *novo);

#endif