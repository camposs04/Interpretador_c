#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

void erroSemantico(int codigoErro, int linha);
void analisarSemantica(NoAST *raiz);
int errosSemanticos(void);

#endif