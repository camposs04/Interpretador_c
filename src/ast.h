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

#endif