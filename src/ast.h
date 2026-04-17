#ifndef AST_H
#define AST_H

#include "tipos.h"

NoAST *criarNoInt(int valor);
NoAST *criarNoFloat(float valor);
NoAST *criarNoId(char *nome);
NoAST *criarNoOp(char operador, NoAST *esq, NoAST *dir);
void imprimirAST(NoAST *raiz);
NoAST *criarNoSeq(NoAST *esq, NoAST *dir);
NoAST *criarNoAtrib(char *nome, NoAST *valor);
NoAST *criarNoChar(char valor);
NoAST *criarNoDecl(Tipo tipo, char *nome, NoAST *valor);
#endif